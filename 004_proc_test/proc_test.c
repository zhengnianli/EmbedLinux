#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/uaccess.h>

static char kernel_buf[1024];

#define MIN(a, b) (a < b ? a : b)

static int hello_proc_open(struct inode *node, struct file *file)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

static ssize_t hello_proc_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	int err;
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
    
	err = copy_to_user(buf, kernel_buf, MIN(1024, size));
	return MIN(1024, size);
}

static ssize_t hello_proc_write(struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
	int err;
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

	err = copy_from_user(kernel_buf, buf, MIN(1024, size));
	return MIN(1024, size);
}

static int hello_proc_close(struct inode *node, struct file *file)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

static const struct file_operations hello_proc_operations = 
{
    .owner	 = THIS_MODULE,
	.open	 = hello_proc_open,
	.read    = hello_proc_read,
	.write   = hello_proc_write,
	.release = hello_proc_close,
};

static int __init hello_proc_init(void)
{
	proc_create("hello_proc", 0, NULL, &hello_proc_operations);
	return 0;
}

static void __exit hello_proc_exit(void)
{
	remove_proc_entry("hello_proc", NULL);
}

module_init(hello_proc_init);
module_exit(hello_proc_exit);
MODULE_DESCRIPTION("proc test");
MODULE_LICENSE("GPL");