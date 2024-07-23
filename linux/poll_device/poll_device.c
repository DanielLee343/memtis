#include <linux/module.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#define DEVICE_NAME "poll_device"
#define BUFFER_SIZE 1024

static char device_buffer[BUFFER_SIZE];
static int buffer_size = 0;
static dev_t dev_num;
static struct cdev c_dev;
static struct class *cl;
static wait_queue_head_t write_queue;
static int data_available = 0;
static struct task_struct *task;

static int device_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t device_read(struct file *file, char __user *user_buffer,
			   size_t count, loff_t *ppos)
{
	if (buffer_size == 0) {
		return 0; // No data to read
	}

	if (count > buffer_size) {
		count = buffer_size;
	}

	if (copy_to_user(user_buffer, device_buffer, count)) {
		return -EFAULT;
	}

	buffer_size = 0; // Data has been read
	return count;
}

static ssize_t device_write(struct file *file, const char __user *user_buffer,
			    size_t count, loff_t *ppos)
{
	if (count > BUFFER_SIZE) {
		count = BUFFER_SIZE;
	}

	if (copy_from_user(device_buffer, user_buffer, count)) {
		return -EFAULT;
	}

	buffer_size = count;
	data_available = 1;
	wake_up_interruptible(&write_queue); // Notify kernel
	return count;
}

static unsigned int device_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;

	poll_wait(file, &write_queue, wait);

	if (data_available) {
		mask |= POLLIN | POLLRDNORM; // Data can be read
	}

	return mask;
}

static int epoll_thread(void *data)
{
	while (!kthread_should_stop()) {
		wait_event_interruptible(write_queue, data_available != 0);

		if (data_available) {
			printk(KERN_INFO "Data from user space: %s\n",
			       device_buffer);
			data_available = 0;
		}

		msleep(100);
	}
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_release,
	.read = device_read,
	.write = device_write,
	.poll = device_poll,
};

static int __init device_init(void)
{
	printk(KERN_INFO "device init!\n");
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

	init_waitqueue_head(&write_queue);
	task = kthread_run(epoll_thread, NULL, "poll_device_thread");
	if (IS_ERR(task)) {
		printk(KERN_ALERT "Failed to create kernel thread\n");
		return PTR_ERR(task);
	}

	printk(KERN_INFO "Poll device registered\n");
	return 0;
}

static void __exit device_exit(void)
{
	kthread_stop(task);
	cdev_del(&c_dev);
	device_destroy(cl, dev_num);
	class_destroy(cl);
	unregister_chrdev_region(dev_num, 1);
	printk(KERN_INFO "Poll device unregistered\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ChatGPT");
MODULE_DESCRIPTION("A simple poll device driver");

module_init(device_init);
module_exit(device_exit);