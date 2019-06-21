#include "pi_2.h"


int main(int argc, char *argv[]){
    if(argc == 2){
        strcpy(ip_, "tcp://");
        strcat(ip_, argv[1]);
        strcat(ip_, ":1883");
    }else{
        strcpy(ip_, "tcp://10.42.0.1:1883");
    }

    humidity = (int*)malloc(sizeof(int));
    temperature = (int*)malloc(sizeof(int));

    arg.humidityP = humidity;
    arg.temperatureP = temperature;

    fd_ = open("/dev/test_dht11_dev", O_RDWR);
    
    MQTTClient_message msg;
    msg = mqtt_init();

    char str[20];

    while(1){
//        ioctl(fd_, IN, 0);
        ioctl(fd_, SENSE_DHT, &arg);
        ioctl(fd_, SEND_DHT, &arg);
        sprintf(str, "%d,%d", *humidity, *temperature);
        if(*temperature > 0){
            publish_msg(str, "/temperature", msg);
        }
  //      ioctl(fd_, OUT, 0);
        sleep(5);
    }
    close_mqtt();
    close(fd_);
}
