#ifndef __MQTT_OTA_ALI_H__
#define __MQTT_OTA_ALI_H__


//服务端推动ota升级包的信息  &  主动获取ota升级包的信息
typedef struct UpgradeInfo{
    unsigned char isGetUpgrade;//此包是否有效
    unsigned int fileSize;
    unsigned int streamId;
    char version[16];
    unsigned int streamFileId;
    char md5[32];
}UpgradeInfo;


//一次Bin文件分块的info
typedef struct DownloadInfo{
    unsigned char isDownload;//此包是否有效
    unsigned int bOffset;   //当前分片的偏移量
    unsigned int fileLength;//总文件大小
    unsigned int bSize;     //当前分片大小
    unsigned char data[1024];
}DownloadInfo;

UpgradeInfo* GetUpgradeInfo(void);
DownloadInfo* GetDownloadInfo(void);

//上报版本号
int MQTTOTA_InformVersion(const unsigned int msgId, const char *version, const char *module);
void MQTTOTA_TopicInit(void);

//订阅服务器推送ota升级包信息
int MQTTOTA_Upgrade(void);

//主动获取ota升级包信息
int MQTTOTA_GetFirmware(const unsigned int msgId, const char *module);
int MQTTOTA_SubFirmwareReply(void);

// 设备端订阅服务器下发bin文件分片
int MQTTOTA_GetFirmwareBin(const unsigned int msgId, const unsigned int size, const unsigned int offset);
int MQTTOTA_Subdownload_reply(void);

// 设备端上报升级的进度
int MQTTOTA_ImportProgress(const unsigned int msgId, const unsigned char step, const char *module);

#endif /* __MQTT_OTA_ALI_H__ */
