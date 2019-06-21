
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#define DEV_NAME "ku_house_in_dev"

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1

#define KU_HOUSE_IN_IOCTL_NUM 'z'
#define KU_HOUSE_IN_ULTRA _IOWR(KU_HOUSE_IN_IOCTL_NUM, IOCTL_NUM1, unsigned long *)


static int dev;
unsigned int value=0;

void open_ultra_detection(void){

	dev = open("/dev/ku_house_in_dev", O_RDWR);
}

void start_ultra_detection(void){

	ioctl(dev, KU_HOUSE_IN_ULTRA, &value);

}

void close_ultra_detection(void){

	close(dev);

}
