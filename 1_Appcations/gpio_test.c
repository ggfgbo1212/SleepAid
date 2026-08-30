#include "dev_gpio.h"
#include "dev_uart.h"
#include "dev_iic.h"
#include "dev_spi.h"
#include "dev_w25qx.h"
#include <string.h>
#include "printf.h"
#include "errno.h"
#include "drv_flash.h"
#include "ring_buffer.h"
#include "dev_wifi.h"
#include "dev_st7789.h"
#include "xpt2046.h"

#include "MQTTFreeRTOS.h"
#include "MQTTClient.h"

#include "mqtt_base.h"
#include "mqtt_ota_ali.h"

/* ==================== LVGL ==================== */
#include "lvgl.h"
#include "gui_guider.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "lvgl_action.h"   /* guider_ui 全局对象 + 控件事件绑定（事件回调在 action 层） */
#include "tickscreen.h"    /* 每个页面一个 tick 循环函数（ui_tick 分发） */

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "iap.h"

WiFiBtDevice * WIFIBTDev = NULL;

/* MQTT 任务管理（避免硬删持锁任务导致 mqtt_mutex 永久锁死）：
 * mqtt_thread      存放任务句柄，ThreadStart 每次创建时覆盖写入；
 * mqtt_alive       mqtt 任务存活标志：任务启动置1、自杀前清0，供 wifi 任务判断旧任务是否退出；
 * mqtt_quit        WiFi 掉线时 wifi 任务置1，mqtt 任务在 MQTTBase_Yield 返回（已解锁）后检查并自杀。 */
static Thread mqtt_thread;
static volatile int mqtt_alive = 0;
static volatile int mqtt_quit = 0;

/* ==================== LVGL 界面 ==================== */

/* guider_ui 全局对象定义在 6_Middlewares/lvgl/action/lvgl_action.c，经 lvgl_action.h extern 使用 */

static TaskHandle_t gDispTaskHandle = NULL;
static TaskHandle_t gWifiTaskHandle = NULL;

/* LVGL 节拍：FreeRTOS 软件定时器每 1ms 回调一次（configTICK_RATE_HZ = 1000） */
static void gui_timer_cb(TimerHandle_t handle)
{
    (void)handle;
    lv_tick_inc(1);
}

/* lv_timer_handler() 返回值的上下限：用它做动态休眠，减少空转（参考 ESP-Epoch lvgl_port_task） */
#define LVGL_TASK_MIN_DELAY_MS   5
#define LVGL_TASK_MAX_DELAY_MS   100

/* 显示设备任务：初始化 LVGL + ST7789 驱动，周期调用 ui_tick + lv_timer_handler() 驱动渲染 */
static void DispTask(void *argument)
{
    (void)argument;

    lv_init();               /* LVGL 核心初始化 */
    lv_port_disp_init();     /* 注册显示驱动（ST7789 rotation 0 竖屏 240×320，单缓冲区） */
    lv_port_indev_init();    /* 注册触摸输入设备（XPT2046 软件 SPI） */

    tickscreen_init();   /* 创建页间信号量（WiFi 连接信号量等） */

    /* 上电默认显示 SettingsPage4Info 信息页 */
    init_scr_del_flag(&guider_ui);                       /* 初始化各屏删除标志（setup_ui 里的那步，别漏） */
    if(NULL == guider_ui.SettingsPage4Info)
        setup_scr_SettingsPage4Info(&guider_ui);
    lv_scr_load(guider_ui.SettingsPage4Info);

    /* 信息页两个状态 LED 初始化为红色（WiFi 未连接 / 云平台未连接） */
    lv_led_set_color(guider_ui.SettingsPage4Info_wifi_status_led, lv_palette_main(LV_PALETTE_RED));
    lv_led_set_color(guider_ui.SettingsPage4Info_aliyun_status_led, lv_palette_main(LV_PALETTE_RED));

    /* 各屏事件在对应 setup_scr_SettingsPage4Xxx() 末尾绑定（见 generated/ 下文件），此处无需集中绑定 */

    /* 软件定时器驱动 LVGL 节拍（1ms） */
    TimerHandle_t guiTimer = xTimerCreate("LVGL Tick", pdMS_TO_TICKS(1), pdTRUE, NULL, gui_timer_cb);
    if(NULL != guiTimer)
    {
        xTimerStart(guiTimer, 0);
    }

    uint32_t task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
    while(1)
    {
        ui_tick();               /* 当前页面周期逻辑（每页一个 tick 函数） */

        task_delay_ms = lv_timer_handler();  /* LVGL 渲染与事件处理，返回下次需处理的时间 */

        if(task_delay_ms > LVGL_TASK_MAX_DELAY_MS)
            task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
        else if(task_delay_ms < LVGL_TASK_MIN_DELAY_MS)
            task_delay_ms = LVGL_TASK_MIN_DELAY_MS;

        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
}

static void mqtt_task(void *arg)
{
    (void)arg;
    /* connect to m2m.eclipse.org, subscribe to a topic, send and receive messages regularly every 1 sec */
    int count = 0;
    int rc = 0;

    mqtt_alive = 1;   /* 登记存活：wifi 任务据此知道本任务在跑，重连前要等它退出 */

    //1.MQTT 连接服务器初始化
    MQTTBase_Init();
    //2.MQTT 连接阿里云平台
    rc = MQTTBaseConnect();
    if(rc == 0)//入云成功
    {
        debugprintf("aliyun connect success\r\n");
        /* 发送"入云成功"信号量 → Info 页 tick 函数里 aliyun_status_led 变绿 */
        xSemaphoreGive(aliyun_connected_sem);
    }
    
    //初始化阿里云OTA相关topic
    MQTTOTA_TopicInit();
    MQTTOTA_InformVersion(1, "0.0.0", DeviceName);//上报当前版本号
    
    MQTTOTA_Upgrade();//查询服务器是否有ota升级信息
    
    MQTTOTA_Subdownload_reply();// 设备端订阅服务器下发bin文件分片
    
    MQTTOTA_SubFirmwareReply();// 设备端订阅服务器返回升级信息对应的应答topic
    
    //MQTTOTA_ImportProgress();//上报下载进度
    HAL_Delay(1000);
    printf("ready to get data\r\n");
    //MQTTOTA_GetFirmwareBin(1, 1024, 0);
    

    
	while (++count)
	{
        rc = MQTTBase_Yield(1000);
        if(rc != 0)//断开云平台连接了
        {
            /* yield 出错继续循环；若 WiFi 掉线，下面的 mqtt_quit 检查会触发自杀 */
        }

        //WiFi 掉线请求退出：此刻 MQTTBase_Yield 已返回并解锁，安全自杀
        //（不能在外面硬删本任务——若删时正持有 mqtt_mutex，互斥锁会永久锁死）
        if(mqtt_quit)
        {
            debugprintf("mqtt task self-delete\r\n");
            mqtt_quit = 0;              /* 复位退出标志，避免下次误判 */
            mqtt_alive = 0;             /* 清除存活标志，wifi 任务据此知道旧任务已退出 */
            vTaskDelete(NULL);          /* 自杀 */
        }

        vTaskDelay(1);
	}
}

/* WiFi 自动连接任务：负责 WiFi 连接/掉线自动重连（任务内容待填写） */
static void wifi_auto_connect_task(void *arg)
{
    (void)arg;
    /* TODO: WiFi 自动连接逻辑，先留空 */
    WIFIBTDev = GetWIFIBTDevice();
    if(WIFIBTDev != NULL) printf("找到wifi设备\r\n");
    else  vTaskDelete(NULL);
    
    WIFIBTDev->Init(WIFIBTDev);
    while(1)
    {
        debugprintf("wifi_auto_connect_task running\r\n");

        if(0 == WIFIBTDev->dev_status)//wifi未连接状态
        {
            if(ESUCCESS == WIFIBTDev->WIFIConnect(WIFIBTDev, "man2", "12345678"))//连接WiFi成功
            {
                debugprintf("wifi connect success\r\n");
                /* 发送"WiFi 已连接"信号量 → Info 页 tick 函数里 wifi_status_led 变绿 */
                xSemaphoreGive(wifi_connected_sem);

                //MQTT连接：先等旧 MQTT 任务退出（掉线时请求自杀，Yield 一轮 1s 内必退出），
                //避免新旧两个任务同时操作同一个 client/sendbuf/readbuf
                if(mqtt_alive)
                {
                    debugprintf("wait old mqtt task exit\r\n");
                    uint8_t wait_cnt = 15;   /* 最多等 ~1.5s */
                    while(wait_cnt-- && mqtt_alive)
                        vTaskDelay(pdMS_TO_TICKS(100));
                }
                if(!mqtt_alive)
                {
                    mqtt_quit = 0;   /* 新任务从干净状态开始 */
                    int rc = ThreadStart(&mqtt_thread, mqtt_task, NULL);
                    if(rc != pdPASS)    debugprintf("mqtt task creat error\r\n");
                }
                else
                {
                    debugprintf("old mqtt task still alive, skip restart\r\n");
                }
            }
        }
        else if(1 == WIFIBTDev->dev_status)//wifi已连接状态
        {
            WIFIBTDev->WIFIStaStatus(WIFIBTDev, 200);//轮询更新wifi连接状态
            
            if(0 == WIFIBTDev->dev_status)//突然断开了
            {
                mqtt_quit = 1;   /* 请求 MQTT 任务在 Yield 返回（已解锁）后自杀；不能硬删持锁任务 */
                /* 发送"WiFi 断开"信号量 → Info 页 tick 里 wifi/aliyun 两个状态 LED 变红 */
                xSemaphoreGive(wifi_disconnected_sem);
            }
            
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_test()
{
    GPIO_DevRegis();//全局GPIO设备注册
    UART_DevRegis();//全局串口设备注册
    IIC_DevRegis();//全局IIC设备注册
    SPI_DevRegis();//全局SPI设备注册



//    struct GPIODev* GPIODev = GPIODev_Find("LED1");
//    if(GPIODev != NULL)
//    {
//        GPIODev->Init(GPIODev);
//        GPIODev->Write(GPIODev, 1);
//    }
//    
//    struct GPIODev* GPIODev1 = GPIODev_Find("LED2");
//    if(GPIODev1 != NULL)
//    {
//        GPIODev1->Init(GPIODev1);
//        GPIODev1->Write(GPIODev1, 1);
//    }
//    
//    struct GPIODev* GPIODev2 = GPIODev_Find("LED3");
//    if(GPIODev2 != NULL)
//    {
//        GPIODev2->Init(GPIODev2);
//        GPIODev2->Write(GPIODev2, 1);
//    }
//     debugprintf("ready\r\n");
//     jump_to_application(0x08064000);
//     while(1)
//     {
//         
//     }






    /* 创建 LVGL 显示任务（软件定时器驱动节拍，任务内做 lv_init + lv_port_disp_init） */
    if(pdPASS != xTaskCreate(DispTask, "DisplayTask", 1024, NULL, 4, &gDispTaskHandle))
    {
        debugprintf("DisplayTask create failed\r\n");
    }

    /* 创建 WiFi 自动连接任务（骨架，任务内容待填写） */
    if(pdPASS != xTaskCreate(wifi_auto_connect_task, "WifiAutoConnect", 2048, NULL, 2, &gWifiTaskHandle))
    {
        debugprintf("WifiAutoConnect task create failed\r\n");
    }

}
