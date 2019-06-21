#include "pi_2.h"


void publish_msg(char* str, char* topic, MQTTClient_message pubmsg){
    pubmsg.payload = str;
    pubmsg.payloadlen = strlen(str);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    printf("Waiting for up to %d seconds for publication of %s\n"
        "on topic %s for client with ClientID: %s\n",
        (int)(TIMEOUT/1000), str, TOPIC, CLIENTID);
    rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    printf("Message with delivery token %d delivered\n", token);
}

MQTTClient_message mqtt_init(){
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	MQTTClient_create(&client, ip_, CLIENTID,
	MQTTCLIENT_PERSISTENCE_NONE, NULL);
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
	printf("Failed to connect, return code %d\n", rc);
	exit(-1);
	}
    return pubmsg;
}

void close_mqtt(){
	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);
}
