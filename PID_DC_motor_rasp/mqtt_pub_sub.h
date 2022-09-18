#include "MQTTClient.h"
#include <string.h>
#include <stdio.h>
#include "stdlib.h"
/*
Broker: broker.emqx.io
TCP Port: 1883 
*/
#define ADDRESS     "ws://broker.emqx.io:8083"
#define CLIENTID    "PIDControllerIot"
#define SUB_TOPIC   "data/PIDdata"
#define PUB_TOPIC   "data/motordata"
#define QOS         0
int rawData[5];
void publish(MQTTClient client, char* topic, char* payload) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = 1;
    pubmsg.retained = 0;
    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    MQTTClient_waitForCompletion(client, token, 1000L);
    printf("Message '%s' with delivery token %d delivered\n", payload, token);
}
float raw[4];

typedef struct {
    float Kp;
    float Ki;
    float Kd;
    float Setpoint;
} PIDdata;
PIDdata PID_sub;
void splitdata(char *str, PIDdata *pid)
{
    const char s[2] = "/";
    char *token;

    int index =0;
    /* lay token dau tien */
    token = strtok(str, s);

    /* duyet qua cac token con lai */
    while( token != NULL ) 
    {
        raw[index] = (float)atof(token);
        token = strtok(NULL, s);
        index ++;
    }
    pid->Kp = raw[0];
    pid->Ki = raw[1];
    pid->Kd = raw[2];
    pid->Setpoint = raw[3];

}