#include "pi_1.h"



void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int i;
    char* payloadptr;
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: %s\n", (char*)message->payload);
    int val = ((char*)message->payload)[0] - 48;
    if( strcmp(topicName, "/buzzer") == 0){
        if(val == 0){
            printf("buzzer off\n");
            ioctl(fd_, ALARM_END, 0);
        }else if(val == 1){
            ioctl(fd_, ALARM_START, 0);
        }
    }else if( strcmp(topicName, "/curtain") == 0){
        int arg;
        if(val == 1){
            printf("curtain on\n");
            arg = 1;
            ioctl(fd_, RUN_MOTOR_90, &arg);
        }else{
            printf("curtain off\n");
            arg = -1;
            ioctl(fd_, RUN_MOTOR_90, &arg);
        }
    }
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}


void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}
int read_light_sensor(char *str){
    int bright;
    int ret;
	ret = ioctl(fd_, READ_LIGHT_SENSOR, &bright);
	printf("bright = %d\n", bright);
    sprintf(str, "%d", bright);
    return ret;
}
void publish_msg(char* str, char* topic, MQTTClient_message pubmsg){
    pubmsg.payload = str;
    pubmsg.payloadlen = strlen(str);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    printf("Waiting for up to %d seconds for publication of %s\n"
        "on topic %s for client with ClientID: %s\n",
        (int)(TIMEOUT/1000), str, topic, CLIENTID);
    rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    printf("Message with delivery token %d delivered\n", token);
}
MQTTClient_message mqtt_init(){

    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
	MQTTClient_create(&client, ip_, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d\n", rc);
		exit(-1);
	}

    
    MQTTClient_subscribe(client, "/buzzer", QOS);
    MQTTClient_subscribe(client, "/curtain", QOS);
    return pubmsg;
}
