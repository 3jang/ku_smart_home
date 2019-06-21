
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/timer.h>
#include <linux/time.h>

#define ULTRA_SENSOR_TRIG1 5
#define ULTRA_SENSOR_ECHO1 6
#define ULTRA_SENSOR_TRIG2 23
#define ULTRA_SENSOR_ECHO2 24
#define ULTRA_SENSOR_TRIG3 20
#define ULTRA_SENSOR_ECHO3 21
#define LED 22

#define DEV_NAME "ku_house_in_dev"

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1

#define KU_HOUSE_IN_IOCTL_NUM 'z'
#define KU_HOUSE_IN_ULTRA _IOWR(KU_HOUSE_IN_IOCTL_NUM, IOCTL_NUM1, unsigned long*)

void stamp_ultra_pattern(void);
void register_ultra_pattern(int tempPattern);
void check_led(void);


static int pattern_count=0;
static int people_num=0;
static int old_pattern=99;
static int threshold=10;

static int in_pattern[] = {10,11,1,0};
static int out_pattern[] = {1,11,10,0};
static int my_pattern[] = {99,99,99,99};
