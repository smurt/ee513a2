#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "MQTTClient.h"

#include <math.h>
#include <json-c/json.h>

#define ADDRESS     "tcp://192.168.0.11:1883"
#define CLIENTID    "pi2"
#define AUTHMETHOD  "sinead"
#define AUTHTOKEN   "murtagh"
#define TOPIC       "ee513/test"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L

/*
 * This subscription will calculate yaw,pitch and roll from accelerometer, and print.
 */
void removeGravity(int &rawX,
		int &rawY,
		int &rawZ,
		double &accelX,
		double &accelY,
		double &accelZ) {

    const float alpha = 0.5;
    
    // remove gravity component with low-pass filter
    accelX = rawX * alpha + (accelX * (1.0 - alpha));
    accelY = rawY * alpha + (accelY * (1.0 - alpha));
    accelZ = rawZ * alpha + (accelZ * (1.0 - alpha));
}

int getPitch(int rawX, int rawY, int rawZ) {

    double accelX, accelY, accelZ = 0;
    // remove gravity
    removeGravity(rawX,rawY,rawZ,accelX,accelY,accelZ);

    int pitch = (atan2(accelX, sqrt(accelY*accelY + accelZ*accelZ))*180.0)/M_PI;
    return pitch;
}

int getRoll(int rawX, int rawY, int rawZ) {

    double accelX, accelY, accelZ = 0;
    // remove gravity
    removeGravity(rawX,rawY,rawZ,accelX,accelY,accelZ);

    int roll = (atan2(-accelY, accelZ)*180.0)/M_PI;
    return roll;
}

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt) {
    printf("SUBSCRIBE1: Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    // using json-c library to parse json message
    struct json_object *parsed_json;
    struct json_object *time;
    struct json_object *accelerometer;
    struct json_object *rawX;
    struct json_object *rawY;
    struct json_object *rawZ;

    parsed_json = json_tokener_parse((char*)message->payload);
    json_object_object_get_ex(parsed_json, "CurrentTime", &time);
    json_object_object_get_ex(parsed_json, "Accelerometer", &accelerometer);
    json_object_object_get_ex(accelerometer, "X", &rawX);
    json_object_object_get_ex(accelerometer, "Y", &rawY);
    json_object_object_get_ex(accelerometer, "Z", &rawZ);

    printf("Current Time: %s              (topic: %s)\n\n", json_object_get_string(time), topicName);

    int pitch = getPitch(json_object_get_int(rawX), json_object_get_int(rawY), json_object_get_int(rawZ));
    int roll  =  getRoll(json_object_get_int(rawX), json_object_get_int(rawY), json_object_get_int(rawZ));
    printf("Accelerometer Pitch: %.02d degrees     (topic: %s)\n", pitch, topicName);
    printf("Accelerometer Roll: %.02d degrees     (topic: %s)\n", roll, topicName);
    printf("\n\n");

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

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
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


