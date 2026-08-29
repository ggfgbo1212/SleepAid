#include "mqtt_ota_ali.h"
#include "mqtt_base.h"
#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include "errno.h"
#include "printf.h"

//1. 设备上报OTA模块版本                      /ota/device/inform/${productKey}/${deviceName}
//2、订阅服务端ota升级推送信息                /ota/device/upgrade/k28qfd0fv6b/BOBEN
//3、设备端发起升级请求(获取升级包信息)       /sys/k28qfd0fv6b/BOBEN/thing/ota/firmware/get
//响应Topic：/sys/k28qfd0fv6b/BOBEN/thing/ota/firmware/get_reply
//4、下载文件分片                             /sys/k28qfd0fv6b/BOBEN/thing/file/download
//响应Topic：/sys/k28qfd0fv6b/BOBEN/thing/file/download_reply
//5、设备端向服务端推送升级进度到Topic：       /ota/device/progress/k28qfd0fv6b/BOBEN


static char InformTopic[64] = "";              //上报版本号topic
static char UpgradeTopic[64] = "";             //订阅服务器推送ota升级信息topic
static char GetFirmwareTopic[64] = "";         //获取ota升级包信息topic
static char FirmwareReplyTopic[64] = "";       //订阅服务器下发ota升级包信息topic
static char FileDownloadTopic[64] = "";        //获取ota升级包分片topic
static char FileDownloadReplyTopic[64] = "";   //订阅服务器下发ota升级包topic
static char ProgressTopic[64] = "";            //上报ota升级进度

static UpgradeInfo pUpgradeInfo;//升级包信息
static DownloadInfo pDownloadInfo;//一次Bin文件分片info
/**
 * @brief 返回升级包信息结构体
 * 
 */
UpgradeInfo* GetUpgradeInfo()
{
    return &pUpgradeInfo;
}

/**
 * @brief 返回一次Bin文件分片info结构体
 * 
 */
DownloadInfo* GetDownloadInfo()
{
    return &pDownloadInfo;
}

/**
 * @brief 初始化所有MQTT主题topic
 * 
 */
void MQTTOTA_TopicInit()
{
    sprintf(InformTopic, "/ota/device/inform/%s/%s", ProductKey, DeviceName);
    sprintf(UpgradeTopic, "/ota/device/upgrade/%s/%s", ProductKey, DeviceName);
    sprintf(GetFirmwareTopic, "/sys/%s/%s/thing/ota/firmware/get", ProductKey, DeviceName);
    sprintf(FirmwareReplyTopic, "/sys/%s/%s/thing/ota/firmware/get_reply", ProductKey, DeviceName);
    sprintf(FileDownloadTopic, "/sys/%s/%s/thing/file/download", ProductKey, DeviceName);
    sprintf(FileDownloadReplyTopic, "/sys/%s/%s/thing/file/download_reply", ProductKey, DeviceName);
    sprintf(ProgressTopic, "/ota/device/progress/%s/%s", ProductKey, DeviceName);
}


//{
//    "id": "1",
//    "params": {
//        "version": "0.0.0",
//        "module": "car01"
//    }
//}
/**
 * @brief 上报当前版本号
 * 
 */
int MQTTOTA_InformVersion(const unsigned int msgId, const char *version, const char *module)
{
    if(version == NULL || module == NULL)  return -EINVAL;
    
    char ID[10] = "";
    sprintf(ID, "%d", msgId);
    
    // 创建cJSON对象
    cJSON *obj = cJSON_CreateObject();
    if(NULL == obj) return -EIO;
    // 给obj添加字符串
    cJSON_AddStringToObject(obj, "id", ID);
    // 创建params对象
    cJSON *params = cJSON_CreateObject();
    // 给params添加字符串
    cJSON_AddStringToObject(params, "version", version);
    cJSON_AddStringToObject(params, "module", module);
    // 添加一个项目到主对象
    cJSON_AddItemToObject(obj, "params", params);

    char *objString = cJSON_Print(obj);
    
    MQTTMessage message;
    message.qos = QOS1;
    message.retained = 0;
    message.payload = objString;
    message.payloadlen = strlen(objString);
    
    //printf("topic=%s, payload=%s, len=%d\r\n", InformTopic, objString, strlen(objString));

    int rc = MQTTBase_Publish(InformTopic, &message);
    
    //释放内存
    cJSON_Delete(obj);
    cJSON_free(objString);
    
    return rc;
}

//{
//    "code":"1000",
//    "data":
//    {
//        "size":5092,                  有用
//        "streamId":14267,             有用
//        "sign":"fbac088f122af1da2fe424f5ccee4320",
//        "dProtocol":"mqtt",
//        "version":"1.0.0",     有用
//        "signMethod":"Md5",
//        "streamFileId":1,      有用
//        "md5":"fbac088f122af1da2fe424f5ccee4320"
//    },
//    "id":1703410472799,
//    "message":"success"
//}
// 解析服务器推送的升级包信息的消息处理回调函数
//TODO 解析MD5值
static void GetUpgradeMsgHandler(MessageData* msgData)
{
    char *string = (char *)msgData->message->payload;
    printf("GetUpgradeMsgHandler: %s\r\n", string);
    
    cJSON *obj = cJSON_Parse(string);
    if(NULL == obj)     return;
    //寻找data
    cJSON *data = cJSON_GetObjectItemCaseSensitive(obj, "data");
    if(NULL == data)    return;
    
    //获取升级包的大小
    cJSON *size = cJSON_GetObjectItemCaseSensitive(data, "size");
    if(NULL == size)    
    {
        pUpgradeInfo.isGetUpgrade = 2;//当前没有版本可升级
        return;
    }
    if(cJSON_IsNumber(size))
    {
        pUpgradeInfo.fileSize = size->valueint;
        //debugprintf("size->valueint: %d\r\n", size->valueint);
    }
    
    //获取升级包的streamId
    cJSON *streamId = cJSON_GetObjectItemCaseSensitive(data, "streamId");
    if(NULL == streamId)    return;
    if(cJSON_IsNumber(streamId))
    {
        pUpgradeInfo.streamId = streamId->valueint;
    }
    
    //获取升级包的version
    cJSON *version = cJSON_GetObjectItemCaseSensitive(data, "version");
    if(NULL == version)    return;
    if(cJSON_IsString(version))
    {
        unsigned int len = strlen(version->valuestring);
        if(len >= sizeof(pUpgradeInfo.version))
            len = sizeof(pUpgradeInfo.version);
        memcpy(pUpgradeInfo.version, version->valuestring, len);
    }
    
    //获取升级包的大小
    cJSON *streamFileId = cJSON_GetObjectItemCaseSensitive(data, "streamFileId");
    if(NULL == streamFileId)    return;
    if(cJSON_IsNumber(streamFileId))
    {
        pUpgradeInfo.streamFileId = streamFileId->valueint;
    }
    
    //获取升级包的md5
    cJSON *md5 = cJSON_GetObjectItemCaseSensitive(data, "md5");
    if(NULL == md5)    return;
    if(cJSON_IsString(md5))
    {
        //储存升级包的md5哈希值
        memcpy(pUpgradeInfo.md5, md5->valuestring, strlen(md5->valuestring));
        debugprintf("ota bin md5 :%s\r\n", md5->valuestring);
        debugprintf("ota bin md5 :%s\r\n", pUpgradeInfo.md5);
    }
    
    printf("ota bin data: size:%d, version:%s streamId: %d streamFileId: %d\r\n", pUpgradeInfo.fileSize, pUpgradeInfo.version, pUpgradeInfo.streamId,pUpgradeInfo.streamFileId);
    
    //获取标志位置1
    pUpgradeInfo.isGetUpgrade = 1;
    cJSON_Delete(obj);
}

/**
 * @brief 订阅服务器推送ota升级信息
 * 
 */
int MQTTOTA_Upgrade(void)
{
    return MQTTBase_Subscribe(UpgradeTopic, QOS1, GetUpgradeMsgHandler);
}

// 设备端主动获取服务器的升级包信息
int MQTTOTA_GetFirmware(const unsigned int msgId, const char *module)
{
    char msgIdString[8];
    sprintf(msgIdString, "%d", msgId);
    // 创建cJSON对象
    cJSON *obj = cJSON_CreateObject();
    if(NULL == obj) return -1;
    // 给主对象添加ID
    cJSON_AddStringToObject(obj, "id", msgIdString);
    // 给主对象添加version
    cJSON_AddStringToObject(obj, "version", "1.0");
    // 创建params对象
    cJSON *params = cJSON_CreateObject();
    // 给params添加字符串
    cJSON_AddStringToObject(params, "module", module);
    // 将params添加到主对象
    cJSON_AddItemToObject(obj, "params", params);
    // 给主对象添加method
    cJSON_AddStringToObject(obj, "method", "thing.ota.firmware.get");
    // 获取cjson主对象的字符串表达
    char *objString = cJSON_Print(obj);
    
    MQTTMessage msg;
    msg.qos = QOS1;
    msg.retained = 0;
    msg.payload = (char*)objString;
    msg.payloadlen = strlen(objString);
    
    //printf("topic=%s, payload=%s, len=%d\r\n", GetFirmwareTopic, objString, strlen(objString));
    int rc = MQTTBase_Publish(GetFirmwareTopic, &msg);
    
    // 释放CJSON占用的内存
    cJSON_Delete(obj);
    cJSON_free(objString);
    
    return rc;
}

// 设备端订阅服务器对于主动获取升级包信息的请求的响应的topic
int MQTTOTA_SubFirmwareReply(void)
{
    int rc = MQTTBase_Subscribe(FirmwareReplyTopic, QOS1, GetUpgradeMsgHandler);
    return rc;
}

// 设备端请求下载Bin文件
int MQTTOTA_GetFirmwareBin(const unsigned int msgId, const unsigned int size, const unsigned int offset)
{
    // 清零升级包数据下载标志位
    pDownloadInfo.isDownload = 0;
    
    char msgIdString[8];
    sprintf(msgIdString, "%d", msgId);
    // 创建cJSON对象
    cJSON *obj = cJSON_CreateObject();
    if(NULL == obj) return -1;
    // 给主对象添加ID
    cJSON_AddStringToObject(obj, "id", msgIdString);
    // 给主对象添加version
    cJSON_AddStringToObject(obj, "version", "1.0");
    // 创建params对象
    cJSON *params = cJSON_CreateObject();
    // 创建fileInfo对象
    cJSON *fileInfo = cJSON_CreateObject();
    // 给fileInfo对象添加streamId
    cJSON_AddNumberToObject(fileInfo, "streamId", pUpgradeInfo.streamId);
    // 给fileInfo对象添加fileId
    cJSON_AddNumberToObject(fileInfo, "fileId", pUpgradeInfo.streamFileId);
    // 将fileInfo对象添加到params对象中
    cJSON_AddItemToObject(params, "fileInfo", fileInfo);
    // 创建fileBlock对象
    cJSON *fileBlock = cJSON_CreateObject();
    // 给fileBlock对象添加size
    cJSON_AddNumberToObject(fileBlock, "size", size);
    // 给fileBlock对象添加offset
    cJSON_AddNumberToObject(fileBlock, "offset", offset);
    // 将fileBlock添加到params
    cJSON_AddItemToObject(params, "fileBlock", fileBlock);
    // 将params添加到主对象
    cJSON_AddItemToObject(obj, "params", params);
    // 获取主对象的字符串表达
    char *objString = cJSON_Print(obj);
    

    // 发布请求数据分片下载
    MQTTMessage msg;
    msg.qos = QOS1;
    msg.retained = 0;
    msg.payload = (char*)objString;
    msg.payloadlen = strlen(objString);
    int rc = MQTTBase_Publish(FileDownloadTopic, &msg);
    
    // 释放CJSON占用的内存
    cJSON_Delete(obj);
    cJSON_free(objString);
    
    return rc;
}

// \{"code":200,"data":{"bOffset":0,"fileLength":5092,"bSize":256},"id":"1","message":"success"}  1 }      KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK 
// 解析服务器下发的Bin文件分片
static void GetUpgradeBinHandler(MessageData* msgData)
{
    char *string = (char *)msgData->message->payload;
    string = string + 2;//去除前面的空格+\
    
    
    
    
    cJSON *obj = cJSON_Parse(string);
    if(NULL == obj)     return;
    //寻找data
    cJSON *data = cJSON_GetObjectItemCaseSensitive(obj, "data");
    if(NULL == data)    return;
    
    //当前分片在bin文件中的偏移量
    cJSON *bOffset = cJSON_GetObjectItemCaseSensitive(data, "bOffset");
    if(NULL == bOffset)    return;
    if(cJSON_IsNumber(bOffset)) 
    {
        pDownloadInfo.bOffset = bOffset->valueint;
    }
    
    //当前分片的大小
    cJSON *bSize = cJSON_GetObjectItemCaseSensitive(data, "bSize");
    if(NULL == bSize)    return;
    if(cJSON_IsNumber(bSize)) 
    {
        pDownloadInfo.bSize = bSize->valueint;
    }
    
    //总bin文件的大小
    cJSON *fileLength = cJSON_GetObjectItemCaseSensitive(data, "fileLength");
    if(NULL == fileLength)    return;
    if(cJSON_IsNumber(fileLength)) 
    {
        pDownloadInfo.fileLength = fileLength->valueint;
    }
    
    // 获取升级包的分片数据
    char *p = strstr(string, "success\"}");
    if(NULL == p)   return;
    p = p + strlen("success\"}");
    memcpy(pDownloadInfo.data, p, pDownloadInfo.bSize);
    
//    printf("ota data: bSize=%d, payloadlen=%d\r\n",
//       pDownloadInfo.bSize,     // 平台声明这一片该多大（来自JSON）
//       msgData->message->payloadlen);  // Paho实际收到的原始字节总数
    
    
    pDownloadInfo.isDownload = 1;
    cJSON_Delete(obj);
}


// 设备端订阅服务器下发bin文件分片
int MQTTOTA_Subdownload_reply(void)
{
    int rc = MQTTBase_Subscribe(FileDownloadReplyTopic, QOS1, GetUpgradeBinHandler);
    return rc;
}

// 设备端上报升级的进度
int MQTTOTA_ImportProgress(const unsigned int msgId, const unsigned char step, const char *module)
{
    char msgIdString[8];
    sprintf(msgIdString, "%d", msgId);
    // 创建cJSON对象
    cJSON *obj = cJSON_CreateObject();
    if(NULL == obj) return -1;
    // 给主对象添加ID
    cJSON_AddStringToObject(obj, "id", msgIdString);
    // 创建params对象
    cJSON *params = cJSON_CreateObject();
    // 给params添加step字符串
    char stepString[4];
    sprintf(stepString, "%d", step);
    cJSON_AddStringToObject(params, "step", stepString);
    // 给params添加desc
    cJSON_AddStringToObject(params, "desc", "success");
    // 给params添加module
    cJSON_AddStringToObject(params, "module", module);
    // 将params添加到主对象
    
    // 将params添加到主对象
    cJSON_AddItemToObject(obj, "params", params);
    // 获取主对象的字符串表达
    char *objString = cJSON_Print(obj);
    
    // 发布请求数据分片下载
    MQTTMessage msg;
    msg.qos = QOS1;
    msg.retained = 0;
    msg.payload = (char*)objString;
    msg.payloadlen = strlen(objString);
    int rc = MQTTBase_Publish(ProgressTopic, &msg);
    
    // 释放CJSON占用的内存
    cJSON_Delete(obj);
    cJSON_free(objString);
    
    return rc;
}
