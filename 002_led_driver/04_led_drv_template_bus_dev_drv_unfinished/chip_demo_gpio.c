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
#include <linux/platform_device.h>
#include <asm/io.h>

#include "led_opr.h"
#include "leddrv.h"
#include "led_resource.h"

static int g_ledpins[100];
static int g_ledcnt = 0;

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

/* 初始化LED, which-哪个LED */    
static int board_demo_led_init (int which)    
{   
	int group, pin;
	unsigned int val;

	group = GROUP(g_ledpins[which]);
	pin = PIN(g_ledpins[which]);
	printk("init gpio: group %d, pin %d\n", group, pin);

	/* 100ask_IMX6uLL_Board LED：GPIO5_3 */
	if ((5 == group) && (3 == pin))
	{
		/* 相关寄存器物理地址与虚拟地址之间的映射 */
		/* 1、地址映射：时钟寄存器 */
		CCM_CCGR1 = ioremap(CCM_CCGR1_BASE, 4);		
		/* 2、地址映射：模式寄存器 */	
		SW_MUX_GPIO5_IO03 = ioremap(SW_MUX_GPIO5_IO03_BASE, 4); 
		/* 3、地址映射：数据寄存器 */
		GPIO5_DR = ioremap(GPIO5_DR_BASE, 4);	
		/* 地址映射：方向寄存器 */
		GPIO5_GDIR = ioremap(GPIO5_GDIR_BASE, 4);

		/* 使能GPIO5时钟 */
		val = readl(CCM_CCGR1); /* 读出当前CCM_CCGR1配置值 */
		val &= ~(3 << 30);		/* 清除以前的设置 */
		val |= (3 << 30);		/* 设置新值 */
		writel(val, CCM_CCGR1);

		/* 设置GPIO5_IO03的为IO模式 */
		writel(5, SW_MUX_GPIO5_IO03);
		
		/* 设置GPIO5_IO03方向为输出 */
		val = readl(GPIO5_GDIR); 
		val &= ~(1 << 3);		 
		val |= (1 << 3);		 
		writel(val, GPIO5_GDIR);
	}
	else
	{
		printk("This is not 100ask_IMX6ULL_Board!\n");
	}
	
	return 0;
}

/* 控制LED, which-哪个LED, status:1-亮,0-灭 */
static int board_demo_led_ctl (int which, char status) 
{
	int group, pin;
	unsigned int val;

	group = GROUP(g_ledpins[which]);
	pin = PIN(g_ledpins[which]);
	printk("init gpio: group %d, pin %d\n", group, pin);

	/* 100ask_IMX6uLL_Board LED：GPIO5_3 */
	if ((5 == group) && (3 == pin))
	{
		/* 点灯 */
		if (1 == status)
		{
			printk("<<<<<<<<led on>>>>>>>>>>\n");
			val = readl(GPIO5_DR);
			val &= ~(1 << 3);	
			writel(val, GPIO5_DR);
		}
		/* 灭灯 */
		else if (0 == status)
		{
			printk("<<<<<<<<led off>>>>>>>>>>\n");
			val = readl(GPIO5_DR);
			val|= (1 << 3);	
			writel(val, GPIO5_DR);
		}
		else{}
	}
	else
	{
		printk("This is not 100ask_IMX6ULL_Board!\n");
	}
	
	return 0;
}

static struct led_operations board_demo_led_opr = {
    .init = board_demo_led_init,
    .ctl  = board_demo_led_ctl,
};

struct led_operations *get_board_led_opr(void)
{
    return &board_demo_led_opr;
}

static int chip_demo_gpio_probe(struct platform_device *pdev)
{
    struct resource *res;
    int i = 0;

    while (1)
    {
        res = platform_get_resource(pdev, IORESOURCE_IRQ, i++);
        if (!res)
            break;
        
        g_ledpins[g_ledcnt] = res->start;
        led_class_create_device(g_ledcnt);
        g_ledcnt++;
    }
    return 0;
    
}

static int chip_demo_gpio_remove(struct platform_device *pdev)
{
    struct resource *res;
    int i = 0;

    while (1)
    {
        res = platform_get_resource(pdev, IORESOURCE_IRQ, i);
        if (!res)
            break;
        
        led_class_destroy_device(i);
        i++;
        g_ledcnt--;
    }
    return 0;
}

/* platform_driver结构体 */
static struct platform_driver chip_demo_gpio_driver = {
    .probe      = chip_demo_gpio_probe,
    .remove     = chip_demo_gpio_remove,
    .driver     = {
        .name   = "100ask_led",
    },
};

static int __init chip_demo_gpio_drv_init(void)
{
    int err;
    
	/* 驱动注册 */
    err = platform_driver_register(&chip_demo_gpio_driver); 
	/* 注册led操作 */
    register_led_operations(&board_demo_led_opr);
    
    return 0;
}

static void __exit lchip_demo_gpio_drv_exit(void)
{
    platform_driver_unregister(&chip_demo_gpio_driver);
}

module_init(chip_demo_gpio_drv_init);
module_exit(lchip_demo_gpio_drv_exit);

MODULE_LICENSE("GPL");

