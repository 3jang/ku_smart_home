#include "ku_house_in.h"
MODULE_LICENSE("GPL");

//=======================================================================

void stamp_ultra_pattern(void){

	unsigned long start,stop;
	struct timeval startTime,stopTime;
	int tempPattern=0;
	int distance;

	gpio_set_value(ULTRA_SENSOR_TRIG1, 1);
	mdelay(10);
	gpio_set_value(ULTRA_SENSOR_TRIG1, 0);

	while(gpio_get_value(ULTRA_SENSOR_ECHO1)==0);
	do_gettimeofday(&startTime);

	while(gpio_get_value(ULTRA_SENSOR_ECHO1)==1);
	do_gettimeofday(&stopTime);

	start = (unsigned long)startTime.tv_sec*1000000+(unsigned long)startTime.tv_usec;
	stop = (unsigned long)stopTime.tv_sec*1000000+(unsigned long)stopTime.tv_usec;

	distance = (stop-start) / 29 / 2;
	
	if(distance < threshold){
		tempPattern = tempPattern+10;
	}

	gpio_set_value(ULTRA_SENSOR_TRIG2, 1);
	mdelay(10);
	gpio_set_value(ULTRA_SENSOR_TRIG2, 0);

	while(gpio_get_value(ULTRA_SENSOR_ECHO2)==0);
	do_gettimeofday(&startTime);

	while(gpio_get_value(ULTRA_SENSOR_ECHO2)==1);
	do_gettimeofday(&stopTime);

	start = (unsigned long)startTime.tv_sec*1000000+(unsigned long)startTime.tv_usec;
	stop = (unsigned long)stopTime.tv_sec*1000000+(unsigned long)stopTime.tv_usec;

	distance = (stop-start) / 29 / 2;

	if(distance < threshold){
		tempPattern = tempPattern+1;
	}

	if(tempPattern!=old_pattern){
		
		register_ultra_pattern(tempPattern);
		old_pattern = tempPattern;
	//	mdela(50);
	}
}

void register_ultra_pattern(int tempPattern){
		
	int i;
	int count;
	if(pattern_count < 4){
		printk("%d\n",tempPattern);
		my_pattern[pattern_count] = tempPattern;
		pattern_count++;
	}else{
		for(i=0; i<3; i++){
			my_pattern[i] = my_pattern[i+1];
		}
		my_pattern[3] = tempPattern;	
		
	}
	printk("tempPattern %d/%d/%d/%d\n",my_pattern[0],my_pattern[1],my_pattern[2],my_pattern[3]);

	for(i=0; i<4; i++){
		if(my_pattern[i] != in_pattern[i]){
			break;
		}
		count++;
	}
	people_num = people_num+count/4;
	count = 0;

	for(i=0; i<4; i++){
		if(my_pattern[i] != out_pattern[i]){
			break;
		}
		count--;
	}
	people_num = people_num+count/4;
	printk("people :%d\n",people_num);
	check_led();
		
}

void check_led(){
	if(people_num > 0){
		gpio_set_value(LED,1);
	}else if(people_num == 0){
		gpio_set_value(LED,0);
	
	}else{
		people_num = 0;
	}
}
//==========================================================================


static int ku_house_in_open(struct inode *inode, struct file* file){
   printk("simple_sensor_open\n");
   return 0;
}

static int ku_house_in_release(struct inode *inode, struct file* file){
   printk("simple snesor close\n");
   return 0;
}

static long ku_house_in_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	
	switch(cmd){
		case KU_HOUSE_IN_ULTRA:
			stamp_ultra_pattern();
			break;
	}
	return 0;
}

struct file_operations ku_house_in_fops =
{
	.open = ku_house_in_open,
	.release = ku_house_in_release,
	.unlocked_ioctl = ku_house_in_ioctl,
};
//===============================================================================
static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init ku_house_in_init(void){
	int ret;

	printk("Init module\n");

	alloc_chrdev_region(&dev_num,0,1,DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev,&ku_house_in_fops);
	cdev_add(cd_cdev,dev_num,1);

	gpio_request_one(ULTRA_SENSOR_TRIG1, GPIOF_OUT_INIT_LOW, "ultra_trig1");
	gpio_request_one(ULTRA_SENSOR_ECHO1, GPIOF_IN, "ultra_echo1");
	gpio_request_one(ULTRA_SENSOR_TRIG2, GPIOF_OUT_INIT_LOW, "ultra_trig2");
	gpio_request_one(ULTRA_SENSOR_ECHO2, GPIOF_IN, "ultra_echo2");
	gpio_request_one(LED, GPIOF_OUT_INIT_LOW, "LED");

	return 0;
}


static void __exit ku_house_in_exit(void){
	struct ultra_data *tmp=0;
	struct list_head *pos=0;
	struct list_head *q =0;
	
	printk("Exit Module\n");
	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num,1);
	

	gpio_free(ULTRA_SENSOR_TRIG1);
	gpio_free(ULTRA_SENSOR_TRIG2);
	gpio_free(ULTRA_SENSOR_ECHO1);
	gpio_free(ULTRA_SENSOR_ECHO2);
	gpio_free(LED);
}

module_init(ku_house_in_init);
module_exit(ku_house_in_exit);


