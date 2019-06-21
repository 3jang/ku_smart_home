#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#include <unistd.h>

#define ADDRESS     "tcp://10.42.0.1:1883"
#define CLIENTID    "temphumPub"
#define TOPIC       "/temperature"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L


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
	void* humidityP;
	void* temperatureP;
};

MQTTClient client;
MQTTClient_deliveryToken token;
int rc;
struct pointers arg;
int *humidity;
int *temperature;


char ip_[40];
int fd_;

void publish_msg(char* str, char* topic, MQTTClient_message pubmsg);
MQTTClient_message mqtt_init();
void close_mqtt();
