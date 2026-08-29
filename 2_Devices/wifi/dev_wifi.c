#include "dev_wifi.h"
#include "errno.h"
#include <string.h>
#include "printf.h"
#include "dev_uart.h"
#include "drv_uart.h"
#include "ring_buffer.h"
#include <stdlib.h>


//管理每次连接驱动
static void NetNodeInsert(NetNode *ptnode);
static NetNode *NetNodeFind(unsigned int port);
static NetNode *NetNodeFind_PassConid(unsigned int conID);
static void NetNodeDelete(unsigned int port);


static int WiFiBTWaitACK(const char *ack, unsigned int timeout);
static int WIFI_Test();
static int WIFI_Reset();
static int WIFI_EchoControl(int status);
static int WiFiBTSetWorkMode(int Mode);
static int WiFiBTSetReceiveMode(unsigned char mode);


static int WifiBT_Init(struct WiFiBtDev *ptdev);
static int WifiBT_WIFIConnect(struct WiFiBtDev *ptdev, const char *username, const char *password);
static int WifiBT_WIFIDisConnect(struct WiFiBtDev *ptdev);
static int WifiBT_NetConnect(struct WiFiBtDev *ptdev, unsigned char type, const char *ip, unsigned int port);
static int WifiBT_NetDisconnect(struct WiFiBtDev *ptdev, unsigned int port);
static int WifiBT_Write(struct WiFiBtDev *ptdev, unsigned int port, unsigned char *buf, unsigned int length);
static int WifiBT_Read(struct WiFiBtDev *ptdev, unsigned int port, unsigned char *buf, unsigned int length);
static int WiFiBTReadFromBuffer(struct WiFiBtDev *ptdev, unsigned int port, unsigned char *buf, unsigned int length);

struct UARTDev* pUart = NULL;//wifi专用串口

static struct NetNode* gHeadNet = NULL;//连接头节点

//wifi对象
WiFiBtDevice gWiFiBtDevice = {
    .name = "WIFIBT",
    .dev_status = 0,
    .net_status = 0,
    .Init = WifiBT_Init,
    .WIFIConnect = WifiBT_WIFIConnect,
    .WIFIDisConnect = WifiBT_WIFIDisConnect,
    .NetConnect = WifiBT_NetConnect,
    .NetDisconnect = WifiBT_NetDisconnect,
    .Write = WifiBT_Write,
    .Read = WiFiBTReadFromBuffer
    
};

WiFiBtDevice *GetWIFIBTDevice(void)
{
    return &gWiFiBtDevice;
}


static int WifiBT_Init(struct WiFiBtDev *ptdev)
{
    if(NULL == ptdev)   return -EINVAL;
    
    //给wifi专用串口对象赋值
    pUart = UARTDev_Find("WIFIUART");
    if(NULL == pUart)   return -ENODEV;
    int ret = pUart->Init(pUart);
    if(ESUCCESS != ret) return ret;
    
    
    //发送AT测试通信
    ret = WIFI_Test();
    if(ESUCCESS != ret) return ret;
    //发送AT+RST 重启一下WiFi芯片
    ret = WIFI_Reset();
    if(ESUCCESS != ret) return ret;
    //关闭回显模式
    ret = WIFI_EchoControl(0);
    if(ESUCCESS != ret) return ret;
    // 设置WiFi工作模式
    WiFiBTSetWorkMode(AP_STA);
    if(ESUCCESS != ret) return ret;
    // 设置网络数据接收模式——主动模式
    ret = WiFiBTSetReceiveMode(1);
    if(ESUCCESS != ret) return ret;
    
    return ret;
}

/**
 * @brief 芯片连接WiFi
 * 
 * status username-wifi名称   password-密码
 */
static int WifiBT_WIFIConnect(struct WiFiBtDev *ptdev, const char *username, const char *password)
{
    if(ptdev == NULL || username == NULL || password == NULL)  return -EINVAL;
    
    char str[100];  
    sprintf(str, "AT+WJAP=%s,%s\r\n", username, password);  
    unsigned char len = strlen(str);
    
    // 先清空buffer
    int ret = pRxBuffer->Clear(pRxBuffer);
    if(ESUCCESS != ret) return ret;
    // 再发送指令
    ret = pUart->Write(pUart, (unsigned char *)str, len);
    if(len != ret) return ret;
    // 超时等待响应
    ret = WiFiBTWaitACK("WIFI_GOT_IP\r\n", 5000);//查询buffer是否有WIFI_GOT_IP
    ptdev->dev_status = 1;  // connected
    printf("%s\r\n", pRxBuffer->info.pHead);
    return ret;   
}

/**
 * @brief 芯片断开连接WiFi
 * 
 */
static int WifiBT_WIFIDisConnect(struct WiFiBtDev *ptdev)
{
    if(ptdev == NULL)  return -EINVAL;
    
    char str[50];  
    sprintf(str, "AT+WDISCONNECT\r\n");  
    unsigned char len = strlen(str);
    
    // 先清空buffer
    int ret = pRxBuffer->Clear(pRxBuffer);
    if(ESUCCESS != ret) return ret;
    // 再发送指令
    ret = pUart->Write(pUart, (unsigned char *)str, len);
    if(len != ret) return ret;
    // 超时等待响应
    ret = WiFiBTWaitACK("OK\r\n", 100);
    ptdev->dev_status = 0;  // disconnected
    return ret;   
}

static int WifiBT_NetConnect(struct WiFiBtDev *ptdev, unsigned char type, const char *ip, unsigned int port)
{
    if(NULL == ptdev || NULL == ip)  return -EINVAL;
    
    if(ptdev->dev_status != 1)  return -EIO;  //保证wifi已连接才继续连接TCP
    
    NetNode *pNode = (NetNode*)malloc(sizeof(NetNode));
    if(NULL == pNode)  return -ENOMEM;
    pNode->ip = (char*)ip;
    pNode->type = type;
    pNode->port = port;
    
    if(NULL == gHeadNet)
        pNode->conID = 1;
    else
        pNode->conID = gHeadNet->conID + 1;
    
    char str[128] = {0};
    sprintf(str, "AT+SOCKET=%d,%s,%d,0,%d\r\n", type, ip, port, pNode->conID);
    unsigned char len = strlen(str);
    // 先清空buffer
    int ret = pRxBuffer->Clear(pRxBuffer);
    if(ESUCCESS != ret)
    {
        free(pNode);
        return ret;
    }
    // 再发送指令
    ret = pUart->Write(pUart, (unsigned char *)str, len);
    if(len != ret)
    {
        free(pNode);
        return ret;
    }
    // 超时等待响应
    ret = WiFiBTWaitACK("OK\r\n", 500);
    if(ESUCCESS != ret)
    {
        debugprintf("%s\r\n", pRxBuffer->info.pHead);
        free(pNode);
        return ret;
    }
    ptdev->net_status = 1;  // 连接上socket服务器
    
    pNode->netbuffer = RingBufferNew(128);//？？？
    if(NULL == pNode->netbuffer)
    {
        free(pNode);
        return -ENOMEM;
    }
   
    NetNodeInsert(pNode);//conID连接管理
    return ESUCCESS;
    
}

static int WifiBT_NetDisconnect(struct WiFiBtDev *ptdev, unsigned int port)
{
    if(NULL == ptdev)   return -EINVAL;
    if(NULL == pUart)   return -ENODEV;
    
    NetNode *ptnode = NetNodeFind(port);
    if(NULL == ptnode)  return -ENODEV;
    
    char str[128] = {0};
    //sprintf(str, "AT+SOCKETDEL=%d\r\n", port);
    sprintf(str, "AT+SOCKETDEL=%d\r\n", ptnode->conID);
    unsigned char len = strlen(str);
    // 先清空buffer
    int ret = pRxBuffer->Clear(pRxBuffer);
    if(ESUCCESS != ret)
    {
        return ret;
    }
    // 再发送指令
    ret = pUart->Write(pUart, (unsigned char *)str, len);
    if(len != ret)
    {
        return ret;
    }
    // 超时等待响应
    ret = WiFiBTWaitACK("OK\r\n", 500);
    if(ESUCCESS != ret)
    {
        debugprintf("%s\r\n", pRxBuffer->info.pHead);
        return ret;
    }
    if(NULL == gHeadNet)
        ptdev->net_status = 0;  // 断开socket服务器的连接
    
    //将这个连接节点删除
    NetNodeDelete(port);
    
    return ESUCCESS;
}

/**
 * @brief 芯片socket连接后发送信息
 * port-端口   buf-信息  length-长度
 */
static int WifiBT_Write(struct WiFiBtDev *ptdev, unsigned int port, unsigned char *buf, unsigned int length)
{
    if(NULL == ptdev)   return -EINVAL;
    if(NULL == buf)     return -EINVAL;
    if(0 == length)     return -EINVAL;
    if(NULL == pRxBuffer)   return -ENODEV;
    if(NULL == pUart)       return -ENODEV;
    
    //检查是否连接上网络
    if(ptdev->dev_status != 1)  return -EIO;
    
    NetNode *pNode = NetNodeFind(port);
    if(NULL == pNode)       return -ENODEV;
    char str[32] = {0};
    sprintf(str, "AT+SOCKETSEND=%d,%d\r\n", pNode->conID, length);
    unsigned char len = strlen(str);
    // 先清空buffer
    int ret = pRxBuffer->Clear(pRxBuffer);
    if(ESUCCESS != ret) return ret;
    // 再发送指令
    ret = pUart->Write(pUart, (unsigned char *)str, len);
    if(len != ret)      return ret;
    // 超时等待响应
    ret = WiFiBTWaitACK(">", 100);//接收到 > 才可以开始发送数据
    if(ESUCCESS != ret) return ret;
    // 发送数据
    ret = pUart->Write(pUart, (unsigned char *)buf, length);
    if(len != ret)      return ret;
    // 超时等待响应
    ret = WiFiBTWaitACK("OK\r\n", 100);
    if(ESUCCESS != ret) return ret;
    
    return (int)length;
}

static int WifiBT_Read(struct WiFiBtDev *ptdev, unsigned int port, unsigned char *buf, unsigned int length)
{
    if(NULL == ptdev || NULL == buf)  return -EINVAL;
    
    return ESUCCESS;
}

static int WiFiBTReadFromBuffer(struct WiFiBtDev *ptdev, unsigned int port, unsigned char *buf, unsigned int length)
{
    if(NULL == ptdev)   return -EINVAL;
    if(NULL == buf)     return -EINVAL;
    if(0 == length)     return -EINVAL;
    if(NULL == pRxBuffer)   return -ENODEV;
    if(NULL == pUart)       return -ENODEV;
    if(ptdev->dev_status != 1)  return -EIO;
    
    NetNode *pNode = NetNodeFind(port);//根据端口寻找Net节点
    if(NULL == pNode)       return -ENODEV;
    
    //从Net节点获取数据
    return pNode->netbuffer->Read(pNode->netbuffer, buf, length);
}


/**
 * @brief 等待wifi芯片回复信息
 * ack回复的信息
 * timeout 超时时间
 */
static int WiFiBTWaitACK(const char *ack, unsigned int timeout)
{
    char buf[128] = {0};
    unsigned char i = 0;
    while(timeout)
    {
        int ret = pRxBuffer->Read(pRxBuffer, (unsigned char *)&buf[i], 1);
        if(ret == 1)    i = (i+1)%128;
        // 判断响应
        if(strstr(buf, ack))   return ESUCCESS;
        else if(strstr(buf, "ERR")) return -EIO;
        else if(strstr(buf, "Unknown cmd")) return -EIO;
        // 超时递减
        timeout--;
        HAL_Delay(1);
    }
    return -EIO;
}

/**
 * @brief 发送基础指令AT测试WiFi芯片
 * 
 */
static int WIFI_Test()
{
    char str[8] = "AT\r\n";
    unsigned char len = strlen(str);
    
    // 先清空buffer
    int ret = pRxBuffer->Clear(pRxBuffer);
    if(ESUCCESS != ret) return ret;
    // 再发送指令
    ret = pUart->Write(pUart, (unsigned char *)str, len);
    if(len != ret) return ret;
     // 超时等待响应
    ret = WiFiBTWaitACK("OK\r\n", 100);
    printf("%s\r\n", pRxBuffer->info.pHead);

    return ret;
}

/**
 * @brief 重启芯片
 * 
 */
static int WIFI_Reset()
{
    char str[8] = "AT+RST\r\n";
    unsigned char len = strlen(str);
    
    // 先清空buffer
    int ret = pRxBuffer->Clear(pRxBuffer);
    if(ESUCCESS != ret) return ret;
    // 再发送指令
    ret = pUart->Write(pUart, (unsigned char *)str, len);
    if(len != ret) return ret;
     // 超时等待响应
    ret = WiFiBTWaitACK("ready\r\n", 2000);
    printf("%s\r\n", pRxBuffer->info.pHead);
    
    return ret;
}

/**
 * @brief 芯片回显模式打开关闭
 * 
 * status 0-关闭回显模式 1-开启回显模式
 */
static int WIFI_EchoControl(int status)
{
    
    char str[6];  
    sprintf(str, "ATE%d\r\n", status);  
    unsigned char len = strlen(str);
    
    // 先清空buffer
    int ret = pRxBuffer->Clear(pRxBuffer);
    if(ESUCCESS != ret) return ret;
    // 再发送指令
    ret = pUart->Write(pUart, (unsigned char *)str, len);
    if(len != ret) return ret;
     // 超时等待响应
    ret = WiFiBTWaitACK("OK\r\n", 100);
    printf("%s\r\n", pRxBuffer->info.pHead);
    
    return ret;
}

/**
 * @brief 芯片wifi模式
 * 
 * status 0-关闭 1-STA 2-AP 3-AP+STA
 */
static int WiFiBTSetWorkMode(int Mode)
{
    char str[30];  
    //默认不保存flash
    sprintf(str, "AT+WMODE=%d,0\r\n", Mode);  
    unsigned char len = strlen(str);
    
    // 先清空buffer
    int ret = pRxBuffer->Clear(pRxBuffer);
    if(ESUCCESS != ret) return ret;
    // 再发送指令
    ret = pUart->Write(pUart, (unsigned char *)str, len);
    if(len != ret) return ret;
     // 超时等待响应
    ret = WiFiBTWaitACK("OK\r\n", 100);
    printf("%s\r\n", pRxBuffer->info.pHead);
    
    return ret;
}

static int WiFiBTSetReceiveMode(unsigned char mode)
{
    if(NULL == pRxBuffer) return -ENODEV;
    if(NULL == pUart)   return -ENODEV;
    char str[32] = {0};
    sprintf(str, "AT+SOCKETRECVCFG=%d\r\n", mode);
    unsigned char len = strlen(str);
    // 先清空buffer
    int ret = pRxBuffer->Clear(pRxBuffer);
    if(ESUCCESS != ret) return ret;
    // 再发送指令
    ret = pUart->Write(pUart, (unsigned char *)str, len);
    if(len != ret) return ret;
    // 超时等待响应
    ret = WiFiBTWaitACK("OK\r\n", 100);
    printf("%s\r\n", pRxBuffer->info.pHead);
    
    return ret;
}



/**
 * @brief WiFi芯片socket连接节点插入
 * 
 * @param ptnode 要插入的节点
 */
static void NetNodeInsert(NetNode *ptnode)
{    
    if(NULL == gHeadNet)
    {
        gHeadNet = ptnode;
    }
    else
    {
        ptnode->next = gHeadNet;
        gHeadNet = ptnode;
    }
}

/**
 * @brief WiFi芯片socket连接节点寻找
 * 
 * @param port 需寻找的节点的端口号
 */
static NetNode *NetNodeFind(unsigned int port)
{    
    NetNode *ptnode = gHeadNet;
    while(NULL != ptnode)
    {
        if(ptnode->port == port)
        {
            return ptnode;
        }
        ptnode = ptnode->next;
    }
    return NULL;
}

/**
 * @brief WiFi芯片socket连接节点寻找
 * 
 * @param port 需寻找的节点的conID
 */
static NetNode *NetNodeFind_PassConid(unsigned int conID)
{    
    NetNode *ptnode = gHeadNet;
    while(NULL != ptnode)
    {
        if(ptnode->conID == conID)
        {
            return ptnode;
        }
        ptnode = ptnode->next;
    }
    return NULL;
}

/**
 * @brief WiFi芯片socket连接节点删除
 * 
 * @param port 需删除的节点的端口号
 */
static void NetNodeDelete(unsigned int port)
{    
    NetNode *ptnode = gHeadNet;
    NetNode *ptmp = ptnode;
    while(NULL != ptnode)
    {
        if(ptnode->port == port)
        {
            ptmp->next = ptnode->next;
            //free(ptnode->netbuffer);//释放内存
            return;
        }
        ptmp = ptnode;
        ptnode = ptnode->next;
    }
}


// +EVENT:SocketDown,1,3,123
// 比如接收到OK ERROR .....——WiFi芯片发送过来的数据，我们是不进行解析的，我们只解析服务器发送过来的数据
void NetSocketDataAnalysis(unsigned char data)
{
    static unsigned char buf[256];
    static unsigned int i = 0;
    static unsigned char step = 0;
    static unsigned short conID = 0;
    static unsigned short recvLen = 0;
    switch(step)
    {
        case 0: // Find '+'
        {
            if(data != '+') return;
            step = 1;
            buf[i++] = data;
            break;
        }
        case 1: // Find "+EVENT:SocketDown,"
        {
            buf[i++] = data;
            if(strstr((char*)buf, "+EVENT:SocketDown,"))//+EVENT:SocketDown,长度为18  +EVENT:SocketDown,   +abcddff:SocketDown,
            {
                memset((char*)buf, 0, i);
                i = 0;
                step = 2;
            }
            else if(i == 18)
            {
                memset((char*)buf, 0, i);
                i= 0;
                step = 0;
            }
            break;
        }
        case 2: // Get conID
        {
            if(data == ',')
            {
                step = 3;
            }
            else
            {
                conID = conID * 10 + data - '0';
            }
            break;
        }
        case 3: // Get Receive Length
        {
            if(data == ',')
            {
                step = 4;
            }
            else
            {
                recvLen = recvLen * 10 + data - '0';
            }
            break;
        }
        case 4: // Save Socket Data
        {
            NetNode * Nettemp = NetNodeFind_PassConid(conID);//根据conID寻找节点
            if(Nettemp == NULL) break;
            Nettemp->netbuffer->Write(Nettemp->netbuffer, &data, 1);
            
            
            i++;//记录收到的真实数据的个数
            if(i==recvLen)
            {
                i = 0;
                conID = 0;
                recvLen = 0;
                step = 0;
            }
            break;
        }
    }
}
