#ifndef __MQTT_BASE_H__
#define __MQTT_BASE_H__

#include "MQTTClient.h"

#define ProductKey   "k28qf9fffpV"
#define DeviceName   "MQTTOTA"
#define DeviceSecret "1c92858996103ca846bb3d5ce7f93b1e"

extern int aiotMqttSign(const char *productKey, const char *deviceName, const char *deviceSecret, 
                     char clientId[150], char username[64], char password[65]);

void MQTTBase_Init(void);
int MQTTBaseConnect(void);
int MQTTBase_Subscribe(const char* topicFilter, enum QoS qos, messageHandler messageHandler);
int MQTTBase_UnSubscribe(const char* topicFilter);
int MQTTBase_Publish(const char* topicFilter, MQTTMessage* message);
int MQTTBase_Yield(int timeout_ms);

#endif /* __MQTT_BASE_H__ */

