#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "MQTTClient.h"

#include<json-c/json.h>

#define ADDRESS     "tcp://192.168.0.11:1883"
#define CLIENTID    "pi3"
#define AUTHMETHOD  "sinead"
#define AUTHTOKEN   "murtagh"
#define TOPIC       "ee513/test"
#define PAYLOAD     "Hello World!"
#define QOS         1 
#define TIMEOUT     10000L

/*
 * This subscription will store and print current time and CPU temperature.
 */

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt) {
    printf("SUBSCRIBE2: Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    // using json-c library to parse json message
    struct json_object *parsed_json;
    struct json_object *temp;
    struct json_object *time;

    parsed_json = json_tokener_parse((char*)message->payload);
    json_object_object_get_ex(parsed_json, "CPUTemp", &temp);
    json_object_object_get_ex(parsed_json, "CurrentTime", &time);
    printf("Current Time: %s    (topic: %s)\n", json_object_get_string(time), topicName);
    printf("CPU Temp:     %d degrees  (topic: %s)\n\n", json_object_get_int(temp), topicName);

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

int main(int argc, char* argv[]) {
    MQTTClient client;
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    int rc;
    int ch;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_USER, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;

    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
    MQTTClient_subscribe(client, TOPIC, QOS);

    do {
        ch = getchar();
    } while(ch!='Q' && ch != 'q');
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}
