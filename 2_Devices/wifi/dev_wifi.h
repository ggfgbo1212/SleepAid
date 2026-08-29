#ifndef __DEV_WIFI_H
#define __DEV_WIFI_H

#include "ring_buffer.h"

//wifi工作模式
typedef enum{
    OFF = 0,
    STA = 1,
    AP = 2,
    AP_STA = 3
}WorkMode;

//socket连接类型
typedef enum{
    UDPServer = 1,
    UDPClient = 2,
    TCPServer = 3,
    TCPClient = 4,
    TCPSeed   = 5,
    SSLServer = 6,
    SSLClient = 7,
    SSLSeed   = 8
}NetType;

//一个网络连接info
typedef struct NetNode{
    char *ip;             //服务端ip
    unsigned char type;   //连接类型
    unsigned int port;    //端口
    unsigned int conID;   //ID
    RingBuffer *netbuffer; //接收信息的buffer
    struct NetNode *next; //链表管理
}NetNode;



typedef struct WiFiBtDev{
    char *name;
    unsigned char dev_status;   // 0-disconnect AP;1-connect AP     wifi连接状态
    unsigned char net_status;   // 0-disconnect;1-connect           网络协议连接状态
    int (*Init)(struct WiFiBtDev *ptdev);
    int (*WIFIConnect)(struct WiFiBtDev *ptdev, const char *username, const char *password);
    int (*WIFIDisConnect)(struct WiFiBtDev *ptdev);
    int (*NetConnect)(struct WiFiBtDev *ptdev, unsigned char type, const char *ip, unsigned int port);
    int (*NetDisconnect)(struct WiFiBtDev *ptdev, unsigned int port);
    int (*Write)(struct WiFiBtDev *ptdev, unsigned int port, unsigned char *buf, unsigned int length);
    int (*Read)(struct WiFiBtDev *ptdev, unsigned int port, unsigned char *buf, unsigned int length);
    int (*WIFIStaStatus)(struct WiFiBtDev *ptdev, unsigned int timeout);   // 查询WiFi连接状态
}WiFiBtDevice;

WiFiBtDevice *GetWIFIBTDevice(void);

//查询WiFi连接状态（发送 AT+STAINFO?，返回 +STAINFO:<status> 的数字，负数表示错误）
int WiFiBTGetStaStatus(unsigned int timeout);

#endif /* __DEV_WIFI_H */
