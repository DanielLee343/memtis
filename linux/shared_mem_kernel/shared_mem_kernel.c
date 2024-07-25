#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/io.h> // for virt_to_phys
#include <linux/version.h>
#include <linux/init.h>

#define DEVICE_NAME "shared_mem_device"
#define BUFFER_SIZE 4096

static char *shared_buffer;
static dev_t dev_num;
static struct cdev c_dev;
static struct class *cl;
static struct task_struct *task;
static struct mutex buffer_mutex;

static int *buffer_size;

static int device_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int device_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long pfn = virt_to_phys(shared_buffer) >> PAGE_SHIFT;
	size_t size = vma->vm_end - vma->vm_start;

	if (size > BUFFER_SIZE) {
		return -EINVAL;
	}

	if (remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot)) {
		return -EAGAIN;
	}

	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_release,
	.mmap = device_mmap,
};

static int kernel_thread_fn(void *data)
{
	while (!kthread_should_stop()) {
		printk(KERN_INFO "buffer_size: %d\n", *buffer_size);
		if (*buffer_size > 0) {
			if (mutex_lock_interruptible(&buffer_mutex)) {
				return -ERESTARTSYS;
			}

			printk(KERN_INFO "Data from user space: %s\n",
			       shared_buffer);
			*buffer_size = 0;

			mutex_unlock(&buffer_mutex);
		}
		msleep(500);
	}
	return 0;
}

static int __init device_init(void)
{
	if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0) {
		return -1;
	}

	if ((cl = class_create(THIS_MODULE, DEVICE_NAME)) == NULL) {
		unregister_chrdev_region(dev_num, 1);
		return -1;
	}

	if (device_create(cl, NULL, dev_num, NULL, DEVICE_NAME) == NULL) {
		class_destroy(cl);
		unregister_chrdev_region(dev_num, 1);
		return -1;
	}

	cdev_init(&c_dev, &fops);

	if (cdev_add(&c_dev, dev_num, 1) == -1) {
		device_destroy(cl, dev_num);
		class_destroy(cl);
		unregister_chrdev_region(dev_num, 1);
		return -1;
	}

	shared_buffer =
		(char *)__get_free_pages(GFP_KERNEL, get_order(BUFFER_SIZE));
	if (!shared_buffer) {
		cdev_del(&c_dev);
		device_destroy(cl, dev_num);
		class_destroy(cl);
		unregister_chrdev_region(dev_num, 1);
		return -ENOMEM;
	}

	memset(shared_buffer, 0, BUFFER_SIZE);
	buffer_size = (int *)(shared_buffer + BUFFER_SIZE - sizeof(int));

	mutex_init(&buffer_mutex);

	task = kthread_run(kernel_thread_fn, NULL, "shared_mem_thread");
	if (IS_ERR(task)) {
		free_pages((unsigned long)shared_buffer,
			   get_order(BUFFER_SIZE));
		cdev_del(&c_dev);
		device_destroy(cl, dev_num);
		class_destroy(cl);
		unregister_chrdev_region(dev_num, 1);
		return PTR_ERR(task);
	}

	printk(KERN_INFO "Shared memory device registered\n");
	return 0;
}

static void __exit device_exit(void)
{
	kthread_stop(task);
	free_pages((unsigned long)shared_buffer, get_order(BUFFER_SIZE));
	cdev_del(&c_dev);
	device_destroy(cl, dev_num);
	class_destroy(cl);
	unregister_chrdev_region(dev_num, 1);
	printk(KERN_INFO "Shared memory device unregistered\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ChatGPT");
MODULE_DESCRIPTION("A simple shared memory device driver");

module_init(device_init);
module_exit(device_exit);