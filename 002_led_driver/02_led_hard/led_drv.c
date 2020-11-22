#include <linux/module.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>

/* 寄存器物理地址 */
#define CCM_CCGR1_BASE				(0X020C406C)	
#define SW_MUX_GPIO5_IO03_BASE		(0X02290014)
#define GPIO5_DR_BASE				(0X020AC000)
#define GPIO5_GDIR_BASE				(0X020AC004)

/* 映射后的寄存器虚拟地址指针 */
static void __iomem *CCM_CCGR1;
static void __iomem *SW_MUX_GPIO5_IO03;
static void __iomem *GPIO5_DR;
static void __iomem *GPIO5_GDIR;


/* 1. 确定主设备号                                                                 */
static int major = 0;
static struct class *led_class;


#define MIN(a, b) (a < b ? a : b)

/* 3. 实现对应的open/read/write等函数，填入file_operations结构体                   */
static ssize_t led_drv_read (struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	int err;
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

	return 0;
}

static ssize_t led_drv_write (struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
	int err;
	char led_status;
	unsigned int val;
	
	printk("%s %s line %d: ", __FILE__, __FUNCTION__, __LINE__);
	err = copy_from_user(&led_status, buf, 1);
	/* 点灯 */
	if (0 == led_status)
	{
		printk("<<<<<<<<led on>>>>>>>>>>\n");
		val = readl(GPIO5_DR);
		val &= ~(1 << 3);	
		writel(val, GPIO5_DR);
	}
	/* 灭灯 */
	else if (1 == led_status)
	{
		printk("<<<<<<<<led off>>>>>>>>>>\n");
		val = readl(GPIO5_DR);
		val|= (1 << 3);	
		writel(val, GPIO5_DR);
	}
	else{}
	
	return 1;
}

static int led_drv_open (struct inode *node, struct file *file)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

static int led_drv_close (struct inode *node, struct file *file)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

/* 2. 定义自己的file_operations结构体                                              */
static struct file_operations led_drv = {
	.owner	 = THIS_MODULE,
	.open    = led_drv_open,
	.read    = led_drv_read,
	.write   = led_drv_write,
	.release = led_drv_close,
};

/* 4. 把file_operations结构体告诉内核：注册驱动程序                                */
/* 5. 谁来注册驱动程序啊？得有一个入口函数：安装驱动程序时，就会去调用这个入口函数 */
static int __init led_init(void)
{
	int err;
	unsigned int val;
	
	/* 相关寄存器物理地址与虚拟地址之间的映射 */
	/* 1、地址映射：时钟寄存器 */
  	CCM_CCGR1 = ioremap(CCM_CCGR1_BASE, 4);		
	/* 2、地址映射：模式寄存器 */	
	SW_MUX_GPIO5_IO03 = ioremap(SW_MUX_GPIO1_IO03_BASE, 4); 
	/* 3、地址映射：数据寄存器 */
	GPIO5_DR = ioremap(GPIO1_DR_BASE, 4);	
	/* 地址映射：方向寄存器 */
	GPIO5_GDIR = ioremap(GPIO1_GDIR_BASE, 4);

	/* 使能GPIO5时钟 */
	val = readl(CCM_CCGR1); /* 读出当前CCM_CCGR1配置值 */
	val &= ~(3 << 30);		/* 清除以前的设置 */
	val |= (3 << 30);		/* 设置新值 */
	writel(val, CCM_CCGR1);

	/* 设置GPIO5_IO03的为IO模式 */
	writel(5, SW_MUX_GPIO5_IO03);
	
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	major = register_chrdev(0, "led", &led_drv);  /* /dev/led */

	led_class = class_create(THIS_MODULE, "led_class");
	err = PTR_ERR(led_class);
	if (IS_ERR(led_class)) {
		printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
		unregister_chrdev(major, "led");
		return -1;
	}
	
	device_create(led_class, NULL, MKDEV(major, 0), NULL, "led"); /* /dev/led */
	
	return 0;
}

/* 6. 有入口函数就应该有出口函数：卸载驱动程序时，就会去调用这个出口函数           */
static void __exit led_exit(void)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	device_destroy(led_class, MKDEV(major, 0));
	class_destroy(led_class);
	unregister_chrdev(major, "led");
}


/* 7. 其他完善：提供设备信息，自动创建设备节点                                     */

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");


