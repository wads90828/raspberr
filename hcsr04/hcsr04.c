#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#define DRIVER_AUTHOR    "waden_wu"
#define DRIVER_DESC    "RPi hc-sr04 Output Demo"
#define DRIVER_NAME	"hcsr04"
void show_result(void);
struct timeval time2;
struct timeval time1;
int time,counter=0;
static int devnum = 1;
dev_t hcsrdev;
static struct cdev devone_cdev;
static struct kobject *hcsr_kobj;
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
struct hcsr04{
		
	struct device			*dev;
	int				gpio;
	int				irq;
	char				distance;
}hc;

void show_result()
{
	pr_info("enter %s", __func__);
	pr_info("time1 = %d \n",time1.tv_usec);
	pr_info("time2 = %d \n",time2.tv_usec);	
	time = time2.tv_usec-time1.tv_usec;	
	hc.distance = time/58;
	pr_info("distance = %d cm\n",hc.distance);
}

irqreturn_t ISR(int irq,void *dev_id)
{	
	pr_info("enter %s", __func__);
	counter++;
 		switch(counter)
		{
			case 1:
				pr_info("case 1");
				do_gettimeofday(&time1);
				break;
			case 2:
				pr_info("case 2");
				do_gettimeofday(&time2);
				show_result();
				break;
		}	
	return IRQ_HANDLED;	
				
}

void hc_sr04_trigger(struct hcsr04 hc )
{ 	
	pr_info("enter %s", __func__);
	gpio_direction_output(hc.gpio, 1);	
	udelay(10);
	gpio_direction_output(hc.gpio, 0);
}

static const struct of_device_id hcsr04_dt[] = {
	{ .compatible = "hcsr04" },
	{ }
};
MODULE_DEVICE_TABLE(of,hcsr04_dt);

static ssize_t hcsr04_show(struct kobject *kobj,struct kobj_attribute *attr,const char *buf)
{	
	return sprintf(buf,"%c\n",hc.distance);
}
static ssize_t hcsr04_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf,size_t count)
{	
	pr_info("enter %s ",__func__);
	counter=0;
	hc_sr04_trigger(hc);
	return count;	
}

static struct kobj_attribute hcsr_attribute =
	__ATTR(hcsr04test,0664,hcsr04_show,hcsr04_store);

static struct attribute *hcsrattrs[] = {
&hcsr_attribute.attr,
NULL,
};

static struct attribute_group hcsrattr_group ={
.attrs =hcsrattrs,

};
static ssize_t hcsr_read(struct file *filp,const char *buf,size_t count,loff_t *ppos)
{

	pr_info("enter %s",__func__);
	pr_info("%c\n",hc.distance);
	counter=0;
	hc_sr04_trigger(hc);
	mdelay(1000);
	copy_to_user(buf,&hc.distance,count);
	return count;
}
static int hcsr_open(struct inode *inode,struct file *filp)
{
	pr_info("enter %s",__func__);
	
	return 0;
}
struct file_operations hcsr_fops =
{	
	read:			hcsr_read,
	open:			hcsr_open,
	
};
static int hcsr04_probe(struct platform_device *pdev)
 {
	
	struct device_node *node= pdev->dev.of_node;
 	int ret;
	struct class_device *class_dev = NULL;
	static struct class *devone_class=NULL;	
	pr_info("enter %s ",__func__);
	
	if(!node)
		{
			pr_info("the node is NULL");
			return -ENODEV;
		}
	/* get trig gpio */ 	
	ret=of_get_named_gpio_flags(node,"hc-trig",0,NULL); 
	if(!gpio_is_valid(ret))
		{
		if(ret!=-EPROBE_DEFER)
			pr_err("Error to get hc triggpio\n");
			return ret;
		}
	hc.gpio=ret;

        ret = gpio_request(hc.gpio, "triggpio");
        if (ret) {
         	  pr_err("gpio_request can not request trig gpio\n");
       		 }
	/* get trig gpio */ 	
	
	/* get echo gpio and set irq */
	ret=of_get_named_gpio_flags(node,"hc-irq",0,NULL); 
	if(!gpio_is_valid(ret))
		{
		if(ret!=-EPROBE_DEFER)
			pr_err("Error to get hc echogpio\n");
			return ret;
		}	
	hc.irq=ret;

	ret = gpio_request(hc.irq, "echogpio");
        if (ret) {
         	  pr_err("gpio_request can not request echo gpio\n");
       		 }

	ret=request_irq(gpio_to_irq(hc.irq),ISR,IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,"testhcsr04",NULL);
	if(ret)
		{
		pr_info("request irq failed");
		}
	/* get echo gpio and set irq */
	
	hc_sr04_trigger(hc);

	/*sysfs*/
	hcsr_kobj=kobject_create_and_add("hcsrtest",NULL);
		if(!hcsr_kobj)
			{return -ENOMEM;}
	
	ret=sysfs_create_group(hcsr_kobj,&hcsrattr_group);
		if(ret)
			kobject_put(hcsr_kobj);
			
	/*sysfs*/
	
	///////test file operation///////
		ret = alloc_chrdev_region(&hcsrdev,0,devnum,"testhcsr04device");	
	if(ret)		
	{
		pr_info("can't get major");
	}
	else
	{		
		pr_info("get major success major number: %i",MAJOR(hcsrdev));
	}		
	devone_class = class_create(THIS_MODULE,"hcsrdev");

	if(IS_ERR(devone_class))
	{
		pr_info("devone_class error");
	}	
	class_dev = device_create(devone_class,NULL,&hcsrdev,NULL,"testhcsr04device");
	if(class_dev==NULL)
	{
		pr_info("device_create failed");
	}		
	cdev_init(&devone_cdev,&hcsr_fops);
	ret=cdev_add(&devone_cdev,&hcsrdev,1);
	if(ret==-1)
	{
		pr_info("cdev_add error ");
	}
	return ret;
	///////test file operation///////
}





static struct platform_driver hcsr04_driver = {
	.driver = {
		.name	= "hcsr04",
		.of_match_table = hcsr04_dt,
	},
	.probe  = hcsr04_probe,
	
};
module_platform_driver(hcsr04_driver);

