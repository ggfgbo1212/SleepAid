#include "mqtt_base.h"
#include "printf.h"
#include "errno.h"

#include "FreeRTOS.h"
#include "semphr.h"

static MQTTClient client;
static Network network;
static unsigned char sendbuf[2048], readbuf[2048];

/* MQTT 全局互斥锁：mqtt_task 的 MQTTBase_Yield 与 UI 线程（LVGL 回调）的
 * MQTTBase_Publish/Subscribe 并发访问同一个 client + sendbuf/readbuf，
 * 必须串行化。在 MQTTBase_Init() 里创建。 */
static SemaphoreHandle_t mqtt_mutex = NULL;

static void mqtt_lock(void)
{
    if(NULL != mqtt_mutex)
        xSemaphoreTake(mqtt_mutex, portMAX_DELAY);
}

static void mqtt_unlock(void)
{
    if(NULL != mqtt_mutex)
        xSemaphoreGive(mqtt_mutex);
}

/**
 * @brief MQTT 入网初始化
 * 
 */
void MQTTBase_Init(void)
{
	if(NULL == mqtt_mutex)
		mqtt_mutex = xSemaphoreCreateMutex();
	if(NULL == mqtt_mutex)
		debugprintf("mqtt_mutex create failed\r\n");

	NetworkInit(&network);
	MQTTClientInit(&client, &network, 10000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));
}

/**
 * @brief MQTT 连接阿里云平台
 * 
 */
int MQTTBaseConnect(void)
{
    int rc = 0;
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
    
    mqtt_lock();   /* 连接过程操作共享 client，需串行化 */

    char* address = "iot-06z00fj5kcoes6j.mqtt.iothub.aliyuncs.com";
	if ((rc = NetworkConnect(&network, address, 1883)) != 0)
    {
        printf("Return code from network connect is %d\r\n", rc);
    }


    //使用官方算法自动生成密码等信息
    char clientId[150] = {0};
    char username[65] = {0};
    char password[65] = {0};
    aiotMqttSign(ProductKey, DeviceName, DeviceSecret, clientId, username, password);

	connectData.MQTTVersion = 3;
	connectData.clientID.cstring = clientId;
    connectData.username.cstring = username;
    connectData.password.cstring = password;

    rc = MQTTConnect(&client, &connectData);
	if (rc != 0)
    {
        printf("Return code from MQTT connect is %d\r\n", rc);
    }
	else
		printf("MQTT Connected\r\n");

    mqtt_unlock();

    return rc;
}

//订阅主题默认处理函数
void Default_messageArrived(MessageData* data)
{
	printf("Message arrived on topic %.*s: %.*s\r\n", data->topicName->lenstring.len, data->topicName->lenstring.data,
		data->message->payloadlen, data->message->payload);
}

/**
 * @brief 默认MQTT主题订阅信息处理函数
 * 
 */
int MQTTBase_Subscribe(const char* topicFilter, enum QoS qos, messageHandler messageHandler)
{
    if(topicFilter == NULL) return -ENAVAIL;
    if(messageHandler == NULL) messageHandler = Default_messageArrived;

    mqtt_lock();
    int rc = MQTTSubscribe(&client, topicFilter, qos, messageHandler);
    mqtt_unlock();
	if (rc != 0)
    {
        printf("Return code from MQTT subscribe is %d\r\n", rc);
    }

    return rc;
}

/**
 * @brief MQTT 取消订阅主题
 * 
 * topicFilter:topic 
 */
int MQTTBase_UnSubscribe(const char* topicFilter)
{
    if(topicFilter == NULL)  return -EINVAL;

    mqtt_lock();
    int rc = MQTTUnsubscribe(&client, topicFilter);
    mqtt_unlock();
    if (rc != 0)
    {
        debugprintf("Return code from MQTT Unsubscribe is %d\r\n", rc);
    }

    return rc;
}

/**
 * @brief MQTT 发布信息
 * 
 * topicFilter:topic   message:消息内容
 */
int MQTTBase_Publish(const char* topicFilter, MQTTMessage* message)
{
    if(topicFilter == NULL)  return -EINVAL;

    mqtt_lock();
    int rc = MQTTPublish(&client, topicFilter, message);
    mqtt_unlock();
    if (rc != 0)
        printf("Return code from MQTT publish is %d\r\n", rc);

    return rc;
}

/**
 * @brief MQTT核心函数-心跳，需要定期执行
 * 
 * timeout_ms；阻塞时间
 */
int MQTTBase_Yield(int timeout_ms)
{
    mqtt_lock();
    int rc = MQTTYield(&client, timeout_ms);
    mqtt_unlock();
    if (rc != 0)
        printf("Return code from yield is %d\r\n", rc);

    return rc;
}


