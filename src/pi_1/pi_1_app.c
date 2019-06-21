#include "pi_1.h"

#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


#define DEV_NAME "pi_1_dev"


int main(int argc, char *argv[]){
    if(argc == 2){
        strcpy(ip_, "tcp://");
        strcat(ip_, argv[1]);
        strcat(ip_, ":1883");
    }else{
        //default
        strcpy(ip_, "tcp://10.42.0.1:1883");
    }

	fd_ = open("/dev/pi_1_dev", O_RDWR);
	if(fd_ == -1){
	    printf("file open error\n");
	    return -1;
	}
    MQTTClient_message msg; 
    msg = mqtt_init();


    char str[15];
	while(1){
        read_light_sensor(str);
        publish_msg(str, "/light", msg);
		sleep(5);
	}


	close(fd_);
    return 0;
}
