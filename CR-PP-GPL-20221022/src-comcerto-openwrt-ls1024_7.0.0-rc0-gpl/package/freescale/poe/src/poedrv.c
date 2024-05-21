/******************************************************************************
 * 2017-02-20:Rajeshkumar K <rajeshkumar.k@nxp.com>                           *
 * PoE application driver to configure and control the pd69104b               *
 ******************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <mach/gpio.h>
#include <linux/string.h>
#include <asm/io.h>
/* ---------- pd69104b reg ---------- */
#define INT                     0x00
#define INTMASK                 0x01
#define PWREVN                  0x02
#define PWREVN_COR              0x03
#define DETEVN                  0x04
#define DETEVN_COR              0x05
#define FLTEVN                  0x06
#define FLTEVN_COR              0x07
#define TSEVN                   0x08
#define TSEVN_COR               0x09
#define SUPEVN                  0x0A
#define SUPEVN_COR              0x0B
#define STATP1                  0x0C
#define STATP2                  0x0D
#define STATP3                  0x0E
#define STATP4                  0x0F
#define STATPWR                 0x10
#define STATPIN                 0x11
#define OPMD                    0x12
#define DISENA                  0x13
#define DETENA                  0x14
#define MIDSPAN                 0x15
#define MCONF                   0x17
#define DETPB                   0x18
#define PWRPB                   0x19
#define RSTPB                   0x1A
#define ID                      0x1B
#define TLIM12                  0x1E
#define TLIM34                  0x1F
#define IP1LSB                  0x30
#define IP1MSB                  0x31
#define VP1LSB                  0x32
#define VP1MSB                  0x33
#define IP2LSB                  0x34
#define IP2MSB                  0x35
#define VP2LSB                  0x36
#define VP2MSB                  0x37
#define IP3LSB                  0x38
#define IP3MSB                  0x39
#define VP3LSB                  0x3A
#define VP3MSB                  0x3B
#define IP4LSB                  0x3C
#define IP4MSB                  0x3D
#define VP4LSB                  0x3E
#define VP4MSB                  0x3F
#define FIRMWARE                0x41
#define WDOG                    0x42
#define DEVID                   0x43
#define HPEN                    0x44
#define HPMD1                   0x46
#define CUT1                    0x47
#define LIM1                    0x48
#define HPSTAT1                 0x49
#define HPMD2                   0x4B
#define CUT2                    0x4C
#define LIM2                    0x4D
#define HPSTAT2                 0x4E
#define HPMD3                   0x50
#define CUT3                    0x51
#define LIM3                    0x52
#define HPSTAT3                 0x53
#define HPMD4                   0x55
#define CUT4                    0x56
#define LIM4                    0x57
#define HPSTAT4                 0x58
#define VTEMP                   0x70
#define VMAIN_LSB               0x71
#define VMAIN_MSB               0x72
#define PORT_SR12               0x75
#define PORT_SR34               0x76
#define INVD_CNT                0x77
#define PWRD_CNT                0x78
#define OVL_CNT                 0x79
#define UDL_CNT                 0x7A
#define SC_CNT                  0x7B
#define CLS_CNT                 0x7C
#define INTR_EN                 0x7D
#define SYS_CFG                 0x7E
#define SW_CFG                  0x7F
#define PRIO_CR                 0x80
#define PWR_CR1                 0x81
#define PWR_CR2                 0x82
#define PWR_CR3                 0x83
#define PWR_CR4                 0x84
#define TMP_PWR_CR1             0x85
#define TMP_PWR_CR2             0x86
#define TMP_PWR_CR3             0x87
#define TMP_PWR_CR4             0x88
#define PWR_BNK0                0x89
#define PWR_BNK1                0x8A
#define PWR_BNK2                0x8B
#define PWR_BNK3                0x8C
#define PWR_BNK4                0x8D
#define PWR_BNK5                0x8E
#define PWR_BNK6                0x8F
#define PWR_BNK7                0x90
#define PWRGD                   0x91
#define PORT1_CONS              0x92
#define PORT2_CONS              0x93
#define PORT3_CONS              0x94
#define PORT4_CONS              0x95
#define TOTAL_PWR_CONS          0x96
#define TOTAL_PWR_CALC          0x97
#define CHIP_PWR_REQ            0x98
#define ICUT_AT_MAX_LSB         0x99
#define ICUT_AT_MAX_MSB         0x9A
#define POE_MAX_LED_GB          0x9F
#define VMIAN_LOW_TH_LSB        0xCB
#define VMIAN_LOW_TH_MSB        0xCC
/* ---------- pd69104b reg ---------- */
#define I2C_READ       0
#define I2C_WRITE      1
#define I2C_READ_FAILED      -1

static struct i2c_client *g_client = NULL;
                   
#define POE_PORT_INDEX_0 0
#define POE_PORT_INDEX_1 1
#define POE_PORT_INDEX_2 2
#define POE_PORT_INDEX_3 3

#define BIT_ON_0_4  0x11
#define BIT_ON_1_5  0x22
#define BIT_ON_2_6  0x44
#define BIT_ON_3_7  0x88
#define BIT_ON_0_1  0x03 
#define BIT_ON_2_3  0x0C
#define BIT_ON_4_5  0x30
#define BIT_ON_6_7  0xC0


//#define POE_EXTRA_COMMAND // enable for extra commands
//#define POE_DEBUG //debug logs

#define MAX_POE_PORTS    4
#define MAX_POWER_BUDGET 0x3C    // 60W

#define MAX_PORT_POWER_IN_W               0x1E // 30W
#define MAX_PORT_POWER_IN_MW              30000
#define MAX_DEFAULT_PORT_MODE_POWER_IN_MW 15000

#define PORT_LIMIT_MODE  1
#define CLASS_LIMIT_MODE 2

#define POWER_PRIORITY_CRITICAL 1
#define POWER_PRIORITY_HIGH     2
#define POWER_PRIORITY_LOW      3
    
#define POE_STD_DISP_STR_AF "802.3 AF"
#define POE_STD_DISP_STR_AT "802.3 AT"

typedef struct poeport
{
    int8_t power_status;
    int8_t prev_power_status;
    int8_t admin_status;
    int8_t power_priority;
    int32_t admin_power_alloc;
}poeport_t;

typedef struct poeglobal
{
    int8_t portmode;
    int8_t legacy_enabled;
    int8_t trap_enabled;
    int8_t trap_threshold;
    struct poeport port[MAX_POE_PORTS];
}poeglobal_t;

static struct poeglobal poe_db_g;
static struct poeglobal poe_db_p;

int pse_config_on_g;
int Log_gen_g;
/*
 *  Cisco shared Tesla switch power range for the class are,
 *  Class 0/1/2/3/4  30/4/7/15.4/30 Watts
 *
 * Result of last classification on Port 1. 0=Unknown; 1=Class 1; 2=Class 2;
 * 3=Class 3; 4=Class 4; 5=Reserved; 6=Class 0; 7=Over-current.
 */

int32_t poe_class_power_arr[8]={30,4,7,16,30,30,30,30};

static inline void poe_set_bit(u8 *x, int bitNum) {
        *x |= (1 << bitNum);
}

static inline void poe_clear_bit(u8 *x, int bitNum) {
        *x &= ~(1 << bitNum);
}


static inline int poe_check_bit(unsigned value, unsigned bitpos)
{
        return ((value & (1 << bitpos)) != 0);
}


static int poe_read(u8 reg)
{
    int ret, retry = 0;

    if(g_client == NULL){
        printk(KERN_ERR "poe i2c g_client don't exist\n");
        return -1;
    }
    ret = i2c_smbus_read_byte_data(g_client,reg);
/*
 *LS1024A ASK 5.4.1 I2C repeat start function code,
 always generates double repeat start I2C signal.
 So, We tried to modify I2C repeat start function code to make sure
 I2C always send out one time repeat start signal once call I2C repeat function code,
 but the other issue happen, sometime I2C control respond error  happen( hit rate
 So we have I2C retry for I2C bus error detection.

 For 100KHz, polling  every 3 sec seen failure for 50, so increased to 150.
 This retry is upon failures,so no issue with the count
  */
    while((ret < 0) && (retry < 150))
    {
        retry++;
        ret = i2c_smbus_read_byte_data(g_client,reg);
    }

#ifdef POE_DEBUG
    if(ret < 0)
    {
        printk(KERN_ERR "poe i2c read fail! reg=0x%x\n",reg);
    }
#endif
    return ret;
}

static int poe_write(u8 reg, u8 val)
{
    int ret, retry = 0;

    if(g_client == NULL){
        printk(KERN_ERR "poe i2c g_client don't exist(write)\n");
        return -1;
    }
    ret = i2c_smbus_write_byte_data(g_client,reg,val);

    while((ret < 0) && (retry < 150))
    {
        retry++;
        ret = i2c_smbus_write_byte_data(g_client,reg,val);
    }

#ifdef POE_DEBUG
    if(ret < 0)
    {
        printk(KERN_ERR "poe i2c write fail! reg =0x%x\n",reg);
    }
#endif
    return ret;
}

/**************************************************************/
int poe_config(u8 ctrl, u8 reg, u8 val)
{
    u8 output_val=0;
    switch(ctrl)
    {
        case I2C_READ:
            output_val = poe_read(reg);
            printk("read reg: 0x%x\n",reg);
            printk("read value: 0x%x\n",output_val);
            break;
        case I2C_WRITE:
            output_val = poe_read(reg);
            printk("write reg: 0x%x\n",reg);
            printk("Before write value: 0x%x\n",output_val);
            output_val = val;
            poe_write(reg,output_val);
            printk("ready to write value: 0x%x\n",output_val);
            break;
    }
    return 0;
}
EXPORT_SYMBOL_GPL(poe_config);


static int poe_reg_show(struct seq_file *seq, void *v)
{
   seq_printf(seq,
            "Usage:\n"
            "poe read:  echo r reg > /proc/poe/poe_reg\n"
            "poe write: echo w reg val > /proc/poe/poe_reg\n");

    return 0;
}

static int poe_reg_write(struct file *file, const char __user * buffer, size_t count, loff_t *ppos)
{
    char line[64];
    char empty= ' ';
    char *ep = &empty;
    char *p1, *p2;
    u8 reg, write_val, read_val;
    int ret;

    ret = copy_from_user(line, buffer, count);
    if(ret)
        return -EFAULT;

    if(!strncmp(line, "w", 1))
    {
        p1 = strchr(line, ' ');
        if(p1 != NULL)
        {
            p1++;
            reg = simple_strtoul(p1, &ep, 0);
            p2 = strchr(p1, ' ');
            if(p2 != NULL)
            {
                p2++;
                write_val = simple_strtoul(p2, &ep, 0);
                poe_write(reg, write_val);
            }
        }
    }
    else if(!strncmp(line, "r", 1))
    {
        p1 = strchr(line, ' ');
        if(p1 != NULL)
        {
            p1++;
            reg = simple_strtoul(p1, &ep, 0);
            printk("poe read: reg=0x%x\n", reg);
            read_val = poe_read(reg);
            printk("read value=0x%x\n", read_val);
        }
    }

    return count;
}

static int poe_portpower_show(struct seq_file *seq, void *v)
{
    int val;
    int consumed_power;
    int ther=0; 

    /* return, during  pse configuration, avoid read calls */
    if(pse_config_on_g)
    {
      seq_printf(seq,"%d:",9);
      return 0;
    }

    val =  poe_read(STATPWR);

     //store the power status    
    poe_db_g.port[0].power_status = ((val&BIT_ON_0_4)?1:0);
    poe_db_g.port[1].power_status = ((val&BIT_ON_1_5)?1:0);
    poe_db_g.port[2].power_status = ((val&BIT_ON_2_6)?1:0);
    poe_db_g.port[3].power_status = ((val&BIT_ON_3_7)?1:0);


    // print the power status
    seq_printf(seq,"%d:",poe_db_g.port[0].power_status);
    seq_printf(seq,"%d:",poe_db_g.port[1].power_status);
    seq_printf(seq,"%d:",poe_db_g.port[2].power_status);
    seq_printf(seq,"%d:",poe_db_g.port[3].power_status);

    // print the prev power status
    seq_printf(seq,"%d:",poe_db_g.port[0].prev_power_status);
    seq_printf(seq,"%d:",poe_db_g.port[1].prev_power_status);
    seq_printf(seq,"%d:",poe_db_g.port[2].prev_power_status);
    seq_printf(seq,"%d:",poe_db_g.port[3].prev_power_status);

    // update the prev power status with current
    poe_db_g.port[0].prev_power_status = poe_db_g.port[0].power_status;
    poe_db_g.port[1].prev_power_status = poe_db_g.port[1].power_status;
    poe_db_g.port[2].prev_power_status = poe_db_g.port[2].power_status;
    poe_db_g.port[3].prev_power_status = poe_db_g.port[3].power_status;

    consumed_power = poe_read(TOTAL_PWR_CONS);
    if( consumed_power && ((poe_db_g.trap_threshold*(MAX_POWER_BUDGET*10)) < consumed_power*1000))
    {
        if(Log_gen_g == 0) // generate log/trap only once for the thresold  hit
        {
          Log_gen_g = 1;
          ther = consumed_power*1000;
        }
        else // thresold hit case, log is already generated.
          ther = 0;
    }
    else if( consumed_power && ((poe_db_g.trap_threshold*(MAX_POWER_BUDGET*10)) > consumed_power*1000))
    {// Consumed power is less than the threshold, reset the log_gen flag.
        Log_gen_g = 0;
        ther = 0;
    }
    seq_printf(seq,"%u:",ther);
    seq_printf(seq,"%d\n",poe_db_g.trap_enabled?1:0);

    return 0;
}

static int poe_setconf_show(struct seq_file *seq, void *v)
{
    return 0;
}

static int poe_portpower_write(struct file *file, const char __user * buffer, size_t count,loff_t * ppos)
{
    char line[64];
    u8 operation=0,detection=0;
    int ret,port=0;

    ret = copy_from_user(line, buffer, count);
    if(ret)
        return -EFAULT;


    do
    {
        switch(port)
        {
            case POE_PORT_INDEX_0:
                if(line[port] == '1')
                {
                    operation |= BIT_ON_0_1;
                    detection |= BIT_ON_0_4; 
                }
                break;

            case POE_PORT_INDEX_1:
                if(line[port] == '1')
                {
                    operation |= BIT_ON_2_3;
                    detection |= BIT_ON_1_5; 
                }
                break;

            case POE_PORT_INDEX_2:
                if(line[port] == '1')
                {
                    operation |= BIT_ON_4_5;
                    detection |= BIT_ON_2_6; 
                }
                break;

            case POE_PORT_INDEX_3:
                if(line[port] == '1')
                {
                    operation |= BIT_ON_6_7;
                    detection |= BIT_ON_3_7; 
                }
                break;
            default:
                break;
        }
        port++;
    }while(port < MAX_POE_PORTS);

    //set Operation mode
    poe_write(OPMD,operation);
    //Enable Detection on all ports
    poe_write(DETENA,detection);

    return count;
}

static int poe_setconf_write(struct file *file, const char __user * buffer, size_t count,loff_t * ppos)
{
    char buf[64];
    char *line;
    u8 operation=0,detection=0,legacy_rval=0,priority_rval=0;
    int ret=0;
    const char *sep = ":";
    char *cp;
    int reset_power=0;
    u8 legacy_reg=0;
    u8 sw_cfg=0;
    //u8 pwr1,pwr2,pwr3,pwr4=0;
    u8 hpen_val=0;

    ret = copy_from_user(buf, buffer, count);
    if(ret)
        return -EFAULT;

    pse_config_on_g = 1;

    line=buf;
    cp = strsep(&line, sep);
    if (cp && *cp)
        poe_db_g.portmode = (int8_t)simple_strtol(cp, NULL, 0);

    cp = strsep(&line, sep);
    if (cp && *cp)
        poe_db_g.legacy_enabled  = (int8_t)simple_strtol(cp, NULL, 0);

    cp = strsep(&line, sep);
    if (cp && *cp)
        poe_db_g.trap_enabled  = (int8_t)simple_strtol(cp, NULL, 0);

    // Generate Trap, upon enabling the trap flag . reset log generated flag.
    if(poe_db_g.trap_enabled)
      Log_gen_g = 0;

    cp = strsep(&line, sep);
    if (cp && *cp)
        poe_db_g.trap_threshold  = (int8_t)simple_strtol(cp, NULL, 0);

    cp = strsep(&line, sep);
    if (cp && *cp)
        poe_db_g.port[0].admin_status  = (int8_t)simple_strtol(cp, NULL, 0);

    cp = strsep(&line, sep);
    if (cp && *cp)
        poe_db_g.port[0].power_priority  = (int8_t)simple_strtol(cp, NULL, 0);

    cp = strsep(&line, sep);
    if (cp && *cp)
        poe_db_g.port[0].admin_power_alloc  = (int32_t)simple_strtol(cp, NULL, 0);

    cp = strsep(&line, sep);
    if (cp && *cp)
        poe_db_g.port[1].admin_status  = (int8_t)simple_strtol(cp, NULL, 0);

    cp = strsep(&line, sep);
    if (cp && *cp)
        poe_db_g.port[1].power_priority  = (int8_t)simple_strtol(cp, NULL, 0);

    cp = strsep(&line, sep);
    if (cp && *cp)
        poe_db_g.port[1].admin_power_alloc  = (int32_t)simple_strtol(cp, NULL, 0);
    cp = strsep(&line, sep);
    if (cp && *cp)
        poe_db_g.port[2].admin_status  = (int8_t)simple_strtol(cp, NULL, 0);

    cp = strsep(&line, sep);
    if (cp && *cp)
        poe_db_g.port[2].power_priority  = (int8_t)simple_strtol(cp, NULL, 0);

    cp = strsep(&line, sep);
    if (cp && *cp)
        poe_db_g.port[2].admin_power_alloc  = (int32_t)simple_strtol(cp, NULL, 0);
    cp = strsep(&line, sep);
    if (cp && *cp)
        poe_db_g.port[3].admin_status  = (int8_t)simple_strtol(cp, NULL, 0);

    cp = strsep(&line, sep);
    if (cp && *cp)
        poe_db_g.port[3].power_priority  = (int8_t)simple_strtol(cp, NULL, 0);

    cp = strsep(&line, sep);
    if (cp && *cp)
        poe_db_g.port[3].admin_power_alloc  = (int32_t)simple_strtol(cp, NULL, 0);

    /* check changes of the power values and set reset_power  flag and port shutdown */
    if((poe_db_g.portmode != poe_db_p.portmode) ||
        (poe_db_g.legacy_enabled != poe_db_p.legacy_enabled) ||
        (poe_db_g.port[0].power_priority != poe_db_p.port[0].power_priority) ||
        (poe_db_g.port[1].power_priority != poe_db_p.port[1].power_priority) ||
        (poe_db_g.port[2].power_priority != poe_db_p.port[2].power_priority) ||
        (poe_db_g.port[3].power_priority != poe_db_p.port[3].power_priority) ||
        (poe_db_g.port[0].admin_power_alloc != poe_db_p.port[0].admin_power_alloc) ||
        (poe_db_g.port[1].admin_power_alloc != poe_db_p.port[1].admin_power_alloc) ||
        (poe_db_g.port[2].admin_power_alloc != poe_db_p.port[2].admin_power_alloc) ||
        (poe_db_g.port[3].admin_power_alloc != poe_db_p.port[3].admin_power_alloc) ||
        ((poe_db_g.port[0].admin_status) != (poe_db_p.port[0].admin_status)) ||
        ((poe_db_g.port[1].admin_status) != (poe_db_p.port[1].admin_status)) ||
        ((poe_db_g.port[2].admin_status) != (poe_db_p.port[2].admin_status)) ||
        ((poe_db_g.port[3].admin_status) != (poe_db_p.port[3].admin_status)))
    {
      poe_write(OPMD,0);   //Disable all the ports
     // mdelay(50);  // delay between shutdwon and powerup
      reset_power =1;
    }

    if(poe_db_g.portmode != poe_db_p.portmode)
    {

        sw_cfg = poe_read(SW_CFG);
        if(poe_db_g.portmode == CLASS_LIMIT_MODE)
        {
            /* power management - static   */
            poe_clear_bit(&sw_cfg,1);
        }
        else if(poe_db_g.portmode == PORT_LIMIT_MODE)
        {
            /* power management - Dynamic */
            poe_set_bit(&sw_cfg,1);
        }
        poe_write(SW_CFG,sw_cfg);
    }

    if(poe_db_g.portmode == CLASS_LIMIT_MODE)
    {
      /* as the class 3 and 4 are above 15w,
       * set the high power feature enabled for all ports in class mode*/
      poe_write(HPEN,0xF);
    }
    else if(poe_db_g.portmode == PORT_LIMIT_MODE)
    {
      /* power management - Dynamic */
      if(poe_db_g.port[0].admin_power_alloc > MAX_DEFAULT_PORT_MODE_POWER_IN_MW)
        poe_set_bit(&hpen_val,0);
      if(poe_db_g.port[1].admin_power_alloc > MAX_DEFAULT_PORT_MODE_POWER_IN_MW)
        poe_set_bit(&hpen_val,1);
      if(poe_db_g.port[2].admin_power_alloc > MAX_DEFAULT_PORT_MODE_POWER_IN_MW)
        poe_set_bit(&hpen_val,2);
      if(poe_db_g.port[3].admin_power_alloc > MAX_DEFAULT_PORT_MODE_POWER_IN_MW)
        poe_set_bit(&hpen_val,3);

      // DNI PoE Tester, High current spike leads to bringup the 4th port
      // Disable the High power feature, if the PPL is <= 15000
      poe_write(HPEN,hpen_val);
    }

    /* The values on HPMD[1..4] is bit[1]='1' for legacy enable. 
     * SYS_CFG  0x7E bit[0]='0' in order to support legacy detection.  */
    if(poe_db_g.legacy_enabled != poe_db_p.legacy_enabled)
    {
        legacy_reg = poe_read(SYS_CFG);
        if(poe_db_g.legacy_enabled)
        {
            legacy_rval = 0x03;
            poe_clear_bit(&legacy_reg,0);
        }
        else
        {
            legacy_rval = 0x01;
            poe_set_bit(&legacy_reg,0);
        }
        poe_write(SYS_CFG,legacy_reg);

        poe_write(HPMD1,legacy_rval);
        poe_write(HPMD2,legacy_rval);
        poe_write(HPMD3,legacy_rval);
        poe_write(HPMD4,legacy_rval);
    }

    /* set port priority */
    if((poe_db_g.port[0].power_priority != poe_db_p.port[0].power_priority) || 
            (poe_db_g.port[1].power_priority != poe_db_p.port[1].power_priority) || 
            (poe_db_g.port[2].power_priority != poe_db_p.port[2].power_priority) || 
            (poe_db_g.port[3].power_priority != poe_db_p.port[3].power_priority))
    {
        //  priority_rval= poe_read(PRIO_CR);

        switch(poe_db_g.port[0].power_priority)
        {
            case POWER_PRIORITY_CRITICAL:
                poe_clear_bit(&priority_rval,0);
                poe_clear_bit(&priority_rval,1);
                break;
            case POWER_PRIORITY_HIGH:
                poe_set_bit(&priority_rval,0);
                break;
            case POWER_PRIORITY_LOW:
                poe_set_bit(&priority_rval,1);
                break;
            default:
                break;
        }

        switch(poe_db_g.port[1].power_priority)
        {
            case POWER_PRIORITY_CRITICAL:
                poe_clear_bit(&priority_rval,2);
                poe_clear_bit(&priority_rval,3);
                break;
            case POWER_PRIORITY_HIGH:
                poe_set_bit(&priority_rval,2);
                break;
            case POWER_PRIORITY_LOW:
                poe_set_bit(&priority_rval,3);
                break;
            default:
                break;
        }

        switch(poe_db_g.port[2].power_priority)
        {
            case POWER_PRIORITY_CRITICAL:
                poe_clear_bit(&priority_rval,4);
                poe_clear_bit(&priority_rval,5);
                break;
            case POWER_PRIORITY_HIGH:
                poe_set_bit(&priority_rval,4);
                break;
            case POWER_PRIORITY_LOW:
                poe_set_bit(&priority_rval,5);
                break;
            default:
                break;
        }

        switch(poe_db_g.port[3].power_priority)
        {
            case POWER_PRIORITY_CRITICAL:
                poe_clear_bit(&priority_rval,6);
                poe_clear_bit(&priority_rval,7);
                break;
            case POWER_PRIORITY_HIGH:
                poe_set_bit(&priority_rval,6);
                break;
            case POWER_PRIORITY_LOW:
                poe_set_bit(&priority_rval,7);
                break;
            default:
                break;
        }

        poe_write(PRIO_CR,priority_rval);
    } 

 /*   if( (poe_db_g.legacy_enabled) && (poe_db_g.portmode == CLASS_LIMIT_MODE) )
    {
      poe_write(PWR_CR1,MAX_PORT_POWER_IN_W);
      poe_write(TMP_PWR_CR1,MAX_PORT_POWER_IN_W);
      poe_write(PWR_CR2,MAX_PORT_POWER_IN_W);
      poe_write(TMP_PWR_CR2,MAX_PORT_POWER_IN_W);
      poe_write(PWR_CR3,MAX_PORT_POWER_IN_W);
      poe_write(TMP_PWR_CR3,MAX_PORT_POWER_IN_W);
      poe_write(PWR_CR4,MAX_PORT_POWER_IN_W);
      poe_write(TMP_PWR_CR4,MAX_PORT_POWER_IN_W);
    }
    else */ // removing the legacy 30w ppl alloc
    {
      /* set power limit */
      if(poe_db_g.port[0].admin_power_alloc != poe_db_p.port[0].admin_power_alloc)
      {
        poe_write(PWR_CR1,(poe_db_g.port[0].admin_power_alloc)/1000);
        poe_write(TMP_PWR_CR1,(poe_db_g.port[0].admin_power_alloc)/1000);
      }

      if(poe_db_g.port[1].admin_power_alloc != poe_db_p.port[1].admin_power_alloc)
      {
        poe_write(PWR_CR2,(poe_db_g.port[1].admin_power_alloc)/1000);
        poe_write(TMP_PWR_CR2,(poe_db_g.port[1].admin_power_alloc)/1000);
      }

      if(poe_db_g.port[2].admin_power_alloc != poe_db_p.port[2].admin_power_alloc)
      {
        poe_write(PWR_CR3,(poe_db_g.port[2].admin_power_alloc)/1000);
        poe_write(TMP_PWR_CR3,(poe_db_g.port[2].admin_power_alloc)/1000);
      }

      if(poe_db_g.port[3].admin_power_alloc != poe_db_p.port[3].admin_power_alloc)
      {
        poe_write(PWR_CR4,(poe_db_g.port[3].admin_power_alloc)/1000);
        poe_write(TMP_PWR_CR4,(poe_db_g.port[3].admin_power_alloc)/1000);
      }
    }

     /* port Enable/Disalbe */ 
   /*  if(((poe_db_g.port[0].admin_status) != (poe_db_p.port[0].admin_status)) ||
            ((poe_db_g.port[1].admin_status) != (poe_db_p.port[1].admin_status)) ||
            ((poe_db_g.port[2].admin_status) != (poe_db_p.port[2].admin_status)) ||
            ((poe_db_g.port[3].admin_status) != (poe_db_p.port[3].admin_status))) */
    {
        if(poe_db_g.port[0].admin_status)
        {
            operation |= BIT_ON_0_1;
            detection |= BIT_ON_0_4; 
        }
        if(poe_db_g.port[1].admin_status)
        {
            operation |= BIT_ON_2_3;
            detection |= BIT_ON_1_5;
        }
        if(poe_db_g.port[2].admin_status)
        {
            operation |= BIT_ON_4_5;
            detection |= BIT_ON_2_6; 
        }
        if(poe_db_g.port[3].admin_status)
        {
            operation |= BIT_ON_6_7;
            detection |= BIT_ON_3_7;
        }
    }

   if(reset_power)
   {
        poe_write(OPMD,operation);   //set Operation mode 
        mdelay(5000); 
        poe_write(DETENA,detection); //Enable Detection 
   }


    memcpy(&poe_db_p,&poe_db_g,sizeof(poeglobal_t));
    pse_config_on_g = 0;
    return count;
}

/* This will be called from polling script every 3 second to update the PPL/TPPL for Class mode */
static int poe_getportstats_write(struct file *file, const char __user * user_buffer, size_t count, loff_t * ppos)
{
    //u8 power_stats;
    int val=0,portindex=0;
    //u8 hpen_val=0;

    /* return,  pse is ongoing configuration */
    if(pse_config_on_g)
    {
      return count;
    }

    if(poe_db_g.portmode == CLASS_LIMIT_MODE)
    {
      //power_stats =  poe_read(STATPWR);
     /* if(poe_db_g.legacy_enabled)
      {
        poe_write(PWR_CR1,MAX_PORT_POWER_IN_W);
        poe_write(TMP_PWR_CR1,MAX_PORT_POWER_IN_W);
        poe_write(PWR_CR2,MAX_PORT_POWER_IN_W);
        poe_write(TMP_PWR_CR2,MAX_PORT_POWER_IN_W);
        poe_write(PWR_CR3,MAX_PORT_POWER_IN_W);
        poe_write(TMP_PWR_CR3,MAX_PORT_POWER_IN_W);
        poe_write(PWR_CR4,MAX_PORT_POWER_IN_W);
        poe_write(TMP_PWR_CR4,MAX_PORT_POWER_IN_W);
      }
      else */ // removing the legacy enabled PPl/TPPL 30W
      {// set the class power value for legacy disabled case
        do
        {
          switch(portindex)
          {
            case POE_PORT_INDEX_0:
              // set PPL/TPPL irrespective of power on status.
              // if(poe_check_bit(power_stats,portindex))
              {
                val =  poe_read(STATP1);
                val =  (val & 0x70) >> 4;
                poe_write(PWR_CR1,poe_class_power_arr[val]);
                poe_write(TMP_PWR_CR1,poe_class_power_arr[val]);
              }
              break;

            case POE_PORT_INDEX_1:
              //if(poe_check_bit(power_stats,portindex))
              {
                val =  poe_read(STATP2);
                val =  (val & 0x70) >> 4;
                poe_write(PWR_CR2,poe_class_power_arr[val]);
                poe_write(TMP_PWR_CR2,poe_class_power_arr[val]);
              }
              break;

            case POE_PORT_INDEX_2:
              //if(poe_check_bit(power_stats,portindex))
              {
                val =  poe_read(STATP3);
                val =  (val & 0x70) >> 4;
                poe_write(PWR_CR3,poe_class_power_arr[val]);
                poe_write(TMP_PWR_CR3,poe_class_power_arr[val]);
              }
              break;

            case POE_PORT_INDEX_3:
              //if(poe_check_bit(power_stats,portindex))
              {
                val =  poe_read(STATP4);
                val =  (val & 0x70) >> 4;
                poe_write(PWR_CR4,poe_class_power_arr[val]);
                poe_write(TMP_PWR_CR4,poe_class_power_arr[val]);
              }
              break;

            default:
              break;
          }
          portindex++;
        }while(portindex < MAX_POE_PORTS);
      }
    } /* configure PPL and TPPL again for port mode, to avoid changing to class power level */
    else if(poe_db_g.portmode == PORT_LIMIT_MODE)
    {
      portindex=0;
      do
      {
        switch(portindex)
        {
          case POE_PORT_INDEX_0:
            {
              poe_write(PWR_CR1,(poe_db_g.port[0].admin_power_alloc)/1000);
              poe_write(TMP_PWR_CR1,(poe_db_g.port[0].admin_power_alloc)/1000);
            }
            break;

          case POE_PORT_INDEX_1:
            {
              poe_write(PWR_CR2,(poe_db_g.port[1].admin_power_alloc)/1000);
              poe_write(TMP_PWR_CR2,(poe_db_g.port[1].admin_power_alloc)/1000);
            }
            break;

          case POE_PORT_INDEX_2:
            {
              poe_write(PWR_CR3,(poe_db_g.port[2].admin_power_alloc)/1000);
              poe_write(TMP_PWR_CR3,(poe_db_g.port[2].admin_power_alloc)/1000);
            }
            break;

          case POE_PORT_INDEX_3:
            {
              poe_write(PWR_CR4,(poe_db_g.port[3].admin_power_alloc)/1000);
              poe_write(TMP_PWR_CR4,(poe_db_g.port[3].admin_power_alloc)/1000);
            }
            break;

          default:
            break;
        }
        portindex++;
      }while(portindex < MAX_POE_PORTS);
    }
	return count;
}

static int poe_getpowerstats_write(struct file *file, const char __user * user_buffer, size_t count, loff_t * ppos)
{
	return count;
}

static int poe_showallregs_write(struct file *file, const char __user * user_buffer, size_t count, loff_t * ppos)
{
	return count;
}

static int poe_showallstats_write(struct file *file, const char __user * user_buffer, size_t count, loff_t * ppos)
{
	return count;
}

static int poe_getportstats_show(struct seq_file *seq, void *v)
{
    int val,portindex=0,port_type;
    int over_load_cnt=0,under_load_cnt=0,short_circuit_cnt=0;
    int power_denied_cnt=0,invalid_cnt = 0;
    int val_low = 0, val_high = 0, cval = 0,vval=0,classindex,pval;
    int cur_base = 12207;     //122.07uA
    int vol_base = 5835;    //5.835mV
    u8 power_stats;

    /* read the poe type as same all the ports*/ 
    //val =  poe_read(SYS_CFG);
    //port_type =  (val & 0x80) >> 7;
    /* Check for the high power feature for AT configuration. Internal mapping 1-AT,0-AF */
    val =  poe_read(HPEN);

    /* read over load counter  */
    over_load_cnt =  poe_read(OVL_CNT);

    /* read under load counter  */
    under_load_cnt =  poe_read(UDL_CNT);

    /* read short circuit event counter  */
    short_circuit_cnt =  poe_read(SC_CNT);

    /* read power denied event counter  */
    power_denied_cnt =  poe_read(PWRD_CNT);

    /* read invalid event counter  */
    invalid_cnt =  poe_read(INVD_CNT);

    power_stats =  poe_read(STATPWR);
//    mdelay(100); // delay to avoid the immediate read upon write - I2C noise 100KHz
    do
    {
        switch(portindex)
        {
            case POE_PORT_INDEX_0:
                {
                    seq_printf(seq, "%d|",portindex);//port no

                    val =  poe_read(STATP1);
                    val =  (val & 0x70) >> 4;
                    /* Don't show the class name,
                     even port fail to power up due out of power budget and class detection is successful. */
                   /* PPSBR-1951 The PoE class should reset when unplug PD */
                    seq_printf(seq, "%s|",((power_stats&BIT_ON_0_4) == 0)?"0": (val == 0) ? "0" : ((val == 1) ? "1" : ((val == 2) ? "2" : ((val == 3) ? "3" : ((val == 4) ? "4" : ((val == 5) ? "0" : ((val == 6) ? "0" : "4")))))));//class

                    classindex = val;

                    //val =  poe_read(PORT1_CONS);
                   //seq_printf(seq, "%u|",(val*1000)); //consumed power

                    val_low  = poe_read(IP1LSB);
                    val_high = poe_read(IP1MSB);
                    val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
                    cval = val * cur_base;
                    val_low  = poe_read(VP1LSB);
                    val_high = poe_read(VP1MSB);
                    val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
                    vval = val * vol_base;

                    pval = ((cval/100000)*(vval/1000000));// power consumption

                    if(poe_db_g.portmode == PORT_LIMIT_MODE)
                    {
                      seq_printf(seq, "%u|",poe_db_g.port[portindex].admin_power_alloc); //max power
                    }
                    else if(poe_db_g.portmode == CLASS_LIMIT_MODE) // Show max power as the class range,if unpowered
                    {
                     /* if(poe_db_g.legacy_enabled)// legacy enabled case, show consumed as max power
                      {
                        seq_printf(seq, "%u|",(power_stats&BIT_ON_0_4)?(pval):(MAX_PORT_POWER_IN_MW));
                      }
                      else */// legacy disabled case, show the class max power range as max power alloc
                      {
                        seq_printf(seq, "%u|",(power_stats&BIT_ON_0_4)?(poe_class_power_arr[classindex]*1000):(MAX_PORT_POWER_IN_MW));
                      }
                    }

                    seq_printf(seq, "%u|",(power_stats&BIT_ON_0_4)?(pval):0);// power consumption

                    val = (over_load_cnt & BIT_ON_0_1);
                    seq_printf(seq, "%u|",val); //overload

                    val = (under_load_cnt & BIT_ON_0_1);
                    seq_printf(seq, "%u|",val); //absent_counter

                    val = (short_circuit_cnt & BIT_ON_0_1);
                    seq_printf(seq, "%u|",val); //short_counter

                    val = (power_denied_cnt & BIT_ON_0_1);
                    seq_printf(seq, "%u|",val); //denied_counter

                    val = (invalid_cnt & BIT_ON_0_1);
                    seq_printf(seq, "%u|",val); //invalid_signature_counter

                    seq_printf(seq, "%s|", (port_type == 0) ? POE_STD_DISP_STR_AF:POE_STD_DISP_STR_AT);
                }
                break;

            case POE_PORT_INDEX_1:
                {
                    seq_printf(seq, "%d|",portindex);//port no

                    val =  poe_read(STATP2);
                    val =  (val & 0x70) >> 4;
                    seq_printf(seq, "%s|", ((power_stats&BIT_ON_1_5) == 0)?"0": (val == 0) ? "0" : ((val == 1) ? "1" : ((val == 2) ? "2" : ((val == 3) ? "3" : ((val == 4) ? "4" : ((val == 5) ? "0" : ((val == 6) ? "0" : "4")))))));//class
                    classindex = val;

                    //val =  poe_read(PORT2_CONS);
                    //seq_printf(seq, "%u|",val); //consumed power
                    val_low  = poe_read(IP2LSB);
                    val_high = poe_read(IP2MSB);
                    val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
                    cval = val * cur_base;
                    val_low  = poe_read(VP2LSB);
                    val_high = poe_read(VP2MSB);
                    val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
                    vval = val * vol_base;
                    pval = ((cval/100000)*(vval/1000000));// power consumption

                    if(poe_db_g.portmode == PORT_LIMIT_MODE)
                    {
                      seq_printf(seq, "%u|",poe_db_g.port[portindex].admin_power_alloc); //max power
                    }
                    else if(poe_db_g.portmode == CLASS_LIMIT_MODE) // Show max power as the class range,if unpowered
                    {
                      /* if(poe_db_g.legacy_enabled)// legacy enabled case, show consumed as max power
                      {
                        seq_printf(seq, "%u|",(power_stats&BIT_ON_1_5)?(pval):(MAX_PORT_POWER_IN_MW));
                      }
                      else */// legacy disabled case, show the class max power range as max power alloc
                      {
                        seq_printf(seq, "%u|",(power_stats&BIT_ON_1_5)?(poe_class_power_arr[classindex]*1000):(MAX_PORT_POWER_IN_MW));
                      }
                    }

                    seq_printf(seq, "%u|",(power_stats&BIT_ON_1_5)?(pval):0);

                    val = (over_load_cnt & BIT_ON_2_3) >> 2;
                    seq_printf(seq, "%u|",val); //overload

                    val = (under_load_cnt & BIT_ON_2_3) >> 2;
                    seq_printf(seq, "%u|",val); //absent_counter

                    val = (short_circuit_cnt & BIT_ON_2_3) >> 2;
                    seq_printf(seq, "%u|",val); //short_counter

                    val = (power_denied_cnt & BIT_ON_2_3) >> 2;
                    seq_printf(seq, "%u|",val); //denied_counter

                    val = (invalid_cnt & BIT_ON_2_3) >> 2;
                    seq_printf(seq, "%u|",val); //invalid_signature_counter

                    seq_printf(seq, "%s|", (port_type == 0) ? POE_STD_DISP_STR_AF:POE_STD_DISP_STR_AT);
                }
                break;

            case POE_PORT_INDEX_2:
                {
                    seq_printf(seq, "%d|",portindex);//port no

                    val =  poe_read(STATP3);
                    val =  (val & 0x70) >> 4;
                    seq_printf(seq, "%s|",((power_stats&BIT_ON_2_6) == 0)?"0": (val == 0) ? "0" : ((val == 1) ? "1" : ((val == 2) ? "2" : ((val == 3) ? "3" : ((val == 4) ? "4" : ((val == 5) ? "0" : ((val == 6) ? "0" : "4")))))));//class
                    classindex = val;

                    //val =  poe_read(PORT3_CONS);
                    //seq_printf(seq, "%u|",val); //consumed power
                    val_low  = poe_read(IP3LSB);
                    val_high = poe_read(IP3MSB);
                    val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
                    cval = val * cur_base;
                    val_low  = poe_read(VP3LSB);
                    val_high = poe_read(VP3MSB);
                    val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
                    vval = val * vol_base;
                    pval = ((cval/100000)*(vval/1000000));// power consumption

                    if(poe_db_g.portmode == PORT_LIMIT_MODE)
                    {
                      seq_printf(seq, "%u|",poe_db_g.port[portindex].admin_power_alloc); //max power
                    }
                    else if(poe_db_g.portmode == CLASS_LIMIT_MODE) // Show max power as the class range,if unpowered
                    {
                      /* if(poe_db_g.legacy_enabled)// legacy enabled case, show consumed as max power
                      {
                        seq_printf(seq, "%u|",(power_stats&BIT_ON_2_6)?(pval):(MAX_PORT_POWER_IN_MW));
                      }
                      else */// legacy disabled case, show the class max power range as max power alloc
                      {
                        seq_printf(seq, "%u|",(power_stats&BIT_ON_2_6)?(poe_class_power_arr[classindex]*1000):(MAX_PORT_POWER_IN_MW));
                      }
                    }

                    seq_printf(seq, "%u|",(power_stats&BIT_ON_2_6)?(pval):0);

                    val = (over_load_cnt & BIT_ON_4_5) >> 4;
                    seq_printf(seq, "%u|",val); //overload

                    val = (under_load_cnt & BIT_ON_4_5) >> 4;
                    seq_printf(seq, "%u|",val); //absent_counter

                    val = (short_circuit_cnt & BIT_ON_4_5) >> 4;
                    seq_printf(seq, "%u|",val); //short_counter

                    val = (power_denied_cnt & BIT_ON_4_5) >> 4;
                    seq_printf(seq, "%u|",val); //denied_counter

                    val = (invalid_cnt & BIT_ON_4_5) >> 4;
                    seq_printf(seq, "%u|",val); //invalid_signature_counter

                    seq_printf(seq, "%s|", (port_type == 0) ?POE_STD_DISP_STR_AF:POE_STD_DISP_STR_AT );
                }
                break;

            case POE_PORT_INDEX_3:
                {
                    seq_printf(seq, "%d|",portindex);//port no

                    val =  poe_read(STATP4);
                    val =  (val & 0x70) >> 4;
                    seq_printf(seq, "%s|",((power_stats&BIT_ON_3_7) == 0)?"0": (val == 0) ? "0" : ((val == 1) ? "1" : ((val == 2) ? "2" : ((val == 3) ? "3" : ((val == 4) ? "4" : ((val == 5) ? "0" : ((val == 6) ? "0" : "4")))))));//class
                    classindex = val;

                    //val =  poe_read(PORT4_CONS);
                    //seq_printf(seq, "%u|",val); //consumed power
                    val_low  = poe_read(IP4LSB);
                    val_high = poe_read(IP4MSB);
                    val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
                    cval = val * cur_base;
                    val_low  = poe_read(VP4LSB);
                    val_high = poe_read(VP4MSB);
                    val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
                    vval = val * vol_base;
                    pval = ((cval/100000)*(vval/1000000));// power consumption

                    if(poe_db_g.portmode == PORT_LIMIT_MODE)
                    {
                      seq_printf(seq, "%u|",poe_db_g.port[portindex].admin_power_alloc); //max power
                    }
                    else if(poe_db_g.portmode == CLASS_LIMIT_MODE) // Show max power as the class range,if unpowered
                    {
                      /*if(poe_db_g.legacy_enabled)// legacy enabled case, show consumed as max power
                      {
                        seq_printf(seq, "%u|",(power_stats&BIT_ON_3_7)?(pval):(MAX_PORT_POWER_IN_MW));
                      }
                      else */// legacy disabled case, show the class max power range as max power alloc
                      {
                        seq_printf(seq, "%u|",(power_stats&BIT_ON_3_7)?(poe_class_power_arr[classindex]*1000):(MAX_PORT_POWER_IN_MW));
                      }
                    }

                    seq_printf(seq, "%u|",(power_stats&BIT_ON_3_7)?(pval):0);

                    val = (over_load_cnt & BIT_ON_6_7) >> 6;
                    seq_printf(seq, "%u|",val); //overload

                    val = (under_load_cnt & BIT_ON_6_7) >> 6;
                    seq_printf(seq, "%u|",val); //absent_counter

                    val = (short_circuit_cnt & BIT_ON_6_7) >> 6;
                    seq_printf(seq, "%u|",val); //short_counter

                    val = (power_denied_cnt & BIT_ON_6_7) >> 6;
                    seq_printf(seq, "%u|",val); //denied_counter

                    val = (invalid_cnt & BIT_ON_6_7) >> 6;
                    seq_printf(seq, "%u|",val); //invalid_signature_counter

                    seq_printf(seq, "%s\n", (port_type == 0) ?POE_STD_DISP_STR_AF:POE_STD_DISP_STR_AT );
                }
                break;

            default:
                break;
        }

        portindex++;
    }while(portindex < MAX_POE_PORTS);

    // printk(KERN_ERR "%s:return\n val;%s\n",__func__,seq->buf);
    return 0;
}

static int poe_getpowerstats_show(struct seq_file *seq, void *v)
{
    int consumed_power,avail_power;
    int allocated_power; 
    int state12;
    int state34;
    int max_power_budget;

    state12 =  poe_read(PORT_SR12);
    state34 =  poe_read(PORT_SR34);
    /* Port MOSFET over temperature event.
     * Port is OFF Due to PM
     * */
    if(poe_check_bit(state12,0) || poe_check_bit(state12,1) 
            || poe_check_bit(state12,4) || poe_check_bit(state12,5) 
            || poe_check_bit(state34,0) || poe_check_bit(state34,1) 
            || poe_check_bit(state34,4) || poe_check_bit(state34,5))
        seq_printf(seq,"%s|","1"); // POE state fault
    else
        seq_printf(seq,"%s|","0"); // POE state normal


    max_power_budget = MAX_POWER_BUDGET;//poe_read(PWR_BNK0);
    seq_printf(seq,"%d|",max_power_budget);

    /* to adjust the 1w difference b/wthe configured 61w and displayed 60w */
    allocated_power =  poe_read(TOTAL_PWR_CALC);
    allocated_power = ((allocated_power > max_power_budget)?max_power_budget:allocated_power);

    consumed_power =  poe_read(TOTAL_PWR_CONS);
    consumed_power = ((consumed_power > max_power_budget)?max_power_budget:consumed_power);

    seq_printf(seq,"%u|", consumed_power);
    seq_printf(seq,"%u|", allocated_power);

    if(poe_db_g.portmode == PORT_LIMIT_MODE)
        avail_power = (max_power_budget-consumed_power);
    else
        avail_power = (max_power_budget-allocated_power);

    seq_printf(seq,"%u\n", avail_power);

    return 0;
}

static int poe_showallregs_show(struct seq_file *seq, void *v)
{
    int reg=0x0,val;

    val =  poe_read(reg);
    seq_printf(seq,"0x%x | 0x%x ",reg,val);
    do
    {
        reg +=0x1;
        val =  poe_read(reg);
        seq_printf(seq,"0x%x | 0x%x ",reg,val);

        if(reg%5)
            seq_printf(seq,"\t");
        else
            seq_printf(seq,"\n");

    }while( reg <= VMIAN_LOW_TH_MSB );
    seq_printf(seq,"\n");
    return 0;
}


/* This function is to print the technical info for 1.overall power  2. per port power details */
static int poe_showallstats_show(struct seq_file *seq, void *v)
{
  int consumed_power,avail_power;
  int allocated_power;
  int state12;
  int state34;
  int val,portindex=0,port_type;
  int over_load_cnt=0,under_load_cnt=0,short_circuit_cnt=0;
  int power_denied_cnt=0,invalid_cnt = 0;
  int val_low = 0, val_high = 0, cval = 0,vval=0,classindex,pval;
  int cur_base = 12207;     //122.07uA
  int vol_base = 5835;    //5.835mV
  u8 power_stats;

  int max_power_budget;

  state12 =  poe_read(PORT_SR12);
  state34 =  poe_read(PORT_SR34);
  /* Port MOSFET over temperature event.
   * Port is OFF Due to PM
   * */

  seq_printf(seq,"\n%s","-------------------------");
  seq_printf(seq,"\n%s","Port Mode       : ");
  if(poe_db_g.portmode == PORT_LIMIT_MODE)
    seq_printf(seq,"%s","Power Limit");
  else
    seq_printf(seq,"%s","Class Limit");

  seq_printf(seq,"\n%s%s","Legacy          : ",(poe_db_g.legacy_enabled)?"Enabled":"Disabled");
  seq_printf(seq,"\n%s%s","SNMP Trap       : ",(poe_db_g.trap_enabled)?"Enabled":"Disabled");
  seq_printf(seq,"\nTrap Threshold  : %d",poe_db_g.trap_threshold);

  seq_printf(seq,"\n%s","-------------------------");
  seq_printf(seq,"\n%s","PoE State       : ");

  if(poe_check_bit(state12,0) || poe_check_bit(state12,1)
      || poe_check_bit(state12,4) || poe_check_bit(state12,5)
      || poe_check_bit(state34,0) || poe_check_bit(state34,1)
      || poe_check_bit(state34,4) || poe_check_bit(state34,5))
    seq_printf(seq,"%s ","Fault",state12,state34); // POE state fault
  else
    seq_printf(seq,"%s ","Normal",state12,state34);
    //seq_printf(seq,"%s  (SR12:34 0x%x:0x%x) ",Reg,state12,state34); 

  seq_printf(seq,"\n%s","Total Power     : ");
  max_power_budget = MAX_POWER_BUDGET;//poe_read(PWR_BNK0);
  seq_printf(seq,"%d",max_power_budget);

  /* to adjust the 1w difference b/wthe configured 61w and displayed 60w */
  allocated_power =  poe_read(TOTAL_PWR_CALC);
  allocated_power = ((allocated_power > max_power_budget)?max_power_budget:allocated_power);

  consumed_power =  poe_read(TOTAL_PWR_CONS);
  consumed_power = ((consumed_power > max_power_budget)?max_power_budget:consumed_power);

  seq_printf(seq,"\n%s","Consumed Power  : ");
  seq_printf(seq,"%u", consumed_power);
  seq_printf(seq,"\n%s","Allocated Power : ");
  seq_printf(seq,"%u", allocated_power);

  if(poe_db_g.portmode == PORT_LIMIT_MODE)
    avail_power = (max_power_budget-consumed_power);
  else
    avail_power = (max_power_budget-allocated_power);

  seq_printf(seq,"\n%s","Available Power : ");
  seq_printf(seq,"%u", avail_power);


  // read ports
  /* read the poe type as same all the ports*/
  //val =  poe_read(SYS_CFG);
  //port_type =  (val & 0x80) >> 7;
  /* Check for the high power feature for AT configuration. Internal mapping 1-AT,0-AF */
  val =  poe_read(HPEN);

  /* read over load counter  */
  over_load_cnt =  poe_read(OVL_CNT);

  /* read under load counter  */
  under_load_cnt =  poe_read(UDL_CNT);

  /* read short circuit event counter  */
  short_circuit_cnt =  poe_read(SC_CNT);

  /* read power denied event counter  */
  power_denied_cnt =  poe_read(PWRD_CNT);

  /* read invalid event counter  */
  invalid_cnt =  poe_read(INVD_CNT);

  power_stats =  poe_read(STATPWR);
  //    mdelay(100); // delay to avoid the immediate read upon write - I2C noise 100KHz
  do
  {
    switch(portindex)
    {
      case POE_PORT_INDEX_0:
        {
          seq_printf(seq,"\n%s","-------------------------");
          seq_printf(seq,"\n%s","Port no         : ");
          seq_printf(seq, "%d",portindex);//port no
          seq_printf(seq,"\n%s","Class           : ");
          val =  poe_read(STATP1);
          val =  (val & 0x70) >> 4;
          /* Don't show the class name,
             even port fail to power up due out of power budget and class detection is successful. */
          /* PPSBR-1951 The PoE class should reset when unplug PD */
          seq_printf(seq, "%s",((power_stats&BIT_ON_0_4) == 0)?"0": (val == 0) ? "0" : ((val == 1) ? "1" : ((val == 2) ? "2" : ((val == 3) ? "3" : ((val == 4) ? "4" : ((val == 5) ? "0" : ((val == 6) ? "0" : "4")))))));//class

          classindex = val;

          //val =  poe_read(PORT1_CONS);
          //seq_printf(seq, "%u",(val*1000)); //consumed power

          val_low  = poe_read(IP1LSB);
          val_high = poe_read(IP1MSB);
          val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
          cval = val * cur_base;
          val_low  = poe_read(VP1LSB);
          val_high = poe_read(VP1MSB);
          val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
          vval = val * vol_base;

          pval = ((cval/100000)*(vval/1000000));// power consumption
          seq_printf(seq,"\n%s","Max Power Alloc : ");
          if(poe_db_g.portmode == PORT_LIMIT_MODE)
          {
            seq_printf(seq, "%u",poe_db_g.port[portindex].admin_power_alloc); //max power
          }
          else if(poe_db_g.portmode == CLASS_LIMIT_MODE) // Show max power as the class range,if unpowered
          {
            /*if(poe_db_g.legacy_enabled)// legacy enabled case, show consumed as max power
            {
              seq_printf(seq, "%u",(power_stats&BIT_ON_0_4)?(pval):(MAX_PORT_POWER_IN_MW));
            }
            else */// legacy disabled case, show the class max power range as max power alloc
            {
              seq_printf(seq, "%u",(power_stats&BIT_ON_0_4)?(poe_class_power_arr[classindex]*1000):(MAX_PORT_POWER_IN_MW));
            }
          }

          seq_printf(seq,"\n%s%s","Port Priority   : ",((poe_db_g.port[portindex].power_priority) == POWER_PRIORITY_CRITICAL)? "Critical":((poe_db_g.port[portindex].power_priority) == POWER_PRIORITY_HIGH)? "High":"Low");

          seq_printf(seq,"\n%s","Consumed Power  : ");
          seq_printf(seq, "%u",(power_stats&BIT_ON_0_4)?(pval):0);// power consumption

          seq_printf(seq,"\n%s","Over Load count : ");
          val = (over_load_cnt & BIT_ON_0_1);
          seq_printf(seq, "%u",val); //overload

          seq_printf(seq,"\n%s","Absent Count    : ");
          val = (under_load_cnt & BIT_ON_0_1);
          seq_printf(seq, "%u",val); //absent_counter

          seq_printf(seq,"\n%s","Short Count     : ");
          val = (short_circuit_cnt & BIT_ON_0_1);
          seq_printf(seq, "%u",val); //short_counter

          seq_printf(seq,"\n%s","Denied count    : ");
          val = (power_denied_cnt & BIT_ON_0_1);
          seq_printf(seq, "%u",val); //denied_counter

          seq_printf(seq,"\n%s","InvalidSignature: ");
          val = (invalid_cnt & BIT_ON_0_1);
          seq_printf(seq, "%u",val); //invalid_signature_counter

          seq_printf(seq,"\n%s","PoE Std         : ");
          seq_printf(seq, "%s", (port_type == 0) ? POE_STD_DISP_STR_AF:POE_STD_DISP_STR_AT);
        }
        break;

      case POE_PORT_INDEX_1:
        {
          seq_printf(seq,"\n%s","--------------------------");
          seq_printf(seq,"\n%s","Port no         : ");
          seq_printf(seq, "%d",portindex);//port no

          seq_printf(seq,"\n%s","Class           : ");
          val =  poe_read(STATP2);
          val =  (val & 0x70) >> 4;
          seq_printf(seq, "%s", ((power_stats&BIT_ON_1_5) == 0)?"0": (val == 0) ? "0" : ((val == 1) ? "1" : ((val == 2) ? "2" : ((val == 3) ? "3" : ((val == 4) ? "4" : ((val == 5) ? "0" : ((val == 6) ? "0" : "4")))))));//class
          classindex = val;

          //val =  poe_read(PORT2_CONS);
          //seq_printf(seq, "%u",val); //consumed power
          val_low  = poe_read(IP2LSB);
          val_high = poe_read(IP2MSB);
          val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
          cval = val * cur_base;
          val_low  = poe_read(VP2LSB);
          val_high = poe_read(VP2MSB);
          val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
          vval = val * vol_base;
          pval = ((cval/100000)*(vval/1000000));// power consumption

          seq_printf(seq,"\n%s","Max Power Alloc : ");
          if(poe_db_g.portmode == PORT_LIMIT_MODE)
          {
            seq_printf(seq, "%u",poe_db_g.port[portindex].admin_power_alloc); //max power
          }
          else if(poe_db_g.portmode == CLASS_LIMIT_MODE) // Show max power as the class range,if unpowered
          {
            /*if(poe_db_g.legacy_enabled)// legacy enabled case, show consumed as max power
            {
              seq_printf(seq, "%u",(power_stats&BIT_ON_1_5)?(pval):(MAX_PORT_POWER_IN_MW));
            }
            else */// legacy disabled case, show the class max power range as max power alloc
            {
              seq_printf(seq, "%u",(power_stats&BIT_ON_1_5)?(poe_class_power_arr[classindex]*1000):(MAX_PORT_POWER_IN_MW));
            }
          }

          seq_printf(seq,"\n%s%s","Port Priority   : ",((poe_db_g.port[portindex].power_priority) == POWER_PRIORITY_CRITICAL)? "Critical":((poe_db_g.port[portindex].power_priority) == POWER_PRIORITY_HIGH)? "High":"Low");

          seq_printf(seq,"\n%s","Consumed Power  : ");
          seq_printf(seq, "%u",(power_stats&BIT_ON_1_5)?(pval):0);

          seq_printf(seq,"\n%s","Over Load Count : ");
          val = (over_load_cnt & BIT_ON_2_3) >> 2;
          seq_printf(seq, "%u",val); //overload

          seq_printf(seq,"\n%s","Absent Count    : ");
          val = (under_load_cnt & BIT_ON_2_3) >> 2;
          seq_printf(seq, "%u",val); //absent_counter

          seq_printf(seq,"\n%s","Short Count     : ");
          val = (short_circuit_cnt & BIT_ON_2_3) >> 2;
          seq_printf(seq, "%u",val); //short_counter

          seq_printf(seq,"\n%s","Denied Count    : ");
          val = (power_denied_cnt & BIT_ON_2_3) >> 2;
          seq_printf(seq, "%u",val); //denied_counter

          seq_printf(seq,"\n%s","InvalidSignature: ");
          val = (invalid_cnt & BIT_ON_2_3) >> 2;
          seq_printf(seq, "%u",val); //invalid_signature_counter

          seq_printf(seq,"\n%s","PoE Std         : ");
          seq_printf(seq, "%s", (port_type == 0) ? POE_STD_DISP_STR_AF:POE_STD_DISP_STR_AT);
        }
        break;

      case POE_PORT_INDEX_2:
        {
          seq_printf(seq,"\n%s","-------------------------");
          seq_printf(seq,"\n%s","Port no         : ");
          seq_printf(seq, "%d",portindex);//port no
          seq_printf(seq,"\n%s","Class           : ");
          val =  poe_read(STATP3);
          val =  (val & 0x70) >> 4;
          seq_printf(seq, "%s",((power_stats&BIT_ON_2_6) == 0)?"0": (val == 0) ? "0" : ((val == 1) ? "1" : ((val == 2) ? "2" : ((val == 3) ? "3" : ((val == 4) ? "4" : ((val == 5) ? "0" : ((val == 6) ? "0" : "4")))))));//class
          classindex = val;

          //val =  poe_read(PORT3_CONS);
          //seq_printf(seq, "%u",val); //consumed power
          val_low  = poe_read(IP3LSB);
          val_high = poe_read(IP3MSB);
          val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
          cval = val * cur_base;
          val_low  = poe_read(VP3LSB);
          val_high = poe_read(VP3MSB);
          val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
          vval = val * vol_base;
          pval = ((cval/100000)*(vval/1000000));// power consumption

          seq_printf(seq,"\n%s","Max Power Alloc : ");
          if(poe_db_g.portmode == PORT_LIMIT_MODE)
          {
            seq_printf(seq, "%u",poe_db_g.port[portindex].admin_power_alloc); //max power
          }
          else if(poe_db_g.portmode == CLASS_LIMIT_MODE) // Show max power as the class range,if unpowered
          {
            /*if(poe_db_g.legacy_enabled)// legacy enabled case, show consumed as max power
            {
              seq_printf(seq, "%u",(power_stats&BIT_ON_2_6)?(pval):(MAX_PORT_POWER_IN_MW));
            }
            else */// legacy disabled case, show the class max power range as max power alloc
            {
              seq_printf(seq, "%u",(power_stats&BIT_ON_2_6)?(poe_class_power_arr[classindex]*1000):(MAX_PORT_POWER_IN_MW));
            }
          }

          seq_printf(seq,"\n%s%s","Port Priority   : ",((poe_db_g.port[portindex].power_priority) == POWER_PRIORITY_CRITICAL)? "Critical":((poe_db_g.port[portindex].power_priority) == POWER_PRIORITY_HIGH)? "High":"Low");

          seq_printf(seq,"\n%s","Cosumed Power   : ");
          seq_printf(seq, "%u",(power_stats&BIT_ON_2_6)?(pval):0);

          seq_printf(seq,"\n%s","OverLoad Count  : ");
          val = (over_load_cnt & BIT_ON_4_5) >> 4;
          seq_printf(seq, "%u",val); //overload

          seq_printf(seq,"\n%s","Absent Count    : ");
          val = (under_load_cnt & BIT_ON_4_5) >> 4;
          seq_printf(seq, "%u",val); //absent_counter

          seq_printf(seq,"\n%s","Short Count     : ");
          val = (short_circuit_cnt & BIT_ON_4_5) >> 4;
          seq_printf(seq, "%u",val); //short_counter

          seq_printf(seq,"\n%s","Denied Count    : ");
          val = (power_denied_cnt & BIT_ON_4_5) >> 4;
          seq_printf(seq, "%u",val); //denied_counter

          seq_printf(seq,"\n%s","InvalidSignature: ");
          val = (invalid_cnt & BIT_ON_4_5) >> 4;
          seq_printf(seq, "%u",val); //invalid_signature_counter

          seq_printf(seq,"\n%s","PoE Std         : ");
          seq_printf(seq, "%s", (port_type == 0) ?POE_STD_DISP_STR_AF:POE_STD_DISP_STR_AT );
        }
        break;

      case POE_PORT_INDEX_3:
        {
          seq_printf(seq,"\n%s","-------------------------");
          seq_printf(seq,"\n%s","Port no         : ");
          seq_printf(seq, "%d",portindex);//port no

          seq_printf(seq,"\n%s","Class           : ");
          val =  poe_read(STATP4);
          val =  (val & 0x70) >> 4;
          seq_printf(seq, "%s",((power_stats&BIT_ON_3_7) == 0)?"0": (val == 0) ? "0" : ((val == 1) ? "1" : ((val == 2) ? "2" : ((val == 3) ? "3" : ((val == 4) ? "4" : ((val == 5) ? "0" : ((val == 6) ? "0" : "4")))))));//class
          classindex = val;

          //val =  poe_read(PORT4_CONS);
          //seq_printf(seq, "%u",val); //consumed power
          val_low  = poe_read(IP4LSB);
          val_high = poe_read(IP4MSB);
          val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
          cval = val * cur_base;
          val_low  = poe_read(VP4LSB);
          val_high = poe_read(VP4MSB);
          val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
          vval = val * vol_base;
          pval = ((cval/100000)*(vval/1000000));// power consumption

          seq_printf(seq,"\n%s","Max Power Alloc : ");
          if(poe_db_g.portmode == PORT_LIMIT_MODE)
          {
            seq_printf(seq, "%u",poe_db_g.port[portindex].admin_power_alloc); //max power
          }
          else if(poe_db_g.portmode == CLASS_LIMIT_MODE) // Show max power as the class range,if unpowered
          {
            /*if(poe_db_g.legacy_enabled)// legacy enabled case, show consumed as max power
            {
              seq_printf(seq, "%u",(power_stats&BIT_ON_3_7)?(pval):(MAX_PORT_POWER_IN_MW));
            }
            else */// legacy disabled case, show the class max power range as max power alloc
            {
              seq_printf(seq, "%u",(power_stats&BIT_ON_3_7)?(poe_class_power_arr[classindex]*1000):(MAX_PORT_POWER_IN_MW));
            }
          }

          seq_printf(seq,"\n%s%s","Port Priority   : ",((poe_db_g.port[portindex].power_priority) == POWER_PRIORITY_CRITICAL)? "Critical":((poe_db_g.port[portindex].power_priority) == POWER_PRIORITY_HIGH)? "High":"Low");

          seq_printf(seq,"\n%s","Consumed Power  : ");
          seq_printf(seq, "%u",(power_stats&BIT_ON_3_7)?(pval):0);

          seq_printf(seq,"\n%s","Over Load Count : ");
          val = (over_load_cnt & BIT_ON_6_7) >> 6;
          seq_printf(seq, "%u",val); //overload

          seq_printf(seq,"\n%s","absent Count    : ");
          val = (under_load_cnt & BIT_ON_6_7) >> 6;
          seq_printf(seq, "%u",val); //absent_counter

          seq_printf(seq,"\n%s","Short Count     : ");
          val = (short_circuit_cnt & BIT_ON_6_7) >> 6;
          seq_printf(seq, "%u",val); //short_counter

          seq_printf(seq,"\n%s","Denied Count    : ");
          val = (power_denied_cnt & BIT_ON_6_7) >> 6;
          seq_printf(seq, "%u",val); //denied_counter

          seq_printf(seq,"\n%s","InvalidSignature: ");
          val = (invalid_cnt & BIT_ON_6_7) >> 6;
          seq_printf(seq, "%u",val); //invalid_signature_counter

          seq_printf(seq,"\n%s","PoE Std         : ");
          seq_printf(seq, "%s\n", (port_type == 0) ?POE_STD_DISP_STR_AF:POE_STD_DISP_STR_AT );
          seq_printf(seq,"%s\n\n","-------------------------");
        }
        break;

      default:
        break;
    }

    portindex++;
  }while(portindex < MAX_POE_PORTS);

  return 0;
}

#ifdef POE_EXTRA_COMMAND
static int poe_type_show(struct seq_file *seq, void *v)
{
	int val;

	val =  poe_read(SYS_CFG);
	seq_printf(seq, "SYS_CFG = 0x%x\n", val);
	val =  (val & 0x80) >> 7;
	seq_printf(seq, "poe type = %s\n", (val == 0) ? "AF" : "AT");

	return 0;
}

static int poe_type_open(struct inode *inode, struct file *file)
{
	return single_open(file, poe_type_show, PDE_DATA(inode));
}

static int poe_type_write(struct file *file, const char __user * user_buffer, size_t count, loff_t * ppos)
{
	int val;

	if(sscanf(user_buffer, "%d", &val) != 1)
		return -EINVAL;

	if(val == 1)
	{
		poe_write(SYS_CFG, poe_read(SYS_CFG) | (1 << 7));
		printk("poe poe type set to be AT\n");
	}
	else if(val == 0)
	{
		poe_write(SYS_CFG, poe_read(SYS_CFG) & ~(1 << 7));
		printk("poe poe type set to be AF\n");
	}

	return count;
}

static int poe_class_show(struct seq_file *seq, void *v)
{
	int val;

	val =  poe_read(STATP1);
	seq_printf(seq, "STATP1 = 0x%x\n", val);
	val =  (val & 0x70) >> 4;
	seq_printf(seq, "port1 poe class = %s\n",
	        (val == 0) ? "Unknown" : ((val == 1) ? "Class 1" : ((val == 2) ? "Class 2" : ((val == 3) ? "Class 3" : ((val == 4) ? "Class 4" : ((val == 5) ? "Reserved" : ((val == 6) ? "Class 0" : "Over-current")))))));

	val =  poe_read(STATP2);
	seq_printf(seq, "STATP2 = 0x%x\n", val);
	val =  (val & 0x70) >> 4;
	seq_printf(seq, "port2 poe class = %s\n",
	        (val == 0) ? "Unknown" : ((val == 1) ? "Class 1" : ((val == 2) ? "Class 2" : ((val == 3) ? "Class 3" : ((val == 4) ? "Class 4" : ((val == 5) ? "Reserved" : ((val == 6) ? "Class 0" : "Over-current")))))));

	val =  poe_read(STATP3);
	seq_printf(seq, "STATP3 = 0x%x\n", val);
	val =  (val & 0x70) >> 4;
	seq_printf(seq, "port3 poe class = %s\n",
	        (val == 0) ? "Unknown" : ((val == 1) ? "Class 1" : ((val == 2) ? "Class 2" : ((val == 3) ? "Class 3" : ((val == 4) ? "Class 4" : ((val == 5) ? "Reserved" : ((val == 6) ? "Class 0" : "Over-current")))))));

	val =  poe_read(STATP4);
	seq_printf(seq, "STATP4 = 0x%x\n", val);
	val =  (val & 0x70) >> 4;
	seq_printf(seq, "port4 poe class = %s\n",
	        (val == 0) ? "Unknown" : ((val == 1) ? "Class 1" : ((val == 2) ? "Class 2" : ((val == 3) ? "Class 3" : ((val == 4) ? "Class 4" : ((val == 5) ? "Reserved" : ((val == 6) ? "Class 0" : "Over-current")))))));

	return 0;
}

static int poe_class_open(struct inode *inode, struct file *file)
{
	return single_open(file, poe_class_show, PDE_DATA(inode));
}

static int poe_class_write(struct file *file, const char __user * user_buffer, size_t count, loff_t * ppos)
{
	return count;
}

static int poe_power_show(struct seq_file *seq, void *v)
{
	int val;

	val =  poe_read(PORT1_CONS);
	seq_printf(seq, "PORT1_CONS = 0x%x\n", val);
	val =  val & 0x3F;
	seq_printf(seq, "port1 power = %dW\n", val);

	val =  poe_read(PORT2_CONS);
	seq_printf(seq, "PORT2_CONS = 0x%x\n", val);
	val =  val & 0x3F;
	seq_printf(seq, "port2 power = %dW\n", val);

	val =  poe_read(PORT3_CONS);
	seq_printf(seq, "PORT3_CONS = 0x%x\n", val);
	val =  val & 0x3F;
	seq_printf(seq, "port3 power = %dW\n", val);

	val =  poe_read(PORT4_CONS);
	seq_printf(seq, "PORT4_CONS = 0x%x\n", val);
	val =  val & 0x3F;
	seq_printf(seq, "port4 power = %dW\n", val);

	return 0;
}

static int poe_power_open(struct inode *inode, struct file *file)
{
	return single_open(file, poe_power_show, PDE_DATA(inode));
}

static int poe_power_write(struct file *file, const char __user * user_buffer, size_t count, loff_t * ppos)
{
	return count;
}

static int poe_current_show(struct seq_file *seq, void *v)
{
	int val_low = 0, val_high = 0, val = 0;
	int cur_base = 12207;     //122.07uA

	val_low  = poe_read(IP1LSB);
	seq_printf(seq, "IP1LSB = 0x%x\n", val_low);
	val_high = poe_read(IP1MSB);
	seq_printf(seq, "IP1MSB = 0x%x\n", val_high);
	val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
	val = val * cur_base;
	seq_printf(seq, "port1 current = %d.%dmA\n", val/100000, val%100000);

	val_low  = poe_read(IP2LSB);
	seq_printf(seq, "IP2LSB = 0x%x\n", val_low);
	val_high = poe_read(IP2MSB);
	seq_printf(seq, "IP2MSB = 0x%x\n", val_high);
	val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
	val = val * cur_base;
	seq_printf(seq, "port2 current = %d.%dmA\n", val/100000, val%100000);

	val_low  = poe_read(IP3LSB);
	seq_printf(seq, "IP3LSB = 0x%x\n", val_low);
	val_high = poe_read(IP3MSB);
	seq_printf(seq, "IP3MSB = 0x%x\n", val_high);
	val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
	val = val * cur_base;
	seq_printf(seq, "port3 current = %d.%dmA\n", val/100000, val%100000);

	val_low  = poe_read(IP4LSB);
	seq_printf(seq, "IP4LSB = 0x%x\n", val_low);
	val_high = poe_read(IP4MSB);
	seq_printf(seq, "IP4MSB = 0x%x\n", val_high);
	val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
	val = val * cur_base;
	seq_printf(seq, "port4 current = %d.%dmA\n", val/100000, val%100000);

	return 0;
}

static int poe_current_open(struct inode *inode, struct file *file)
{
	return single_open(file, poe_current_show, PDE_DATA(inode));
}

static int poe_current_write(struct file *file, const char __user * user_buffer, size_t count, loff_t * ppos)
{
	return count;
}

static int poe_voltage_show(struct seq_file *seq, void *v)
{
	int val_low = 0, val_high = 0, val = 0;
	int vol_base = 5835;    //5.835mV

	val_low  = poe_read(VP1LSB);
	seq_printf(seq, "VP1LSB = 0x%x\n", val_low);
	val_high = poe_read(VP1MSB);
	seq_printf(seq, "VP1MSB = 0x%x\n", val_high);
	val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
	val = val * vol_base;
	seq_printf(seq, "port1 voltage = %d.%dV\n", val/1000000,val%1000000);

	val_low  = poe_read(VP2LSB);
	seq_printf(seq, "VP2LSB = 0x%x\n", val_low);
	val_high = poe_read(VP2MSB);
	seq_printf(seq, "VP2MSB = 0x%x\n", val_high);
	val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
	val = val * vol_base;
	seq_printf(seq, "port2 voltage = %d.%dV\n", val/1000000,val%1000000);

	val_low  = poe_read(VP3LSB);
	seq_printf(seq, "VP3LSB = 0x%x\n", val_low);
	val_high = poe_read(VP3MSB);
	seq_printf(seq, "VP3MSB = 0x%x\n", val_high);
	val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
	val = val * vol_base;
	seq_printf(seq, "port3 voltage = %d.%dV\n", val/1000000,val%1000000);

	val_low  = poe_read(VP4LSB);
	seq_printf(seq, "VP4LSB = 0x%x\n", val_low);
	val_high = poe_read(VP4MSB);
	seq_printf(seq, "VP4MSB = 0x%x\n", val_high);
	val = (val_low & 0xFF) + ((val_high << 8) & 0xFF00);
	val = val * vol_base;
	seq_printf(seq, "port4 voltage = %d.%dV\n", val/1000000,val%1000000);

	return 0;
}

static int poe_voltage_open(struct inode *inode, struct file *file)
{
	return single_open(file, poe_voltage_show, PDE_DATA(inode));
}

static int poe_voltage_write(struct file *file, const char __user * user_buffer, size_t count, loff_t * ppos)
{
	return count;
}

#endif
static int poe_reg_open(struct inode *inode, struct file *file)
{
	return single_open(file, poe_reg_show, PDE_DATA(inode));
}

static int poe_portpower_open(struct inode *inode, struct file *file)
{
	return single_open(file, poe_portpower_show, PDE_DATA(inode));
}

static int poe_setconf_open(struct inode *inode, struct file *file)
{
	return single_open(file, poe_setconf_show, PDE_DATA(inode));
}

static int poe_getportstats_open(struct inode *inode, struct file *file)
{
	return single_open(file, poe_getportstats_show, PDE_DATA(inode));
}

static int poe_getpowerstats_open(struct inode *inode, struct file *file)
{
	return single_open(file, poe_getpowerstats_show, PDE_DATA(inode));
}

static int poe_showallregs_open(struct inode *inode, struct file *file)
{
	return single_open(file, poe_showallregs_show, PDE_DATA(inode));
}

static int poe_showallstats_open(struct inode *inode, struct file *file)
{
	return single_open(file, poe_showallstats_show, PDE_DATA(inode));
}
static struct file_operations poe_proc_poe_reg_fops =
{
	.owner = THIS_MODULE,
	.open = poe_reg_open,
	.read = seq_read,
	.write = poe_reg_write,
	.llseek = seq_lseek,
	.release = single_release,
};
static struct file_operations poe_proc_poe_portpower_fops =
{
	.owner = THIS_MODULE,
	.open = poe_portpower_open,
	.read = seq_read,
	.write = poe_portpower_write,
	.llseek = seq_lseek,
	.release = single_release,
};
static struct file_operations poe_proc_poe_setconf_fops =
{
	.owner = THIS_MODULE,
	.open = poe_setconf_open,
	.read = seq_read,
	.write = poe_setconf_write,
	.llseek = seq_lseek,
	.release = single_release,
};
static struct file_operations poe_proc_poe_getportstats_fops =
{
	.owner = THIS_MODULE,
	.open = poe_getportstats_open,
	.read = seq_read,
	.write = poe_getportstats_write,
	.llseek = seq_lseek,
	.release = single_release,
};
static struct file_operations poe_proc_poe_getpowerstats_fops =
{
	.owner = THIS_MODULE,
	.open = poe_getpowerstats_open,
	.read = seq_read,
	.write = poe_getpowerstats_write,
	.llseek = seq_lseek,
	.release = single_release,
};
static struct file_operations poe_proc_poe_showallregs_fops =
{
	.owner = THIS_MODULE,
	.open = poe_showallregs_open,
	.read = seq_read,
	.write = poe_showallregs_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct file_operations poe_proc_poe_showallstats_fops =
{
	.owner = THIS_MODULE,
	.open = poe_showallstats_open,
	.read = seq_read,
	.write = poe_showallstats_write,
	.llseek = seq_lseek,
	.release = single_release,
};
#ifdef POE_EXTRA_COMMAND
static struct file_operations poe_proc_poe_type_fops =
{
	.owner = THIS_MODULE,
	.open = poe_type_open,
	.read = seq_read,
	.write = poe_type_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct file_operations poe_proc_poe_class_fops =
{
	.owner = THIS_MODULE,
	.open = poe_class_open,
	.read = seq_read,
	.write = poe_class_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct file_operations poe_proc_poe_power_fops =
{
	.owner = THIS_MODULE,
	.open = poe_power_open,
	.read = seq_read,
	.write = poe_power_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct file_operations poe_proc_poe_current_fops =
{
	.owner = THIS_MODULE,
	.open = poe_current_open,
	.read = seq_read,
	.write = poe_current_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static struct file_operations poe_proc_poe_voltage_fops =
{
	.owner = THIS_MODULE,
	.open = poe_voltage_open,
	.read = seq_read,
	.write = poe_voltage_write,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

static int poe_proc_fs_create(void)
{
    struct proc_dir_entry *proc_poe_dir = NULL;
   
    proc_poe_dir = proc_mkdir("poe", NULL);
    if(!proc_poe_dir)
        return -ENOMEM;

    proc_create("poe_reg", 0644, proc_poe_dir,&poe_proc_poe_reg_fops);
    proc_create("portpower", 0644, proc_poe_dir,&poe_proc_poe_portpower_fops);
#ifdef POE_EXTRA_COMMAND
    proc_create("poe_type", 0644, proc_poe_dir, &poe_proc_poe_type_fops);
	proc_create("poe_class", 0644, proc_poe_dir, &poe_proc_poe_class_fops);
	proc_create("poe_power", 0644, proc_poe_dir, &poe_proc_poe_power_fops);
	proc_create("poe_current", 0644, proc_poe_dir, &poe_proc_poe_current_fops);
	proc_create("poe_voltage", 0644, proc_poe_dir, &poe_proc_poe_voltage_fops);
#endif 
    proc_create("setconf", 0644, proc_poe_dir,&poe_proc_poe_setconf_fops);
    proc_create("getportstats", 0644, proc_poe_dir,&poe_proc_poe_getportstats_fops);
    proc_create("getpowerstats", 0644, proc_poe_dir,&poe_proc_poe_getpowerstats_fops);
    proc_create("showallregs", 0644, proc_poe_dir,&poe_proc_poe_showallregs_fops);
    proc_create("showallstats", 0644, proc_poe_dir,&poe_proc_poe_showallstats_fops);

    return 0;
}

static int poe_proc_fs_destroy(void)
{
    remove_proc_entry("poe/poe_reg", NULL);
    remove_proc_entry("poe/portpower", NULL);
    remove_proc_entry("poe/setconf", NULL);
    remove_proc_entry("poe/getportstats", NULL);
    remove_proc_entry("poe/getpowerstats", NULL);
    remove_proc_entry("poe/showallregs", NULL);
    remove_proc_entry("poe/showallstats", NULL);
#ifdef POE_EXTRA_COMMAND
	remove_proc_entry("poe/poe_type", NULL);
	remove_proc_entry("poe/poe_class", NULL);
	remove_proc_entry("poe/poe_power", NULL);
	remove_proc_entry("poe/poe_current", NULL);
	remove_proc_entry("poe/poe_voltage", NULL);
#endif
    remove_proc_entry("poe", NULL);

    return 0;
}



void poe_configure_default_setting(void)
{
    u8 sys_cfg_reg=0;
    u8 sw_cfg=0;

    poe_write(OPMD,0);   //Disable all the ports

    sw_cfg = poe_read(SW_CFG);
    /*set default ports type as AT */
    sys_cfg_reg = poe_read(SYS_CFG);
    /*  disable legacy by default  */
    poe_set_bit(&sys_cfg_reg,0);
    /* 2 pair config */
    poe_clear_bit(&sys_cfg_reg,1);
    poe_clear_bit(&sys_cfg_reg,2);
    /* bank 0  */
    poe_clear_bit(&sys_cfg_reg,6);
    /* clear the bit as AF-  AT during startup is experimental
     *high power HPEN should be enough for AT ports */
    poe_clear_bit(&sys_cfg_reg,7);
    poe_write(SYS_CFG, sys_cfg_reg);

    /* Icut TPPL */
    poe_clear_bit(&sw_cfg,0);
    /* power management - Dynamic (according to real port consumption) */
    poe_set_bit(&sw_cfg,1);
    /*  set 0 - Class 0 is detected as high power (*) */
    poe_clear_bit(&sw_cfg,2);
    /*  set 0 -  Class 123 detected as hig power */
    poe_clear_bit(&sw_cfg,3);
    /*6:5 01 Set high current class as class 4 */
    poe_set_bit(&sw_cfg,5);
    /* To avoid know issue in PM,IC mightnot continue to next port */
    poe_clear_bit(&sw_cfg,7);
    poe_write(SW_CFG,sw_cfg);

    /* set high power feature for 4 ports as zero. If the PPL is > 15W enable it*/
    poe_write(HPEN,0x0);
    /* current limit fold-back setting - AT 0xC, AF-0x80 alonf with HPEN */
    poe_write(LIM1,0xC0);
    poe_write(LIM2,0xC0);
    poe_write(LIM3,0xC0);
    poe_write(LIM4,0xC0);

    /*set overall max power budget to 61W */
    poe_write(PWR_BNK0,MAX_POWER_BUDGET+1);

}
/**************************************************************/

void gpio_release_poe_reset(void)
{
    writel(readl(COMCERTO_GPIO_OUTPUT_REG) | (POE_RESET_GPIO_PIN), COMCERTO_GPIO_OUTPUT_REG);
    return;
}

static int poe_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    printk("PoE App Driver probe\n");
    g_client = client;
    poe_proc_fs_create();
    return 0;
}

static int poe_remove(struct i2c_client *client)
{
    /* disable port and detection. This is to handle,non-script stop cases */
    poe_write(OPMD,0);
    poe_write(DETENA,0);
    printk("PoE App Driver Removed\n");
    poe_proc_fs_destroy();
    return 0;
}

static const struct i2c_device_id poe_id[] = {
    {"pd69104b", 0x20},
    {}
};

static struct i2c_driver poe_driver = {
    .driver = {
        .name = "poeappdrv",
        .owner=THIS_MODULE,
    },
    .probe = poe_probe,
    .remove = poe_remove,
    .id_table = poe_id,
};

static int __init poe_init(void)
{
    int ret;

    ret = i2c_add_driver(&poe_driver);
    if(ret < 0)
        return ret;

    gpio_release_poe_reset();
    mdelay(320);
    poe_configure_default_setting();
    return 0;
}

static void __exit poe_exit(void)
{
    i2c_del_driver(&poe_driver);
}

module_init(poe_init);
module_exit(poe_exit);

MODULE_DESCRIPTION("PoE App driver");
MODULE_LICENSE("GPL");
