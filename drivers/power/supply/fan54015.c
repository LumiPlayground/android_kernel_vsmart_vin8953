/**********uniscope-driver-modify-file-on-qualcomm-platform*****************/
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/param.h>
#include <linux/stat.h>
#include <linux/irq.h>
#include <linux/of_gpio.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include "fan54015.h"


#define SPRD_EX_DEBUG(format, arg...)
#define FAN_54015_DEBUG_FS
static struct fan54015 *fan54015_data = NULL;

static int fan54015_write_reg(int reg, u8 val)
{
	int ret;
	SPRD_EX_DEBUG("#### writereg reg = %d val = %d\n",reg,val);
	ret = i2c_smbus_write_byte_data(fan54015_data->client, reg, val);

	if (ret < 0)
	{
		SPRD_EX_DEBUG("%s: error = %d", __func__, ret);
	}
    return ret;
}

int fan54015_read_reg(int reg, u8 *dest)
{
	int ret;

	ret = i2c_smbus_read_byte_data(fan54015_data->client, reg);
	if (ret < 0) {
		SPRD_EX_DEBUG("%s reg(0x%x), ret(%d)\n", __func__, reg, ret);
		return ret;
	}

	ret &= 0xff;
	*dest = ret;
	SPRD_EX_DEBUG("######fan54015readreg reg  = %d value =%d/%x\n",reg, ret, ret);
	return 0;
}

static void fan54015_set_value(BYTE reg, BYTE reg_bit,BYTE reg_shift, BYTE val)
{
	BYTE tmp = 0;
	fan54015_read_reg(reg, &tmp);
	tmp = (tmp & (~reg_bit)) |(val << reg_shift);
	if((0x04 == reg)&&(FAN5405_RESET != reg_bit)){
		tmp &= 0x7f;
	}
	fan54015_write_reg(reg,tmp);
	
}

static BYTE fan54015_get_value(BYTE reg, BYTE reg_bit, BYTE reg_shift)
{
	BYTE data = 0;
	int ret = 0 ;
	ret = fan54015_read_reg(reg, &data);
	ret = (data & reg_bit) >> reg_shift;

	return ret;
}

void  fan54015_reset_timer(void)
{
	SPRD_EX_DEBUG("fan54015 reset timer\n");
	fan54015_set_value(FAN5405_REG_CONTROL0, FAN5405_TMR_RST_OTG,FAN5405_TMR_RST_OTG_SHIFT, RESET32S);

}
EXPORT_SYMBOL_GPL(fan54015_reset_timer);
void  fan54015_sw_reset(void)
{
	SPRD_EX_DEBUG("fan54015_sw_reset\n");
	fan54015_set_value(FAN5405_REG_IBAT, FAN5405_RESET, FAN5405_RESET_SHIFT, 1);
}
EXPORT_SYMBOL_GPL(fan54015_sw_reset);
void fan54015_enable_cur_terminal(int enable){
	if(enable)
		fan54015_set_value(FAN5405_REG_CONTROL1, FAN5405_TE,FAN5405_TE_SHIFT, ENTE);
	else
		fan54015_set_value(FAN5405_REG_CONTROL1, FAN5405_TE,FAN5405_TE_SHIFT, DISTE);
}
EXPORT_SYMBOL_GPL(fan54015_enable_cur_terminal);
void fan54015_set_vindpm(BYTE reg_val)
{
	fan54015_write_reg(FAN5405_REG_SP_CHARGER,0x80);/*0x84 zhangbing@uniscope_drv 20160708 add*/
}
EXPORT_SYMBOL_GPL(fan54015_set_vindpm);

void fan54015_termina_cur_set(BYTE reg_val)
{
	fan54015_set_value(FAN5405_REG_IBAT, FAN5405_ITERM,FAN5405_ITERM_SHIFT, reg_val);
}
EXPORT_SYMBOL_GPL(fan54015_termina_cur_set);

void fan54015_termina_vol_set(BYTE reg_val)
{
	fan54015_set_value(FAN5405_REG_OREG, FAN5405_OREG,FAN5405_OREG_SHIFT, reg_val);
}
EXPORT_SYMBOL_GPL(fan54015_termina_vol_set);

void fan54015_init(void)
{
	BYTE data = 0;
	int i = 0;
	SPRD_EX_DEBUG("fan54015_init\n");
	
	/*zhangbing@uniscope_drv 20160708 add to write all registers begin*/
	fan54015_write_reg(FAN5405_REG_SAFETY,0x69);
	fan54015_write_reg(FAN5405_REG_CONTROL1,0xf8);
	fan54015_write_reg(FAN5405_REG_IBAT,0x40);
	fan54015_write_reg(FAN5405_REG_OREG,0x92);
	fan54015_write_reg(FAN5405_REG_SP_CHARGER,0x80);//0x84
	/*zhangbing@uniscope_drv 20160708 add to write all registers end*/

	for(i = 0;  i < 15;  i++){
		fan54015_read_reg(i, &data);
		SPRD_EX_DEBUG("fan54015_ReadReg1 i = %d, data = %x\n",i,data);
	}
}
EXPORT_SYMBOL_GPL(fan54015_init);

void fan54015_set_safety_vol(BYTE reg_val )
{
	fan54015_write_reg(FAN5405_REG_SAFETY,0x69);/*zhangbing@uniscope_drv 20160708 add*/
}

EXPORT_SYMBOL_GPL(fan54015_set_safety_vol);

void fan54015_set_safety_cur(BYTE reg_val )
{
	fan54015_write_reg(FAN5405_REG_SAFETY,0x69);/*zhangbing@uniscope_drv 20160708 add*/
}
EXPORT_SYMBOL_GPL(fan54015_set_safety_cur);

void fan54015_otg_enable(BYTE reg_val)
{
	SPRD_EX_DEBUG("fan54015_OTG_Enable enable =%d\n",reg_val);
	
	if(reg_val){

	    fan54015_set_value(FAN5405_REG_CONTROL1, FAN5405_OPA_MODE, FAN5405_OPA_MODE_SHIFT, reg_val);
	}else{
	    fan54015_set_value(FAN5405_REG_CONTROL1, FAN5405_OPA_MODE, FAN5405_OPA_MODE_SHIFT, reg_val);
	}
}
EXPORT_SYMBOL_GPL(fan54015_otg_enable);

void fan54015_stop_charging(void)
{
	 gpio_set_value(12,1); /*zhangbing@uniscope_drv 20160708 add*/
}
EXPORT_SYMBOL_GPL(fan54015_stop_charging);

BYTE fan54015_get_vendor_id(void)
{
	return fan54015_get_value(FAN5405_REG_IC_INFO, FAN5405_VENDOR_CODE, FAN5405_VENDOR_CODE_SHIFT);
}
EXPORT_SYMBOL_GPL(fan54015_get_vendor_id);

BYTE fan54015_get_chg_status(void)
{
	return fan54015_get_value(FAN5405_REG_CONTROL0, FAN5405_STAT, FAN5405_STAT_SHIFT);
}
EXPORT_SYMBOL_GPL(fan54015_get_chg_status);

BYTE fan54015_get_fault_val(void)
{
	return fan54015_get_value(FAN5405_REG_CONTROL0, FAN5405_FAULT, FAN5405_FAULT_SHIFT);
}
EXPORT_SYMBOL_GPL(fan54015_get_fault_val);

BYTE fan54015_get_monitor_val(void)
{
	return fan54015_get_value(FAN5405_REG_MONITOR, FAN5405_VBUS_VALID, FAN5405_VBUS_VALID_SHIFT);
}
EXPORT_SYMBOL_GPL(fan54015_get_monitor_val);

void fan54015_enable_chg()
{
	gpio_set_value(12,0);/*zhangbing@uniscope_drv 20160708 add*/
}
EXPORT_SYMBOL_GPL(fan54015_enable_chg);

void fan54015_set_chg_current(BYTE reg_val)
{
	if(reg_val == 0){   
		fan54015_write_reg(FAN5405_REG_CONTROL1,0x78);/*zhangbing@uniscope_drv 20160708 add*/
	}else{
		fan54015_write_reg(FAN5405_REG_CONTROL1,0xf8);/*zhangbing@uniscope_drv 20160708 add*/
	}
	fan54015_write_reg(FAN5405_REG_IBAT,0x40);/*zhangbing@uniscope_drv 20160708 add*/
}
EXPORT_SYMBOL_GPL(fan54015_set_chg_current);

BYTE fan54015_get_chg_current(void)
{
	return fan54015_get_value(FAN5405_REG_IBAT, FAN5405_IOCHARGE, FAN5405_IOCHARGE_SHIFT);
}
EXPORT_SYMBOL_GPL(fan54015_get_chg_current);

bool fan54015_get_opa_mode(void)
{
   return fan54015_get_value(FAN5405_REG_CONTROL1, FAN5405_OPA_MODE, FAN5405_OPA_MODE_SHIFT)?1:0;
}
EXPORT_SYMBOL_GPL(fan54015_get_opa_mode);

void fan54015_set_thermal_current(int reg_val)
{
    if (reg_val == 0)
	    fan54015_set_value(FAN5405_REG_CONTROL1, FAN5405_IINLIM, FAN5405_IINLIM_SHIFT, 0x01);
    else {
        fan54015_set_value(FAN5405_REG_CONTROL1, FAN5405_IINLIM, FAN5405_IINLIM_SHIFT, 0x03);
    }
}

#ifdef FAN_54015_DEBUG_FS

static ssize_t set_regs_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long set_value;
	int reg;  // bit 16:8
	int val;  //bit7:0
	int ret;
	set_value = simple_strtoul(buf, NULL, 16);

	reg = (set_value & 0xff00) >> 8;
	val = set_value & 0xff;
	SPRD_EX_DEBUG("fan54015 set reg = %d value = %d\n",reg, val);
	ret = fan54015_write_reg(reg, val);

	if (ret < 0){
		printk("set_regs_store error\n");
		return -EINVAL;
		}

	return count;
}

static ssize_t dump_regs_show(struct device *dev, struct device_attribute *attr,
                char *buf)
{
	const int regaddrs[] = {0x00, 0x01, 0x02, 0x03, 0x4, 0x05, 0x06, 0x07,0x08,0x09,0x10 };
	const char str[] = "0123456789abcdef";
	BYTE fan54015_regs[0x60];

	int i = 0, index;
	char val = 0;
	for (i=0; i<0x60; i++) {
		if ((i%3)==2)
			buf[i]=' ';
		else
			buf[i] = 'x';
	}
	buf[0x5d] = '\n';
	buf[0x5e] = 0;
	buf[0x5f] = 0;

	for ( i = 0; i < 11; i++) {
		 fan54015_read_reg(i,&fan54015_regs[i]);
	}

	for (i=0; i<ARRAY_SIZE(regaddrs); i++) {
		index = regaddrs[i];
		val = fan54015_regs[index];
		buf[3*index] = str[(val&0xf0)>>4];
		buf[3*index+1] = str[val&0x0f];
		buf[3*index+1] = str[val&0x0f];
	}

	return 0x60;
}


static DEVICE_ATTR(dump_regs, S_IRUGO | S_IWUSR, dump_regs_show, NULL);
static DEVICE_ATTR(set_regs, S_IRUGO | S_IWUSR, NULL, set_regs_store);
#endif
extern void sprdchg_fan54015_init(void);
/*zhangbing@uniscope_drv 20160708 add begin*/
extern void sprdchg_fan54015_start_chg(int cur);
extern int sprdchg_fan54015_get_charge_status(void);
extern int sprdchg_fan54015_get_charge_fault(void);
extern int sprdchg_fan54015_get_chgcur_step(int cur,int increase);
extern int sprdchg_fan54015_get_charge_vbus_valid(void);
extern void sprdchg_fan54015_reset_timer(void);
extern int sprdchg_fan54015_get_chgcur(void);
extern int sprdchg_fan54015_get_chgcur_step(int cur,int increase);
/*zhangbing@uniscope_drv 20160708 add end*/

static void fan54015_parse_dt(struct device *dev,struct fan54015_platform_data *pdata)
{
	struct device_node *np = dev->of_node;

	pdata->chg_dis_gpio = of_get_named_gpio_flags(np, "qcom,disable-gpio", 0,&pdata->chg_dis_gpio_flags);
}

static int fan54015_probe(
	struct i2c_client *client, const struct i2c_device_id *id)
{
	int rc = 0;
	int err = -1;
	int ret;
	struct fan54015_platform_data *pdata = client->dev.platform_data;
	
#ifdef CONFIG_OF
		struct device_node *np = client->dev.of_node;
		if (np && !pdata){
			pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
			if(!pdata){
				err = -ENOMEM;
				goto exit_alloc_platform_data_failed;
			}
		}
#else
	if (pdata == NULL) {
		pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
		if (pdata == NULL) {
			rc = -ENOMEM;
			pr_err("%s: platform data is NULL\n", __func__);
			goto err_alloc_data_failed;
		}
	}
#endif
	SPRD_EX_DEBUG("@@@@@@@fan54015_probe\n");
	fan54015_data = kzalloc(sizeof(struct fan54015), GFP_KERNEL);
	if (!fan54015_data) {
		SPRD_EX_DEBUG("kzalloc failed!\n");
		rc = -ENOMEM;
		goto err_alloc_data_failed;
	}
	
	i2c_set_clientdata(client, pdata);
	fan54015_data->client = client;

/*zhangbing@uniscope_drv 20160708 add to parse DT and request GPIO begin*/
	fan54015_parse_dt(&client->dev,pdata);

	if (gpio_is_valid(pdata->chg_dis_gpio)) {
		ret = gpio_request(pdata->chg_dis_gpio,"chg_dis_gpio");
		if (ret < 0) {	
			pr_err("chg_dis_gpio request failed\n");
			ret = -ENODEV;
		}
		
		ret = gpio_direction_output(pdata->chg_dis_gpio,1);
		if (ret < 0) {
			pr_err("set direction for chg_dis_gpio failed\n");
			ret = -ENODEV;
		}

		gpio_set_value_cansleep(pdata->chg_dis_gpio,0);

	}
	/*zhangbing@uniscope_drv 20160708 add to parse DT and request GPIO end*/

	/*zhangbing@uniscope_drv 20160708 add to write all registers when probe begin*/
	fan54015_write_reg(FAN5405_REG_SAFETY,0x69);
	fan54015_write_reg(FAN5405_REG_CONTROL1,0xf8);
	fan54015_write_reg(0x51,0x1);
	fan54015_write_reg(FAN5405_REG_IBAT,0x33);
	fan54015_write_reg(FAN5405_REG_OREG,0xb0); 
	fan54015_write_reg(FAN5405_REG_SP_CHARGER,0x04);
	/*zhangbing@uniscope_drv 20160708 add to write all registers when probe end*/
	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		pr_err("%s: i2c check functionality error\n", __func__);
		rc = -ENODEV;
		goto check_funcionality_failed;
	}
#ifdef FAN_54015_DEBUG_FS
	device_create_file(&client->dev, &dev_attr_dump_regs);
	device_create_file(&client->dev, &dev_attr_set_regs);
#endif
	SPRD_EX_DEBUG("@@@@@@@fan54015_probe ok\n");
	
	return rc;

check_funcionality_failed:
	kfree(fan54015_data);
	fan54015_data = NULL;
err_alloc_data_failed:
#ifdef CONFIG_OF
	kfree(pdata);
exit_alloc_platform_data_failed:
	SPRD_EX_DEBUG("@@@@@@@fan54015_probe fail\n");
	return err;
#endif
}
static int fan54015_remove(struct i2c_client *client)
{
	struct fan54015_platform_data *pdata = i2c_get_clientdata(client);
	SPRD_EX_DEBUG("@@@@@@@fan54015_remove \n");
	kfree(pdata);
	flush_scheduled_work();
	kfree(fan54015_data);
	fan54015_data = NULL;
	return 0;
}
#if 0
static int  fan54015_suspend(struct i2c_client *client, pm_message_t message)
{
	return 0;
}

static int fan54015_resume(struct i2c_client *client)
{
	return 0;
}
#endif
static const struct i2c_device_id fan54015_i2c_id[] = {
	{ "fan54015_chg", 0 },
	{ }
};

#ifdef CONFIG_OF
static const struct of_device_id fan54015_of_match[] = {
	{.compatible = "fairchild, fan54015_chg",},
	{}
};
#endif

static struct i2c_driver fan54015_i2c_driver = {
	.driver = {
		.name = "fan54015_chg",
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(fan54015_of_match),
#endif
	},
	.probe    = fan54015_probe,
	.remove   = fan54015_remove,
//	.suspend  = fan54015_suspend,
//	.resume = fan54015_resume,
	.id_table = fan54015_i2c_id,
};

static int __init fan54015_i2c_init(void)
{
	SPRD_EX_DEBUG("@@@@@@@fan54015_i2c_init \n");
	return i2c_add_driver(&fan54015_i2c_driver);
}

static void __exit  fan54015_i2c_exit(void)
{
	 i2c_del_driver(&fan54015_i2c_driver);  
}

subsys_initcall_sync(fan54015_i2c_init);
module_exit(fan54015_i2c_exit);
