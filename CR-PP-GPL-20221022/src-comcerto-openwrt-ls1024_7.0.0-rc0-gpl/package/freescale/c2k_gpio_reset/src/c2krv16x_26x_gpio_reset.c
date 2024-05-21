/*
# 03-03-2017: harry.lin <harry.lin@deltaww.com>
# Do some changing for burn in test for facotry test require
# 2017-02-06: harry.lin <harry.lin@deltaww.com>
# Add c2k_gpio_reset module support.
*/

/* Module to reset target device through GPIO46 reset pin */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>                 
#include <linux/interrupt.h>            
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/kthread.h>

#define GPIO_PIN_NUM_46    46

static struct task_struct *reset_thread;

static const char * const remove_burn_in_file_argv[] =
	{ "/bin/mv", "/mnt/packages/burn_in/burn_in_test", "/tmp/", NULL };

static const char * const stop_burn_in_argv[] =
	{ "/mnt/packages/burn_in/burn-in-init", "stop", NULL };

static const char * const reboot_argv[] = 
    { "/sbin/reboot", NULL };

static const char * const factory_reset_argv[] =
    { "/usr/sbin/config_mgmt.sh", "factory-default" , NULL };

static const char * const cert_reset_argv[] =
    { "/usr/bin/delete_certificates", "factory_default" , NULL };

static const char * const led_indication_argv[] =
    { "/test_scripts/test_led", "on", "amber",  NULL};

static const char * const remove_burn_in_folder_argv[] =
	{ "/bin/rm", "-rf", "/mnt/packages/burn_in/", NULL};

static int c2krv16x_26x_gpio_reboot(void)
{
	printk(KERN_INFO "%s \n",__func__);
	call_usermodehelper(stop_burn_in_argv[0], stop_burn_in_argv, NULL, UMH_WAIT_PROC);
	call_usermodehelper(remove_burn_in_file_argv[0], remove_burn_in_file_argv, NULL, UMH_WAIT_PROC);
	call_usermodehelper(reboot_argv[0], reboot_argv, NULL, UMH_NO_WAIT);
}

static int c2krv16x_26x_gpio_factory_reset(void)
{
        printk(KERN_INFO "%s \n",__func__);
        call_usermodehelper(cert_reset_argv[0], cert_reset_argv, NULL, UMH_WAIT_PROC);
        call_usermodehelper(factory_reset_argv[0], factory_reset_argv, NULL, UMH_WAIT_PROC);
        call_usermodehelper(remove_burn_in_folder_argv[0], remove_burn_in_folder_argv, NULL, UMH_WAIT_PROC);
        call_usermodehelper(led_indication_argv[0], led_indication_argv, NULL, UMH_WAIT_PROC);
}

static int c2krv16x_26x_gpio_get_value(int offset)
{
	if (offset < 32)
		return ((__raw_readl(COMCERTO_GPIO_INPUT_REG) >> offset) & 0x1) ? 1 : 0 ;
	else if (offset < 64)
		return (( __raw_readl(COMCERTO_GPIO_63_32_PIN_INPUT) >> (offset - 32)) & 0x1) ? 1 : 0;
	else
		return -EINVAL;
}

int c2krv16x_26x_reset_thread(void) 
{
	int p1=0;

	printk(KERN_INFO "%s: In thread\n",__func__);

	while(!kthread_should_stop())
   	{
        	if(c2krv16x_26x_gpio_get_value(GPIO_PIN_NUM_46) == 0)
                	p1++;
		
		msleep(100);

        	if(((p1 >= 1) && (p1 < 100)) && (c2krv16x_26x_gpio_get_value(GPIO_PIN_NUM_46)))
        	{
                	printk(KERN_INFO "%s: *******  SYSTEM REBOOT ******\n",__func__);
                	p1 = 0;
                	c2krv16x_26x_gpio_reboot();
        	}
        	else if((p1 >= 100) && (c2krv16x_26x_gpio_get_value(GPIO_PIN_NUM_46)))
        	{
                	printk(KERN_INFO "%s: *******  FACTORY RESET ******\n",__func__);
                	p1 = 0;
			c2krv16x_26x_gpio_factory_reset();
			c2krv16x_26x_gpio_reboot();
        	}

  	}

	return 0;
}
 
static int __init c2krv16x_26x_gpio_reset_init(void)
{
	printk(KERN_INFO "%s:  initializing GPIO pin: %d\n",__func__,GPIO_PIN_NUM_46);

	/* select GPIO46 pin mux */
        __raw_writel(__raw_readl(COMCERTO_GPIO_63_32_PIN_SELECT) | (0x1 << 14), COMCERTO_GPIO_63_32_PIN_SELECT);

        /* configure GPIO46 as input */
        __raw_writel(__raw_readl(COMCERTO_GPIO_63_32_PIN_OUTPUT_EN) | (0x1 << 14), COMCERTO_GPIO_63_32_PIN_OUTPUT_EN);

		msleep(100);
	/* creating kernel thread to monitor GPIO46 status */
	reset_thread = kthread_create(c2krv16x_26x_reset_thread,NULL,"c2krv16x_26x_reset");
	if(reset_thread)
	{
		printk(KERN_INFO "%s: thread created",__func__);
		wake_up_process(reset_thread);
	}

	printk(KERN_INFO "%s:	return success\n",__func__);
	return 0;
}

static void __exit c2krv16x_26x_gpio_reset_exit(void)
{
	printk(KERN_INFO "%s  \n",__func__);
	if(reset_thread)
	{
		kthread_stop(reset_thread);
		reset_thread = NULL;
	}
}

module_init(c2krv16x_26x_gpio_reset_init);
module_exit(c2krv16x_26x_gpio_reset_exit);
MODULE_LICENSE("GPL");
