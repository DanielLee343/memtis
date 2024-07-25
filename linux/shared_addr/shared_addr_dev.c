#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/io.h> // for virt_to_phys
#include <linux/kthread.h>
#include <linux/delay.h>

#define DEVICE_NAME "shared_addr_dev"
#define CLASS_NAME "shared_addr_class"
#define BUFFER_SIZE (PAGE_SIZE) // Allocate enough space for your array

static char *shared_buffer;
static int major_number;
static struct class *shared_mem_class = NULL;
static struct device *shared_mem_device = NULL;
static struct cdev c_dev;
static int *buffer_size;
static struct task_struct *task;
static struct mutex buffer_mutex;
#define NUM_ADDRESSES 10
// File operations prototypes
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static int device_mmap(struct file *, struct vm_area_struct *);

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_release,
	.mmap = device_mmap,
};

static int device_open(struct inode *inodep, struct file *filep)
{
	return 0;
}

static int device_release(struct inode *inodep, struct file *filep)
{
	return 0;
}

static int device_mmap(struct file *filep, struct vm_area_struct *vma)
{
	// Remap the kernel buffer to user space
	if (remap_pfn_range(vma, vma->vm_start,
			    virt_to_phys(shared_buffer) >> PAGE_SHIFT,
			    BUFFER_SIZE, vma->vm_page_prot)) {
		return -EAGAIN;
	}
	return 0;
}
static int kernel_thread_fn(void *data)
{
	printk(KERN_INFO "enter kernel print thread\n");
	unsigned long *address_array;
	int i; // Declare loop variable outside the loop
	while (!kthread_should_stop()) {
		printk(KERN_INFO "buffer_size: %d\n", *buffer_size);
		if (mutex_lock_interruptible(&buffer_mutex)) {
			return -ERESTARTSYS;
		}
		address_array = (unsigned long *)shared_buffer;

		// Print each address in the shared buffer
		printk(KERN_INFO "Shared Buffer Data:\n");
		for (i = 0; i < NUM_ADDRESSES; ++i) {
			printk(KERN_INFO "Address %d: 0x%lx\n", i,
			       address_array[i]);
		}
		mutex_unlock(&buffer_mutex);
		msleep(500); // Sleep for 5 seconds between checks
	}
	return 0;
}

static int __init shared_mem_init(void)
{
	// Register the character device
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number < 0) {
		printk(KERN_ERR "Failed to register character device\n");
		return major_number;
	}

	shared_mem_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(shared_mem_class)) {
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ERR "Failed to create device class\n");
		return PTR_ERR(shared_mem_class);
	}

	shared_mem_device =
		device_create(shared_mem_class, NULL, MKDEV(major_number, 0),
			      NULL, DEVICE_NAME);
	if (IS_ERR(shared_mem_device)) {
		class_destroy(shared_mem_class);
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ERR "Failed to create the device\n");
		return PTR_ERR(shared_mem_device);
	}

	printk(KERN_INFO "Shared memory device registered, major number: %d\n",
	       major_number);
	// Allocate memory for the shared buffer
	shared_buffer =
		(char *)__get_free_pages(GFP_KERNEL, get_order(BUFFER_SIZE));
	if (!shared_buffer) {
		printk(KERN_ERR "Failed to allocate shared memory\n");
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
		device_destroy(shared_mem_class, MKDEV(major_number, 0));
		class_destroy(shared_mem_class);
		unregister_chrdev(major_number, DEVICE_NAME);
		return PTR_ERR(task);
	}

	printk(KERN_INFO "Shared memory device registered\n");
	return 0;
}

static void __exit shared_mem_exit(void)
{
	kthread_stop(task);
	device_destroy(shared_mem_class, MKDEV(major_number, 0));
	class_unregister(shared_mem_class);
	class_destroy(shared_mem_class);
	unregister_chrdev(major_number, DEVICE_NAME);
	// kfree(shared_buffer);
	free_pages((unsigned long)shared_buffer, get_order(BUFFER_SIZE));
	shared_buffer = NULL;
	mutex_destroy(&buffer_mutex);
	printk(KERN_INFO "Shared memory device unregistered\n");
}

module_init(shared_mem_init);
module_exit(shared_mem_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Shared Memory Device Example");
MODULE_AUTHOR("OpenAI");