// Based on the Paho C code example from www.eclipse.org/paho/
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "MQTTClient.h"

extern "C"
{
    #include "ADXL345.c"
}

#define  CPU_TEMP "/sys/class/thermal/thermal_zone0/temp"

using namespace std;

//Please replace the following address with the address of your server
#define ADDRESS    "tcp://192.168.0.11:1883"
#define CLIENTID   "pi1"
#define AUTHMETHOD "sinead"
#define AUTHTOKEN  "murtagh"
#define TOPIC      "ee513/test"
#define QOS        1
#define TIMEOUT    10000L

float getCPUTemperature() {        // get the CPU temperature
   int cpuTemp;                    // store as an int
   fstream fs;
   fs.open(CPU_TEMP, fstream::in); // read from the file
   fs >> cpuTemp;
   fs.close();
   return (((float)cpuTemp)/1000);
}

void getCurrentTime(char* currentTime) {
   time_t rpiTime = time(NULL);
   struct tm tm = *localtime(&rpiTime);
   sprintf(currentTime, "%d:%d:%d", tm.tm_hour, tm.tm_min, tm.tm_sec);
}

int main(int argc, char* argv[]) {
   char str_payload[500];          // Set your max message size here
   MQTTClient client;
   MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
   MQTTClient_message pubmsg = MQTTClient_message_initializer;
   MQTTClient_deliveryToken token;
   MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
   opts.keepAliveInterval = 20;
   opts.cleansession = 1;
   opts.username = AUTHMETHOD;
   opts.password = AUTHTOKEN;
   int rc;
   if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
      cout << "Failed to connect, return code " << rc << endl;
      return -1;
   }
   // enable accelerometer measurements
   enableAccelerometer();

   // send data 5x (so we can see accelerometer changes)
   for (int i=0; i<5; i++) {
      // get Accelerometer Data
      int xData,yData,zData = 0;
      readAccelerometerData(xData,yData,zData);
      // get CPU Temp Data
      float cpuTemp = getCPUTemperature();
      // get RPi Time
      char rpiTime[10];
      getCurrentTime(rpiTime);

      // built JSON
      sprintf(str_payload, "\n{\n");
      sprintf(str_payload + strlen(str_payload), "    \"CPUTemp\": %f,\n", cpuTemp);
      sprintf(str_payload + strlen(str_payload), "    \"CurrentTime\": \"%s\",\n", rpiTime);
      sprintf(str_payload + strlen(str_payload), "    \"Accelerometer\": {\n");
      sprintf(str_payload + strlen(str_payload), "        \"X\": %d,\n", xData);
      sprintf(str_payload + strlen(str_payload), "        \"Y\": %d,\n", yData);
      sprintf(str_payload + strlen(str_payload), "        \"Z\": %d,\n", zData);
      sprintf(str_payload + strlen(str_payload), "    }\n");
      sprintf(str_payload + strlen(str_payload), "}");

      // publish
      pubmsg.payload = str_payload;
      pubmsg.payloadlen = strlen(str_payload);
      pubmsg.qos = QOS;
      pubmsg.retained = 0;
      MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
      cout << "Waiting for up to " << (int)(TIMEOUT/1000) <<
           " seconds for publication of " << str_payload <<
           " \non topic " << TOPIC << " for ClientID: " << CLIENTID << endl;
      rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
      cout << "Message with token " << (int)token << " delivered." << endl;
      usleep(1000000);
   }
   // disable accelerometer measurements after use (RECOMMENDED)
   disableAccelerometer();
   MQTTClient_disconnect(client, 10000);
   MQTTClient_destroy(&client);
   return rc;
}
