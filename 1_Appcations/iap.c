#include "stm32f4xx_hal.h"
#include "main.h"      /* LCD_LED_Pin / LCD_LED_GPIO_Port：跳转前关背光用 */
#include "printf.h"
#include "iap.h"
#include "FreeRTOS.h"
#include "task.h"    /* vTaskSuspendAll：跳转前挂起 FreeRTOS 调度器 */

typedef  void (*pFunction)(void);

void jump_to_application(unsigned int app_addr)
{
    pFunction JumpToApplication;   // 新固件 Reset_Handler 指针（调用前保持在寄存器里，换栈后不读旧栈）

    //这个栈指针必须指向有效的RAM地址——单片机的内部RAM取值范围是0x2000 000~ 0x2001FFFF
    if (((*(__IO uint32_t*)app_addr) & 0x2FFE0000 ) == 0x20000000)
    {
        debugprintf("0x%x IS Normal RAM Address!\r\n", app_addr);

        /* 跳转前关背光（PB1=0，灭屏）：确认要跳转后立刻黑屏，避免跳转瞬间画面冻结、
         * 看起来像页面卡死。放这里是因为 GPIOB 时钟此刻还开着（HAL_RCC_DeInit 之前），
         * 直接写引脚有效；且 ODR 电平是锁存的——后面时钟树复位后 PB1 仍保持低电平，
         * 背光持续熄灭。新固件初始化 LCD 时（dev_st7789 的 lcd_led(1)）会重新点亮背光。 */
        HAL_GPIO_WritePin(LCD_LED_GPIO_Port, LCD_LED_Pin, GPIO_PIN_RESET);


        /* ==================== 复位 bootloader 遗留状态 → "类上电"交接（工程师建议） ====================
         * 工程师核心思路：跳转前把时钟、中断、外设、调度器都恢复到"刚上电"的样子，否则 APP 一启动，
         * bootloader 残留的时钟/中断/外设状态可能和新固件初始化冲突，或旧中断在新 RTOS 跑起来前
         * 触发，导致卡死或 HardFault。
         *
         * 对照核实本 bootloader 的遗留状态：
         *   调度器 : FreeRTOS 运行中（本回调跑在 DispTask 任务栈 PSP 上）
         *   时基   : SysTick(FreeRTOS) + TIM1(HAL timebase) 两个都在跑
         *   中断   : USART1/3、DMA1、SPI1/3、I2C2、TIM1 更新 等 NVIC 全开
         *   外设   : USART1(DEBUG) USART3(WIFI/ESP32) SPI1(ST7789) SPI3(W25Q) I2C2 均已初始化
         *
         * 顺序固定：①挂起调度器 ②关中断/停时基 ③外设复位 ④时钟树复位 ⑤NVIC清挂起 ⑥VTOR ⑦MSP/跳转 */
        vTaskSuspendAll();                 /* ① 挂起 FreeRTOS 调度器，禁止任务切换 */
        __disable_irq();                   /* ② 全局关中断（保持关闭直到新固件 osKernelStart 自己开） */
        SysTick->CTRL = 0;                 /* 停 FreeRTOS 时基（SysTick） */
        SysTick->LOAD = 0;
        SysTick->VAL = 0;
        SCB->ICSR = SCB_ICSR_PENDSVCLR_Msk | SCB_ICSR_PENDSTCLR_Msk;   /* 清挂起的 PendSV/SysTick */

        /* ③ 时钟树复位：SYSCLK 切回 HSI、关 PLL/HSE、所有分频器复位。
         *    新固件 SystemClock_Config 从 HSI 起步重新配 PLL，和"刚上电"完全一致。
         *    （即便不做这步，新固件的 HAL_RCC_OscConfig 也会先把 SYSCLK 切到 HSI 再配 PLL，
         *      这步是双保险，让交接更彻底）
         *    先于外设复位做：HAL_RCC_DeInit 内部用 HAL_GetTick() 判超时，此时 TIM1(HAL时基)
         *    还活着，超时判断才有效；若先复位 TIM1 则 tick 冻结、超时永远判不中。 */
        HAL_RCC_DeInit();

        /* ④ 外设复位（FORCE_RESET 强制复位再释放 → 外设回上电初态）：
         *    杀掉残留的 TIM1(HAL timebase) 更新中断、USART 残留中断、可能还在跑的 DMA 传输。
         *    注意：本工程 HAL 时基用 TIM1 而非 SysTick，不停它的话新固件 HAL_InitTick 同抢 TIM1。 */
        __HAL_RCC_TIM1_FORCE_RESET();   __HAL_RCC_TIM1_RELEASE_RESET();     /* HAL timebase */
        __HAL_RCC_USART1_FORCE_RESET(); __HAL_RCC_USART1_RELEASE_RESET();   /* DEBUG 串口 */
        __HAL_RCC_USART3_FORCE_RESET(); __HAL_RCC_USART3_RELEASE_RESET();   /* WIFI 串口(ESP32) */
        __HAL_RCC_DMA1_FORCE_RESET();   __HAL_RCC_DMA1_RELEASE_RESET();     /* I2C2/SPI3 用的 DMA1 */
        __HAL_RCC_DMA2_FORCE_RESET();   __HAL_RCC_DMA2_RELEASE_RESET();
        __HAL_RCC_SPI1_FORCE_RESET();   __HAL_RCC_SPI1_RELEASE_RESET();     /* ST7789 屏 */
        __HAL_RCC_SPI3_FORCE_RESET();   __HAL_RCC_SPI3_RELEASE_RESET();     /* W25Q flash */
        __HAL_RCC_I2C2_FORCE_RESET();   __HAL_RCC_I2C2_RELEASE_RESET();     /* 触摸/EEPROM */

        /* ⑤ 全禁用 NVIC + 清挂起：任何外设（哪怕复位后又立刻断言）都不可能再打断 */
        for(char i=0; i<8; i++)
        {
            NVIC->ICER[i] = 0xFFFFFFFF;
            NVIC->ICPR[i] = 0xFFFFFFFF;
        }

        /* ⑥ 切向量表到新固件：跳转前显式设一次。新固件 SystemInit 也会设 VTOR=0x08064000，
         *    但这里先设好，跳转瞬间（BX 到新固件 Reset_Handler）向量表就是新固件的，
         *    杜绝"开中断前任何异常走错表"的窗口。0x08064000 满足 VTOR 的 0x200 对齐要求。 */
        SCB->VTOR = app_addr;

        /* ⑦ 取新固件 Reset_Handler 到局部变量：调用前保持在寄存器，换栈后不读旧栈 */
        JumpToApplication = (pFunction) *(__IO uint32_t*)(app_addr + 4);
        __set_MSP(*(__IO uint32_t*) app_addr);   /* MSP = 新固件栈顶 */

        /* 本 bootloader 跑 FreeRTOS，任务运行在 PSP（CONTROL.SPSEL=1）。必须先把 Thread
         * 模式切回 MSP（SPSEL=0），否则新固件 main() 会继续跑在 bootloader 残留的 PSP 任务栈上
         * ——该栈在 bootloader 的 FreeRTOS 堆里（0x20001B64 起），落在新固件 ZI 区内，
         * 新固件 scatterload 清零 ZI 时会把正在用的栈抹掉 → 一进去就崩、灯不亮。
         * 用 CMSIS 内建函数（cmsis_armcc.h 里是 ARMCC 寄存器变量汇编，自带调度屏障，
         * ARMCC V5 能编译；不要写 GNU 内联汇编，armcc 不支持 GCC 约束语法）。 */
        __set_CONTROL(__get_CONTROL() & ~0x02UL);  /* SPSEL=0：Thread 模式使用 MSP */
        __ISB();
        __enable_irq();                            //重新开启中断
        JumpToApplication();                       /* 跳转新固件 Reset_Handler */
    }
    else
    {
        debugprintf("0x%x NO Normal RAM Address!\r\n", app_addr);
    }
}