/*
 * memory access sampling for hugepage-aware tiered memory management.
 */
#include <linux/kthread.h>
#include <linux/memcontrol.h>
#include <linux/mempolicy.h>
#include <linux/sched.h>
#include <linux/perf_event.h>
#include <linux/delay.h>
#include <linux/sched/cputime.h>

#include "../kernel/events/internal.h"

#include <linux/htmm.h>

struct task_struct *access_sampling = NULL;
struct perf_event ***mem_event;

static bool valid_va(unsigned long addr)
{
	if (!(addr >> (PGDIR_SHIFT + 9)) && addr != 0)
		return true;
	else
		return false;
}

static __u64 get_pebs_event(enum events e)
{
	switch (e) {
	case DRAMREAD:
		return DRAM_LLC_LOAD_MISS;
	case NVMREAD:
		if (!htmm_cxl_mode)
			return NVM_LLC_LOAD_MISS;
		else
			return N_HTMMEVENTS;
	case MEMWRITE:
		return ALL_STORES;
	case CXLREAD:
		if (htmm_cxl_mode)
			return REMOTE_DRAM_LLC_LOAD_MISS;
		else
			return N_HTMMEVENTS;
	default:
		return N_HTMMEVENTS;
	}
}

static int __perf_event_open(__u64 config, __u64 config1, __u64 cpu, __u64 type,
			     __u32 pid)
{
	struct perf_event_attr attr;
	struct file *file;
	int event_fd, __pid;

	memset(&attr, 0, sizeof(struct perf_event_attr));

	attr.type = PERF_TYPE_RAW;
	attr.size = sizeof(struct perf_event_attr);
	attr.config = config;
	attr.config1 = config1;
	if (config == ALL_STORES)
		attr.sample_period = htmm_inst_sample_period;
	else
		attr.sample_period = get_sample_period(0);
	attr.sample_type = PERF_SAMPLE_IP | PERF_SAMPLE_TID | PERF_SAMPLE_ADDR;
	attr.disabled = 0;
	attr.exclude_kernel = 1;
	attr.exclude_hv = 1;
	attr.exclude_callchain_kernel = 1;
	attr.exclude_callchain_user = 1;
	attr.precise_ip = 1;
	attr.enable_on_exec = 1;

	if (pid == 0)
		__pid = -1;
	else
		__pid = pid;

	event_fd = htmm__perf_event_open(&attr, __pid, cpu, -1, 0);
	//event_fd = htmm__perf_event_open(&attr, -1, cpu, -1, 0);
	if (event_fd <= 0) {
		printk("[error htmm__perf_event_open failure] event_fd: %d\n",
		       event_fd);
		return -1;
	}

	file = fget(event_fd);
	if (!file) {
		printk("invalid file\n");
		return -1;
	}
	mem_event[cpu][type] = fget(event_fd)->private_data;
	return 0;
}

static int pebs_init(pid_t pid, int node)
{
	int cpu, event;

	mem_event = kzalloc(sizeof(struct perf_event **) * CPUS_PER_SOCKET,
			    GFP_KERNEL);
	for (cpu = 0; cpu < CPUS_PER_SOCKET; cpu++) {
		mem_event[cpu] = kzalloc(
			sizeof(struct perf_event *) * N_HTMMEVENTS, GFP_KERNEL);
	}

	printk("pebs_init\n");
	for (cpu = 0; cpu < CPUS_PER_SOCKET; cpu++) {
		for (event = 0; event < N_HTMMEVENTS; event++) {
			if (get_pebs_event(event) == N_HTMMEVENTS) {
				mem_event[cpu][event] = NULL;
				continue;
			}

			if (__perf_event_open(get_pebs_event(event), 0, cpu,
					      event, pid))
				return -1;
			if (htmm__perf_event_init(mem_event[cpu][event],
						  BUFFER_SIZE))
				return -1;
		}
	}

	return 0;
}

static void pebs_disable(void)
{
	int cpu, event;

	printk("pebs disable\n");
	for (cpu = 0; cpu < CPUS_PER_SOCKET; cpu++) {
		for (event = 0; event < N_HTMMEVENTS; event++) {
			if (mem_event[cpu][event])
				perf_event_disable(mem_event[cpu][event]);
		}
	}
}

static void pebs_enable(void)
{
	int cpu, event;

	printk("pebs enable\n");
	for (cpu = 0; cpu < CPUS_PER_SOCKET; cpu++) {
		for (event = 0; event < N_HTMMEVENTS; event++) {
			if (mem_event[cpu][event])
				perf_event_enable(mem_event[cpu][event]);
		}
	}
}

static void pebs_update_period(uint64_t value, uint64_t inst_value)
{
	int cpu, event;

	for (cpu = 0; cpu < CPUS_PER_SOCKET; cpu++) {
		for (event = 0; event < N_HTMMEVENTS; event++) {
			int ret;
			if (!mem_event[cpu][event])
				continue;

			switch (event) {
			case DRAMREAD:
			case NVMREAD:
			case CXLREAD:
				ret = perf_event_period(mem_event[cpu][event],
							value);
				break;
			case MEMWRITE:
				ret = perf_event_period(mem_event[cpu][event],
							inst_value);
				break;
			default:
				ret = 0;
				break;
			}

			if (ret == -EINVAL)
				printk("failed to update sample period");
		}
	}
}

static int ksamplingd(void *data)
{
	unsigned long long nr_sampled = 0, nr_dram = 0, nr_nvm = 0,
			   nr_write = 0;
	unsigned long long nr_throttled = 0, nr_lost = 0, nr_unknown = 0;
	unsigned long long nr_skip = 0;

	/* used for calculating average cpu usage of ksampled */
	struct task_struct *t = current;
	/* a unit of cputime: permil (1/1000) */
	u64 total_runtime, exec_runtime, cputime = 0;
	unsigned long total_cputime, elapsed_cputime, cur;
	/* used for periodic checks*/
	unsigned long cpucap_period = msecs_to_jiffies(15000); // 15s
	unsigned long sample_period = 0;
	unsigned long sample_inst_period = 0;
	/* report cpu/period stat */
	unsigned long trace_cputime,
		trace_period = msecs_to_jiffies(1500); // 3s
	unsigned long trace_runtime;
	/* for timeout */
	unsigned long sleep_timeout;

	/* for analytic purpose */
	unsigned long hr_dram = 0, hr_nvm = 0;

	/* orig impl: see read_sum_exec_runtime() */
	trace_runtime = total_runtime = exec_runtime = t->se.sum_exec_runtime;

	trace_cputime = total_cputime = elapsed_cputime = jiffies;
	sleep_timeout = usecs_to_jiffies(2000);

	/* TODO implements per-CPU node ksamplingd by using pg_data_t */
	/* Currently uses a single CPU node(0) */
	const struct cpumask *cpumask = cpumask_of_node(0);
	if (!cpumask_empty(cpumask))
		do_set_cpus_allowed(access_sampling, cpumask);

	while (!kthread_should_stop()) {
		int cpu, event, cond = false;

		if (htmm_mode == HTMM_NO_MIG) {
			msleep_interruptible(10000);
			continue;
		}

		for (cpu = 0; cpu < CPUS_PER_SOCKET; cpu++) {
			for (event = 0; event < N_HTMMEVENTS; event++) {
				do {
					struct perf_buffer *rb;
					struct perf_event_mmap_page *up;
					struct perf_event_header *ph;
					struct htmm_event *he;
					unsigned long pg_index, offset;
					int page_shift;
					__u64 head;

					if (!mem_event[cpu][event]) {
						//continue;
						break;
					}

					__sync_synchronize();

					rb = mem_event[cpu][event]->rb;
					if (!rb) {
						printk("event->rb is NULL\n");
						return -1;
					}
					/* perf_buffer is ring buffer */
					up = READ_ONCE(rb->user_page);
					head = READ_ONCE(up->data_head);
					if (head == up->data_tail) {
						if (cpu < 16)
							nr_skip++;
						//continue;
						break;
					}

					head -= up->data_tail;
					if (head >
					    (BUFFER_SIZE *
					     ksampled_max_sample_ratio / 100)) {
						cond = true;
					} else if (head <
						   (BUFFER_SIZE *
						    ksampled_min_sample_ratio /
						    100)) {
						cond = false;
					}

					/* read barrier */
					smp_rmb();

					page_shift =
						PAGE_SHIFT + page_order(rb);
					/* get address of a tail sample */
					offset = READ_ONCE(up->data_tail);
					pg_index = (offset >> page_shift) &
						   (rb->nr_pages - 1);
					offset &= (1 << page_shift) - 1;

					ph = (void *)(rb->data_pages[pg_index] +
						      offset);
					switch (ph->type) {
					case PERF_RECORD_SAMPLE:
						he = (struct htmm_event *)ph;
						if (!valid_va(he->addr)) {
							break;
						}

						update_pginfo(he->pid, he->addr,
							      event);
						//count_vm_event(HTMM_NR_SAMPLED);
						nr_sampled++;

						if (event == DRAMREAD) {
							nr_dram++;
							hr_dram++;
						} else if (event == CXLREAD ||
							   event == NVMREAD) {
							nr_nvm++;
							hr_nvm++;
						} else
							nr_write++;
						break;
					case PERF_RECORD_THROTTLE:
					case PERF_RECORD_UNTHROTTLE:
						nr_throttled++;
						break;
					case PERF_RECORD_LOST_SAMPLES:
						nr_lost++;
						break;
					default:
						nr_unknown++;
						break;
					}
					if (nr_sampled % 500000 == 0) {
						trace_printk(
							"nr_sampled: %llu, nr_dram: %llu, nr_nvm: %llu, nr_write: %llu, nr_throttled: %llu \n",
							nr_sampled, nr_dram,
							nr_nvm, nr_write,
							nr_throttled);
						nr_dram = 0;
						nr_nvm = 0;
						nr_write = 0;
					}
					/* read, write barrier */
					smp_mb();
					WRITE_ONCE(up->data_tail,
						   up->data_tail + ph->size);
				} while (cond);
			}
		}
		/* if ksampled_soft_cpu_quota is zero, disable dynamic pebs feature */
		if (!ksampled_soft_cpu_quota)
			continue;

		/* sleep */
		schedule_timeout_interruptible(sleep_timeout);

		/* check elasped time */
		cur = jiffies;
		if ((cur - elapsed_cputime) >= cpucap_period) {
			u64 cur_runtime = t->se.sum_exec_runtime;
			exec_runtime = cur_runtime - exec_runtime; //ns
			elapsed_cputime =
				jiffies_to_usecs(cur - elapsed_cputime); //us
			if (!cputime) {
				u64 cur_cputime = div64_u64(exec_runtime,
							    elapsed_cputime);
				// EMA with the scale factor (0.2)
				cputime =
					((cur_cputime << 3) + (cputime << 1)) /
					10;
			} else
				cputime = div64_u64(exec_runtime,
						    elapsed_cputime);

			/* to prevent frequent updates, allow for a slight variation of +/- 0.5% */
			if (cputime > (ksampled_soft_cpu_quota + 5) &&
			    sample_period != pcount) {
				/* need to increase the sample period */
				/* only increase by 1 */
				unsigned long tmp1 = sample_period,
					      tmp2 = sample_inst_period;
				increase_sample_period(&sample_period,
						       &sample_inst_period);
				if (tmp1 != sample_period ||
				    tmp2 != sample_inst_period)
					pebs_update_period(
						get_sample_period(
							sample_period),
						get_sample_inst_period(
							sample_inst_period));
			} else if (cputime < (ksampled_soft_cpu_quota - 5) &&
				   sample_period) {
				unsigned long tmp1 = sample_period,
					      tmp2 = sample_inst_period;
				decrease_sample_period(&sample_period,
						       &sample_inst_period);
				if (tmp1 != sample_period ||
				    tmp2 != sample_inst_period)
					pebs_update_period(
						get_sample_period(
							sample_period),
						get_sample_inst_period(
							sample_inst_period));
			}
			/* does it need to prevent ping-pong behavior? */

			elapsed_cputime = cur;
			exec_runtime = cur_runtime;
		}

		/* This is used for reporting the sample period and cputime */
		if (cur - trace_cputime >= trace_period) {
			unsigned long hr = 0;
			u64 cur_runtime = t->se.sum_exec_runtime;
			trace_runtime = cur_runtime - trace_runtime;
			trace_cputime = jiffies_to_usecs(cur - trace_cputime);
			trace_cputime = div64_u64(trace_runtime, trace_cputime);

			if (hr_dram + hr_nvm == 0)
				hr = 0;
			else
				hr = hr_dram * 10000 / (hr_dram + hr_nvm);
			trace_printk(
				"sample_period: %lu || cputime: %lu  || hit ratio: %lu\n",
				get_sample_period(sample_period), trace_cputime,
				hr);

			hr_dram = hr_nvm = 0;
			trace_cputime = cur;
			trace_runtime = cur_runtime;
		}
	}

	total_runtime = (t->se.sum_exec_runtime) - total_runtime; // ns
	total_cputime = jiffies_to_usecs(jiffies - total_cputime); // us

	printk("nr_sampled: %llu, nr_throttled: %llu, nr_lost: %llu\n",
	       nr_sampled, nr_throttled, nr_lost);
	printk("total runtime: %llu ns, total cputime: %lu us, cpu usage: %llu\n",
	       total_runtime, total_cputime, (total_runtime) / total_cputime);

	return 0;
}

static int ksamplingd_run(void)
{
	int err = 0;

	if (!access_sampling) {
		access_sampling = kthread_run(ksamplingd, NULL, "ksamplingd");
		if (IS_ERR(access_sampling)) {
			err = PTR_ERR(access_sampling);
			access_sampling = NULL;
		}
	}
	return err;
}
// #include <linux/kernel.h>
// #include <linux/syscalls.h>
#include <linux/mm.h>
// #include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
// #include <linux/delay.h>
// #include <linux/cdev.h>
// #include <linux/io.h>
static int major_number;
char *intercepted_addr_buffer;
int *intercepted_addr_buffer_idx;
unsigned long intercepted_addr_buffer_capacity;
bool *buffer_is_ready;
EXPORT_SYMBOL(intercepted_addr_buffer);
EXPORT_SYMBOL(intercepted_addr_buffer_idx);
EXPORT_SYMBOL(intercepted_addr_buffer_capacity);
EXPORT_SYMBOL(buffer_is_ready);
static struct task_struct *shared_mem_thread;
static struct mutex buffer_mutex;
// static dev_t dev_num;
// static struct cdev c_dev;
// static struct class *cl;

static int kernel_thread_fn(void *data)
{
	printk(KERN_INFO "capacity: %ld\n", intercepted_addr_buffer_capacity);
	pid_t pid = *(pid_t *)data;
	int num_slots = intercepted_addr_buffer_capacity / 8;
	enum events tmp_event = TLB_MISS_LOADS;
	unsigned long *address_array;
	int i;
	while (!kthread_should_stop()) {
		// if (*intercepted_addr_buffer_idx >= num_slots) {
		// printk(KERN_INFO "buffer_is_ready: %d\n", *buffer_is_ready);
		if (*buffer_is_ready) {
			if (mutex_lock_interruptible(&buffer_mutex)) {
				return -ERESTARTSYS;
			}
			address_array =
				(unsigned long *)intercepted_addr_buffer;
			// printk(KERN_INFO "Kernel received: %s\n",
			//        intercepted_addr_buffer);
			for (i = 0; i < num_slots; i++) {
				if (!valid_va(address_array[i])) {
					// printk(KERN_INFO
					//        "invalide addr: 0x%lx\n",
					//        address_array[i]);
					continue;
				}
				update_pginfo(pid, address_array[i], tmp_event);
				// printk(KERN_INFO "sent: 0x%lx\n",
				//        address_array[i]);
			}
			*buffer_is_ready = false;
			mutex_unlock(&buffer_mutex);
		}

		msleep(10);
	}
	return 0;
}

int read_device_mem_init(void)
{
	size_t i;
	if (!intercepted_addr_buffer) {
		printk(KERN_WARNING "Device buffer not found\n");
		return -EINVAL;
	}
	printk(KERN_INFO "buffer address: %p\n", intercepted_addr_buffer);
	unsigned long *address_array = (unsigned long *)intercepted_addr_buffer;
	for (i = 0; i < 10; i++) {
		// if (!valid_va(address_array[i])) {
		// 	continue;
		// }
		// update_pginfo(pid, address_array[i], tmp_event);
		printk(KERN_INFO "kernel get: 0x%lx\n", address_array[i]);
	}
	return 0;
	// }
}
int shared_mem_init(pid_t pid)
{
	printk(KERN_INFO "pid in shared_mem: %d\n", pid);
	if (!intercepted_addr_buffer) {
		printk(KERN_WARNING "intercepted_addr_buffer not mapped!\n");
		return -EINVAL;
	}
	// shared_buffer = (char *)__get_free_pages(
	// 	GFP_KERNEL, get_order(BUFFER_SIZE_SHARED_MEM));
	// if (!shared_buffer) {
	// 	printk(KERN_ERR "Failed to init shared_buffer\n");
	// 	cdev_del(&c_dev);
	// 	device_destroy(cl, dev_num);
	// 	class_destroy(cl);
	// 	unregister_chrdev_region(dev_num, 1);
	// 	return -ENOMEM;
	// }
	// memset(shared_buffer, 0, BUFFER_SIZE_SHARED_MEM);
	// memset(intercepted_addr_buffer, 0, BUFFER_SIZE_SHARED_MEM);
	// buffer_size_ptr =
	// 	(int *)(intercepted_addr_buffer + BUFFER_SIZE_SHARED_MEM - sizeof(int));
	mutex_init(&buffer_mutex);

	shared_mem_thread =
		kthread_run(kernel_thread_fn, &pid, "shared_mem_thread");
	if (IS_ERR(shared_mem_thread)) {
		// 	// free_pages((unsigned long)shared_buffer,
		// 	// 	   get_order(BUFFER_SIZE_SHARED_MEM));
		// kfree(intercepted_addr_buffer);
		// 	intercepted_addr_buffer = NULL;
		printk(KERN_ERR "Failed to create kernel thread\n");
		return PTR_ERR(shared_mem_thread);
	}
	return 0;
}
void shared_mem_exit(void)
{
	kthread_stop(shared_mem_thread);
	mutex_destroy(&buffer_mutex);
	printk(KERN_INFO "enter shared mem exit\n");
}
int ksamplingd_init(pid_t pid, int node)
{
	int ret;
	printk("htmm init!\n");
	printk(KERN_INFO "pid in ksamplingd: %d\n", pid);

	if (access_sampling)
		return 0;

	ret = pebs_init(pid, node);
	if (ret) {
		printk("htmm__perf_event_init failure... ERROR:%d\n", ret);
		return 0;
	}

	return ksamplingd_run();
}

void ksamplingd_exit(void)
{
	if (access_sampling) {
		kthread_stop(access_sampling);
		access_sampling = NULL;
	}
	pebs_disable();
}
