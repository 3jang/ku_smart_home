#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include "MQTTClient.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


#define ADDRESS     "tcp://10.42.0.1:1883"
#define CLIENTID    "lightPub"
#define TOPIC       "/light"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L

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


int fd_;
MQTTClient client;
MQTTClient_deliveryToken token;
int rc;
char ip_[40];

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt);
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
void connlost(void *context, char *cause);
int read_light_sensor(char *str);
void publish_msg(char* str, char* topic, MQTTClient_message pubmsg);
MQTTClient_message mqtt_init();
