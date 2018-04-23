#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <mach/hardware.h>
#include <linux/gpio.h>

#include <mach/map.h>
#include <mach/gpio.h>
#include <mach/regs-clock.h>
#include <mach/regs-gpio.h>
#include "btn.h"

struct btn_t {
	int gpio;
	int key;
	char *name;
};
static struct btn_t btns[] = {
	{ EXYNOS4_GPX3(2), 1 ,"K1" },
	{ EXYNOS4_GPX3(3), 2 ,"K2" },
	{ EXYNOS4_GPX3(4), 3 ,"K3" },
	{ EXYNOS4_GPX3(5), 4 ,"K4" },
};

static DECLARE_WAIT_QUEUE_HEAD(btn_waitq);
static volatile char key_value = 0;
static volatile int ev_press = 0;
static unsigned long *GPM4CON_R;
static unsigned long *GPM4DAT_R;

static irqreturn_t btn_irq_handler(int irq, void *dev)
{
	struct btn_t * p = (struct btn_t *)dev;
	key_value = p->key;
	ev_press = 1;
	wake_up_interruptible(&btn_waitq);
	
	return IRQ_HANDLED;
}

static int btn_drv_open(struct inode *inode, struct file *file)
{
	*GPM4CON_R &= ~0xffff;	
	*GPM4CON_R |=  0x1111;	
	*GPM4DAT_R |= 0xf;		
	return 0;
}

static int btn_drv_close(struct inode *inode, struct file *file)
{
	return 0;
}

static int btn_drv_read(struct file *filp, char __user *buff,size_t count, loff_t *offp)
{
	//休眠直到被唤醒且ev_press为真
	wait_event_interruptible(btn_waitq, ev_press);
	ev_press = 0;
	copy_to_user(buff,&key_value,sizeof(key_value));
	return sizeof(key_value);
}

static long btn_drv_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
		case TINY4412_BTN_IORESET:  //复位命令
			key_value = 0;
			printk("TINY4412 btn drv ioctl IORESET!\n ");
			break;
			
		case TINY4412_BTN_IOGET://获取命令
			key_value++;
			copy_to_user((void __user *)arg, &key_value, sizeof(char));
			printk("TINY4412 btn drv ioctl IOGET key_val++\n");
			break;
			
		case TINY4412_BTN_IOSET://设置命令
			copy_from_user(&key_value, (void __user *)arg, sizeof(char));
			printk("TINY4412 btn drv ioctl IOSET\n");
			break;
			
			
		case TINY4412_LED_ON:
			copy_from_user(&key_value, (void __user *)arg, sizeof(char));
			*GPM4DAT_R &= ~(1<<key_value);
			break;
			
		case TINY4412_LED_OFF:
			copy_from_user(&key_value, (void __user *)arg, sizeof(char));
			*GPM4DAT_R |= 1<<key_value;
			break;
			
		default:
			printk("TINY4412 btn drv ioctl error cmd\n");
			return -EINVAL;
	}

	return 0;
}

static struct file_operations btn_fops = {
	.owner		= THIS_MODULE,
	.open		= btn_drv_open,
	.release	= btn_drv_close, 
	.read		= btn_drv_read,
	.unlocked_ioctl	= btn_drv_ioctl,
};

static int major = 0;
static struct class *btn_class = NULL;

static int __init btn_drv_init(void)
{
	int irq;
	int i;
	for(i = 0;i<4;i++){
		irq = gpio_to_irq(btns[i].gpio);
		request_irq(irq, btn_irq_handler, IRQ_TYPE_EDGE_FALLING, 
				btns[i].name, &btns[i]);
	}
	
	GPM4CON_R= ioremap(0x110002e0, 8);
	GPM4DAT_R= GPM4CON_R+1;
	
	major = register_chrdev(0, "btn",&btn_fops);
	btn_class = class_create(THIS_MODULE, "btn");
	device_create(btn_class, NULL, MKDEV(major, 0),NULL,"btn");
	return 0;
}

static void __exit btn_drv_exit(void)
{
	int irq;
	int i;
	for(i = 0;i<4;i++){
		irq = gpio_to_irq(btns[i].gpio);
		free_irq(irq, &btns[i]);
	}
	device_destroy(btn_class, MKDEV(major,0));
	class_destroy(btn_class);
	unregister_chrdev(major, "btn");
	iounmap(GPM4CON_R);
}

module_init(btn_drv_init);
module_exit(btn_drv_exit);
MODULE_LICENSE("GPL");

