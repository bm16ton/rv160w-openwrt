#include <linux/init.h>
#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/fs.h>
#include <linux/cdev.h>  
#include <linux/device.h> 
#include <asm/uaccess.h>
#include <linux/delay.h>

extern int
robo_spi_read(void *cookie, uint16_t reg, uint64_t *buf, int len);

extern int
robo_spi_write(void *cookie, uint16_t reg, uint64_t *buf, int len);

#define ROBO_RREG(_robo, _dev, _page, _reg, _buf, _len) \
        robo_spi_read(NULL, \
                      (_page << 8) | (_reg), _buf, _len)
#define ROBO_WREG(_robo, _dev, _page, _reg, _buf, _len) \
        robo_spi_write(NULL, \
                       (_page << 8) | (_reg), _buf, _len)

#define DEVNAME  "RoboDev"
#define ROBODRV   "/dev/" DEVNAME

#define ROBO_DRIVER_TYPE 'R'
#define ROBO_IOC_CMD _IOW(ROBO_DRIVER_TYPE, 1, ROBO_OBJ_CMD_MSG)
#define NUMBER_OF_MINOR_DEVICE (0)

typedef enum Cmdmsg
{
	DeviceRead,
	DeviceWrite,
	QosStats,
	IMP_Enable,
	IMP_Disable,
	Default_Reg
}Cmdmsg;

typedef struct tag_ROBO_OBJ_CMD_MSG ROBO_OBJ_CMD_MSG;


struct tag_ROBO_OBJ_CMD_MSG
{
	Cmdmsg       commandMsg;
	unsigned int Page_addr;
	unsigned int Reg_addr;
	unsigned int Reg_value;
	unsigned int port;
	unsigned int length;
	uint32_t Qos_stats[4];

};

static struct cdev c_dev;
static dev_t first;

static long av_vid_ioctl(struct file *filp, unsigned int uiVidCmd, unsigned long arg);

static struct file_operations fops = {
	.unlocked_ioctl = av_vid_ioctl,
};

int robo_CmdAction(ROBO_OBJ_CMD_MSG *pstVidObjCmdMsg)
{
	int i=0;

	if(pstVidObjCmdMsg->commandMsg == DeviceRead)
	{
		uint8_t buf[8]={0};
		ROBO_RREG(0, 0, pstVidObjCmdMsg->Page_addr, pstVidObjCmdMsg->Reg_addr, (uint64_t *)&buf, (unsigned int)pstVidObjCmdMsg->length);

		printk("Read [page:0x%x, reg:0x%x, len:%d]", pstVidObjCmdMsg->Page_addr, pstVidObjCmdMsg->Reg_addr,pstVidObjCmdMsg->length);

		for (i=0; i< pstVidObjCmdMsg->length; i++)
		{
			printk("[buf[%d]:0x%x] ", i, buf[i]);
		}
		printk("\n");
		return 0;
	}
	else if(pstVidObjCmdMsg->commandMsg == DeviceWrite)
	{

		uint8_t buf[8]={0};
		buf[0]=pstVidObjCmdMsg->Reg_value;

		ROBO_WREG(0, 0, pstVidObjCmdMsg->Page_addr, pstVidObjCmdMsg->Reg_addr, (uint64_t *)buf, (unsigned int)pstVidObjCmdMsg->length);

		printk("Write [page:0x%x, reg:0x%x, len:%d]", pstVidObjCmdMsg->Page_addr, pstVidObjCmdMsg->Reg_addr,pstVidObjCmdMsg->length);

		for (i=0; i< pstVidObjCmdMsg->length; i++)
		{
			printk("buf[%d]:[0x%x] ", i, buf[i]);
		}
		printk("\n");

		return 0;
	}
	else if(pstVidObjCmdMsg->commandMsg == IMP_Enable)
	{
                printk("IMP port functionality ENABLE ");
		/*** Enable IMP port settting for robo switch BCM53128 and BCM53134
		setreg 0x02 0x03 0x1
		setreg 0x0 0xb 0x7 
		setreg 0x02 0x0 0x80 //As per BCM mail
		setreg 0x0 0x08 0x1c     
		***/
		uint8_t buf[8]={0}, buf1[8]={0}, buf2[8]={0}, buf3[8]={0}, buf4[8]={0};
		
		buf[0] = 0x1;
		ROBO_WREG(0, 0, 0x2, 0x3, (uint64_t *)buf, (unsigned int)1);
		
		buf[0] = 0x7;
		ROBO_WREG(0, 0, 0x0, 0xb, (uint64_t *)buf, (unsigned int)1);

		//buf[0]=0x80; 
		buf[0]=0x82;
		ROBO_WREG(0, 0, 0x2, 0x0, (uint64_t *)buf, (unsigned int)2);

		buf[0] = 0x1c;
		ROBO_WREG(0, 0, 0x0, 0x8, (uint64_t *)buf, (unsigned int)2);


		ROBO_RREG(0, 0, 0x2, 0x3, (uint64_t *)buf1, (unsigned int)1);
		ROBO_RREG(0, 0, 0x0, 0xb, (uint64_t *)buf2, (unsigned int)1);
		ROBO_RREG(0, 0, 0x2, 0x0, (uint64_t *)buf3, (unsigned int)2);
		ROBO_RREG(0, 0, 0x0, 0x8, (uint64_t *)buf4, (unsigned int)2);
		
		printk("[ %x %x %02x %02x ]\n",buf1[0], buf2[0], buf3[0], buf4[0]);
		return 0;	
#if 0
		uint8_t buf[8]={0}, buf1[8]={0}, buf2[8]={0}, buf3[8]={0}, buf4[8]={0}, i;
		for(i = 0; i < 3; i++)
                {
 			buf[0]=0x1;
			ROBO_WREG(0, 0, 0x2, 0x3, (uint64_t *)buf, (unsigned int)1);
			buf[8]=0;

			buf[0]=0x7;
			ROBO_WREG(0, 0, 0x0, 0xb, (uint64_t *)buf, (unsigned int)1);
			buf[8]=0;

			buf[0]=0x82;
			ROBO_WREG(0, 0, 0x2, 0x0, (uint64_t *)buf, (unsigned int)1);
			buf[8]=0;

			buf[0]=0x1c;
			ROBO_WREG(0, 0, 0x0, 0x8, (uint64_t *)buf, (unsigned int)1);
			mdelay(1000);

			ROBO_RREG(0, 0, 0x2, 0x3, (uint64_t *)buf1, (unsigned int)1);
			ROBO_RREG(0, 0, 0x0, 0xb, (uint64_t *)buf2, (unsigned int)1);
			ROBO_RREG(0, 0, 0x2, 0x0, (uint64_t *)buf3, (unsigned int)1);
			ROBO_RREG(0, 0, 0x0, 0x8, (uint64_t *)buf4, (unsigned int)1);

			if((buf1[0] == 0x1) && (buf2[0] == 0x7) && (buf3[0] == 0x82) && (buf4[0] == 0x1c))
			{
				printk("[ %x %x %x %x ]\n",buf1[0], buf2[0], buf3[0], buf4[0]);
				printk("IMP port functionality ENABLED Successfully\n");
				return 0;
			}
			else
			{
				printk("[ %x %x %x %x ]\n",buf1[0], buf2[0], buf3[0], buf4[0]);
				printk("IMP port functionality ENABLED failed ... try again count(%d)\n", i);
                        	mdelay(1000);
                        	continue;
                	}

                }
		printk("[ %x %x %x %x ]\n",buf1[0], buf2[0], buf3[0], buf4[0]);
               	return 0;
#endif
	}
	else if(pstVidObjCmdMsg->commandMsg == IMP_Disable)
	{
                printk("DISABLE IMP port functionality\n");
		/*** Disable IMP port settting for robo switch BCM53128 and BCM53134
		setreg 0x02 0x03 0x0
		setreg 0x0 0xb 0x7 
		setreg 0x02 0x0 0x82 //As per BCM mail
		setreg 0x0 0x08 0x1c     
		***/
		uint8_t buf[8]={0}, buf1[8]={0}, buf2[8]={0}, buf3[8]={0}, buf4[8]={0};

		buf[0] = 0x0;
		ROBO_WREG(0, 0, 0x02, 0x03, (uint64_t *)buf, (unsigned int)1);
		
		buf[0] = 0x7;
		ROBO_WREG(0, 0, 0x0, 0xb, (uint64_t *)buf, (unsigned int)1);

		buf[0]=0x80;
		ROBO_WREG(0, 0, 0x2, 0x0, (uint64_t *)buf, (unsigned int)1);

		buf[0] = 0x1c;
		ROBO_WREG(0, 0, 0x0, 0x8, (uint64_t *)buf, (unsigned int)2);


		ROBO_RREG(0, 0, 0x2, 0x3, (uint64_t *)buf1, (unsigned int)1);
		ROBO_RREG(0, 0, 0x0, 0xb, (uint64_t *)buf2, (unsigned int)1);
		ROBO_RREG(0, 0, 0x2, 0x0, (uint64_t *)buf3, (unsigned int)2);
		ROBO_RREG(0, 0, 0x0, 0x8, (uint64_t *)buf4, (unsigned int)2);
		printk("IMP Register [ %x %x %02x %02x ]",buf1[0], buf2[0], buf3[0], buf4[0]);
		
		return 0;	
	}
	else if(pstVidObjCmdMsg->commandMsg == QosStats)
        {
        	uint32_t   Q0[8] = {0};
		uint32_t   Q1[8] = {0};
		uint32_t   Q2[8] = {0};
		uint32_t   Q3[8] = {0};
		uint16_t page = 0;

		if(pstVidObjCmdMsg->port ==0)
		{
			page=0x20;
		}
		if(pstVidObjCmdMsg->port == 1)
		{
			page=0x21;
		}
		if(pstVidObjCmdMsg->port == 2)
		{
			page=0x22;
		}
		if(pstVidObjCmdMsg->port == 3)
		{
			page=0x23;
		}
		if(pstVidObjCmdMsg->port == 4)
		{
			page=0x24;
		}
		if(pstVidObjCmdMsg->port == 5)
		{
			page=0x25;
		}
		if(pstVidObjCmdMsg->port == 6)
		{
			page=0x26;
		}
		if(pstVidObjCmdMsg->port == 7)
		{
			page=0x27;
		}

		ROBO_RREG(0, 0, page, 0x0c,  (uint64_t *)Q0, (uint8_t)4);
		ROBO_RREG(0, 0, page, 0x3c,  (uint64_t *)Q1, (uint8_t)4);
		ROBO_RREG(0, 0, page, 0x40,  (uint64_t *)Q2, (uint8_t)4);
		ROBO_RREG(0, 0, page, 0x44,  (uint64_t *)Q3, (uint8_t)4);

		pstVidObjCmdMsg->Qos_stats[0] = Q0[0];
		pstVidObjCmdMsg->Qos_stats[1] = Q1[0];
		pstVidObjCmdMsg->Qos_stats[2] = Q2[0];
		pstVidObjCmdMsg->Qos_stats[3] = Q3[0];
	}
 	else if(pstVidObjCmdMsg->commandMsg == Default_Reg)
        {
		uint8_t   buf[8] = {0};
		/* Auto power down register */
		buf[0] = 0x3d;
		buf[1] = 0x94;
		ROBO_WREG(0, 0, 0x10, 0x38, (uint64_t *)buf, (unsigned int)2);
		
		mdelay(1000);
		/* EEE register */
		buf[0] = 0xff;
		buf[1] = 0x1;
		ROBO_WREG(0, 0, 0x92, 0x0, (uint64_t *)buf, (unsigned int)2);
		mdelay(1000);
#if 0
		/* STP registers */
		buf[0] = 0x2;
		ROBO_WREG(0, 0, 0x04, 0x0e, (uint64_t *)buf, (unsigned int)2);

		/* DMAC for STP packets */
		buf[0] = 0x00;
		buf[1] = 0x00;
		buf[2] = 0x00;
		buf[3] = 0xc2;
		buf[4] = 0x80;
		buf[5] = 0x01;
		ROBO_WREG(0, 0, 0x04, 0x10, (uint64_t *)buf, (unsigned int)6);
	
		/* Ports on which STP packet should be sent once DMAC matched */
		buf[0] = 0xff;
		ROBO_WREG(0, 0, 0x04, 0x18, (uint64_t *)buf, (unsigned int)1);
#endif
		/* MAC aging set to default 300 seconds */ 
		buf[0] = 0x2c;
		buf[1] = 0x01;
		buf[2] = 0x0;
		ROBO_WREG(0, 0, 0x02, 0x06, (uint64_t *)buf, (unsigned int)3);

	}
	

	return 0;
}



static long av_vid_ioctl(struct file *filp, unsigned int uiVidCmd, unsigned long arg)
{
	int         rc    = 0;
	void __user *argp = (void __user *)arg;
	ROBO_OBJ_CMD_MSG stVidObjCmdMsg;

	switch( uiVidCmd )
	{
		case ROBO_IOC_CMD:
			if( copy_from_user( &stVidObjCmdMsg, argp, sizeof( stVidObjCmdMsg ) ) )
			{	
				printk("The copy from user failed\r\n");
				rc = -EFAULT;
			}
			else if( robo_CmdAction( &stVidObjCmdMsg ) != 0 )
			{
				printk("robo_CmdAction failed\r\n");
				rc = -EFAULT;
			}
			else if( copy_to_user( argp, &stVidObjCmdMsg, sizeof( stVidObjCmdMsg ) ) )
			{
				printk("copy_to_user failed\r\n");
				rc = -EFAULT;
			}
		break;
	}
	return rc;
}

static int __init nxp_robo_cdev_init(void)
{
	printk("nxp_robo_cdev_init: Entered\n");

	int result, err = -1;

	result = alloc_chrdev_region(&first, 0, NUMBER_OF_MINOR_DEVICE, DEVNAME);
	if( result < 0 )
	{
		printk("Error in allocating device");
		return -1;
	}
	
	cdev_init(&c_dev, &fops);
	err = cdev_add(&c_dev, first, 1);
	if (err)
		printk (KERN_NOTICE "Couldn't add cdev");
  	
	printk("nxp_robo_cdev_init() RETURN 0\n");
	return 0;
}

static void __exit nxp_robo_cdev_exit(void)
{

   	printk("nxp_robo_cdev_exit : Entered\n");
	cdev_del(&c_dev);
	unregister_chrdev_region(first, NUMBER_OF_MINOR_DEVICE);
	printk("nxp_robo_cdev_exit\n");
}

module_init(nxp_robo_cdev_init);
module_exit(nxp_robo_cdev_exit);
MODULE_LICENSE("GPL");

