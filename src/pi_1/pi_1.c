#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

MODULE_LICENSE("GPL");

#define DEV_NAME "pi_1_dev"

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2
#define IOCTL_NUM3 IOCTL_START_NUM+3
#define IOCTL_NUM4 IOCTL_START_NUM+4
#define IOCTL_NUM5 IOCTL_START_NUM+5
#define IOCTL_NUM6 IOCTL_START_NUM+6

#define MCP_IOCTL_NUM 'z'
#define READ_LIGHT_SENSOR _IOWR(MCP_IOCTL_NUM, IOCTL_NUM1, unsigned long*)
#define RUN_MOTOR_90 _IOWR(MCP_IOCTL_NUM, IOCTL_NUM2, unsigned long *)
#define ALARM_START _IOWR(MCP_IOCTL_NUM, IOCTL_NUM3, unsigned long *)
#define ALARM_END _IOWR(MCP_IOCTL_NUM, IOCTL_NUM4, unsigned long *)
#define LED_ON _IOWR(MCP_IOCTL_NUM, IOCTL_NUM5, unsigned long *)
#define LED_OFF _IOWR(MCP_IOCTL_NUM, IOCTL_NUM6, unsigned long *)

#define CS 25   //SS
#define MOSI 17     //Master out
#define MISO 27     //Master in
#define CLK 22      //clock
#define CLK_DELAY  1500 // clock delay

#define PIN1 6
#define PIN2 13
#define PIN3 19
#define PIN4 26
#define MOTOR_DELAY 1000    //motor delay

#define LED 24
#define SPEAKER 12
#define DO 0
#define RAE 1
#define ME 2
#define FA 3
#define SOL 4
#define LA 5
#define SI 6
#define REST 7
#define MAX_INDEX 55
#define ON 1
#define OFF 0

int rote[] = {
    SOL, SOL, REST, ME, REST, ME, REST, SOL, REST, ME, REST, DO, DO, REST,
    RAE, RAE, REST, ME, REST, RAE, REST, DO, REST, ME, REST, SOL, SOL, REST,
    DO, REST, DO, REST, DO, REST, DO, REST, DO, REST, ME, REST, SOL, SOL, REST,
    SOL, SOL, REST, RAE, REST, FA, REST, ME, REST, RAE, REST, DO 
};

unsigned char controlBit[8][5] = {
    {1, 1, 0, 0, 0},
    {1, 1, 0, 0, 1},
    {1, 1, 0, 1, 0},
    {1, 1, 0, 1, 1},
    {1, 1, 1, 0, 0},
    {1, 1, 1, 0, 1},
    {1, 1, 1, 1, 0},
    {1, 1, 1, 1, 1}
};

int steps[8][4] = {
    {1, 0, 0, 1},
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1}
};


int read_light_sensor(int*);
void ready_to_read(int);
int read_data(void);

void set_pin(int, int, int, int);
void move_90(int);
void alarm(void);

int alarm_index;

static int alarm_state;

static void play(int note){
    int i = 0;
    for(i = 0; i < 100; i++){
	gpio_set_value(SPEAKER, 1);
	udelay(note);
	gpio_set_value(SPEAKER, 0);
	udelay(note);
    }
}

void alarm(){
	switch(rote[alarm_index++]){
	    case DO:
		play(1911);
		break;
	    case RAE:
		play(1702);
		break;
	    case ME:
		play(1516);
		break;
	    case FA:
		play(1431);
		break;
	    case SOL:
		play(1275);
		break;
	    case LA:
		play(1136);
		break;
	    case SI:
		play(1012);
		break;
	    case REST:
		mdelay(100);
		break;
	    default:
		alarm_state = OFF;
		break;
    }
	if(alarm_index == MAX_INDEX){
	    alarm_index = 0;
    }
}

void ready_to_read(int num){
    int i;
    for(i = 0; i < 6; i++){
        gpio_set_value(CLK, 1);
        udelay(CLK_DELAY);
        if(i < 5) gpio_set_value(MOSI, controlBit[num][i]);
	gpio_set_value(CLK, 0);
        udelay(CLK_DELAY);
    }
}

int read_data(){
    int i, data = 0;
    for(i = 0; i < 13; i++){
        gpio_set_value(CLK, 1);
        udelay(CLK_DELAY);
        if(i > 0){
            data |= gpio_get_value(MISO);
            data <<= 1;
        }
        gpio_set_value(CLK, 0);
        udelay(CLK_DELAY);
    }
    return data;
}

int read_light_sensor(int* arg){
    int bright, ret = 0;
    gpio_set_value(CS, 0);
    gpio_set_value(CLK, 0);
    udelay(CLK_DELAY);
    ready_to_read(0);
    bright = read_data();

    gpio_set_value(CS, 1);
    ret = copy_to_user(arg, &bright, sizeof(int));
    return ret;
}

void set_pin(int gp1, int gp2, int gp3, int gp4){
    gpio_set_value(PIN1, gp1);
    gpio_set_value(PIN2, gp2);
    gpio_set_value(PIN3, gp3);
    gpio_set_value(PIN4, gp4);
}

void move_90(int direction){
    int start, end, step, i = 0;
    if(direction == 1){
	start = 0;
	end = 1024;
    }else if(direction == -1){
	start = 1023;
	end = -1;
    }else{
	return;
    }
    for(i = start; i != end; i += direction){
	step = i % 8;
	set_pin(steps[step][0], steps[step][1], steps[step][2], steps[step][3]);
	udelay(MOTOR_DELAY);
    }
}



static long light_sensor_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
    int ret;
    int dir;
    switch(cmd){
        case READ_LIGHT_SENSOR:
            ret = read_light_sensor((int*)arg);
            printk("read_light_sensor : %d\n", ret);
            break;
	case RUN_MOTOR_90:
        printk("motor 90\n");
	    dir = *((int*)arg);
	    if(dir != 1 && dir != -1)
		    return -1;
	    move_90(dir);
	    break;
	case ALARM_START:
	    alarm();
        printk("yaya\n");
	    break;
    case ALARM_END:
        alarm_index = 0;
        break;
    case LED_ON:
        gpio_set_value(LED, 1);
        break;
    case LED_OFF:
        gpio_set_value(LED, 0);
        break;    
        
        default:
            return -1;
    }
    return 0;
}

static int light_sensor_open(struct inode *inode, struct file *file){
    printk("light sensor open\n");
    return 0;
}

static int light_sensor_release(struct inode *inode, struct file *file){
    printk("light sensor release\n");
    return 0;
}

struct file_operations light_sensor_fops = {
    .unlocked_ioctl = light_sensor_ioctl,
    .open = light_sensor_open,
    .release = light_sensor_release
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init light_sensor_init(void){
    int ret;
    printk("Init Module\n");

    alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
    cd_cdev = cdev_alloc();
    cdev_init(cd_cdev, &light_sensor_fops);
    ret = cdev_add(cd_cdev, dev_num, 1);
    if(ret < 0){
        printk("fail to add character device\n");
        return -1;
    }

    gpio_request_one(CS, GPIOF_OUT_INIT_HIGH, "CS");
    gpio_request_one(MOSI, GPIOF_OUT_INIT_LOW, "MOSI");
    gpio_request_one(MISO, GPIOF_IN, "MISO");
    gpio_request_one(CLK, GPIOF_OUT_INIT_HIGH, "SCK");

    gpio_request_one(PIN1, GPIOF_OUT_INIT_LOW, "p1");
    gpio_request_one(PIN2, GPIOF_OUT_INIT_LOW, "p2");
    gpio_request_one(PIN3, GPIOF_OUT_INIT_LOW, "p3");
    gpio_request_one(PIN4, GPIOF_OUT_INIT_LOW, "p4");
   
    gpio_request_one(LED, GPIOF_OUT_INIT_LOW, "led");
    gpio_request_one(SPEAKER, GPIOF_OUT_INIT_LOW, "speaker");
    
    //move_90(1); 
    return 0;
}

static void __exit light_sensor_exit(void){
    printk("Exit Module\n");

    cdev_del(cd_cdev);
    unregister_chrdev_region(dev_num, 1);

    gpio_set_value(LED, 0);
    gpio_set_value(CS, 1);
    gpio_free(CS);
    gpio_set_value(MOSI, 0);
    gpio_free(MOSI);
    gpio_free(MISO);
    gpio_set_value(CLK, 1);
    gpio_free(CLK);
    
    gpio_free(SPEAKER);
    gpio_free(PIN1);
    gpio_free(PIN2);
    gpio_free(PIN3);
    gpio_free(PIN4);
    gpio_free(LED);

}

module_init(light_sensor_init);
module_exit(light_sensor_exit);

