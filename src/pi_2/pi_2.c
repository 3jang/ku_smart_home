#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/fcntl.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");

#define MAX_TIMING 85
#define DHT11 21
#define SENSOR1 17
#define LED1 24
#define DEV_NAME "test_dht11_dev"

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2
#define IOCTL_NUM3 IOCTL_START_NUM+3
#define IOCTL_NUM4 IOCTL_START_NUM+4

#define DHT_IOCTL_NUM 'z'
#define SENSE_DHT _IOWR(DHT_IOCTL_NUM, IOCTL_NUM1, unsigned long*)
#define SEND_DHT _IOWR(DHT_IOCTL_NUM, IOCTL_NUM2, unsigned long*)
#define IN _IOWR(DHT_IOCTL_NUM, IOCTL_NUM3, unsigned long*)
#define OUT _IOWR(DHT_IOCTL_NUM, IOCTL_NUM4, unsigned long*)


struct pointers{
	int* humidityP;
	int* temperatureP;
};

static int dht11_data[5] = {0,};
static int *humidity;
static int *temperature;
/*
static int irq_num;
static struct timer_list my_timer;

static void my_timer_func(unsigned long data){
    printk("LED OFF\n");
    gpio_set_value(LED1, 0);
}
*/
void sendToUser(int* humidityP, int* temperatureP){

	copy_to_user(humidityP, humidity, sizeof(int));
	copy_to_user(temperatureP, temperature, sizeof(int));
}

void dht11_read(void){
	int last_state = 1;
	int counter = 0;
	int i=0, j=0;

	dht11_data[0] = dht11_data[1] = dht11_data[2] = dht11_data[3] = dht11_data[4] = 0;

	gpio_direction_output(DHT11,1);
	gpio_set_value(DHT11,0);
	mdelay(18);
	gpio_set_value(DHT11,1);
//	udelay(40);
	gpio_direction_input(DHT11);	
	for(i=0; i< MAX_TIMING; i++){
		counter = 0;
		while(gpio_get_value(DHT11) == last_state){
			
			counter++;
			udelay(1);
			if(counter == 255){
				break;
			}
		}
		last_state = gpio_get_value(DHT11);
		
		if(counter == 255)
			break;
		if((i >= 4) &&(i%2 ==0)){
			dht11_data[j/8] <<=1;
			if(counter > 16)
				dht11_data[j/8] |= 1;
			j++;
		}
		
	}

	if((j>=40) && (dht11_data[4] == ((dht11_data[0]+dht11_data[1]+dht11_data[2]+dht11_data[3])&0xFF))){
		*humidity = dht11_data[0]*10+dht11_data[1];
		*temperature = dht11_data[2]*10+dht11_data[3];
		printk("Humidity : %d,%d Temperature = %d,%d C\n",dht11_data[0],dht11_data[1],dht11_data[2],dht11_data[3]);
	}else{
	//	mdelay(500);
       
	//	dht11_read();	
	}
}

static long dht11_ioctl(struct file *file, unsigned int cmd, unsigned long arg){

	struct pointers *myPointer = (struct pointers*)arg;
	int i;

	switch(cmd){
		case SENSE_DHT:
  //          disable_irq(irq_num);
            for(i=0;i<10;i++){
			    dht11_read();
                mdelay(500);
			}
    //        enable_irq(irq_num);
			break;
		case SEND_DHT:
			sendToUser(myPointer->humidityP, myPointer->temperatureP);
			break;
        case IN:
      //      disable_irq(irq_num);
            break;
        case OUT:
        //    enable_irq(irq_num);
            break;
	}
	return 0;
}
/*
static int simple_sensor_open(struct inode *inode, struct file* file){
    enable_irq(irq_num);
    printk("simple_sensor open \n");
    return 0;
}
static int simple_sensor_release(struct inode *inode, struct file* file){
    disable_irq(irq_num);
    return 0;
}
static irqreturn_t simple_sensor_isr(int irq, void* dev_id){
    printk("detect \n");

    gpio_set_value(LED1, 0);
    del_timer(&my_timer);
    init_timer(&my_timer);
    my_timer.function = my_timer_func;
    my_timer.data = 1L;
    my_timer.expires = jiffies + (3 * HZ);
    add_timer(&my_timer);
    gpio_set_value(LED1, 1);

    return IRQ_HANDLED;
}
*/
struct file_operations dht_11_fops = 
{
  //  .open = simple_sensor_open,
  //  .release = simple_sensor_release,
	.unlocked_ioctl = dht11_ioctl,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init simple_dht11_init(void){
	int i =0;
    int ret; 
    
	gpio_request(DHT11,"DHT11");	
	alloc_chrdev_region(&dev_num,0,1,DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev,&dht_11_fops);
	cdev_add(cd_cdev,dev_num,1);

	humidity = (int*)kmalloc(sizeof(int),GFP_KERNEL);
	temperature = (int*)kmalloc(sizeof(int),GFP_KERNEL);
	
    /*
    gpio_request_one(SENSOR1, GPIOF_IN, "sensor1");
    irq_num = gpio_to_irq(SENSOR1);
    ret = request_irq(irq_num, simple_sensor_isr, IRQF_TRIGGER_RISING, "sensor_irq", NULL);
    if(ret){
        printk(KERN_ERR "Unable to requset IRQ: %d\n", ret);
        free_irq(irq_num, NULL);
    }else{
        disable_irq(irq_num);
    }

    my_timer.function = my_timer_func;
    gpio_request_one(LED1, GPIOF_OUT_INIT_LOW, "led1");
    gpio_set_value(LED1, 0);
*/
    

	return 0;
}

static void __exit simple_dht11_exit(void){
	//del_timer(&my_timer);
    gpio_set_value(DHT11,0);
	gpio_free(DHT11);
    
    //free_irq(irq_num, NULL);
    //gpio_free(SENSOR1);
    
    //gpio_set_value(LED1, 0);
    //gpio_free(LED1);
	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num,1);
}

module_init(simple_dht11_init);
module_exit(simple_dht11_exit);
