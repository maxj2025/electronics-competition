#ifndef SAMPLE_APP_TASK_SCHEDULER_H
#define SAMPLE_APP_TASK_SCHEDULER_H

#include <stdint.h>

/************************** 配置宏定义（移植时需根据硬件调整） **************************/
// 10ms对应的1ms Tick数（默认10ms=10个1ms Tick）
#define NUM_1MS_COUNTS_FOR_10MS    (10U)
// 100ms对应的10ms Tick数（默认100ms=10个10ms Tick）
#define NUM_10MS_COUNTS_FOR_100MS  (10U)
// 1秒对应的100ms Tick数（默认1s=10个100ms Tick）
#define NUM_100MS_COUNTS_FOR_1SEC  (10U)
// 1分钟对应的1秒Tick数（默认1min=60个1s Tick）
#define NUM_1SEC_COUNTS_FOR_1MIN   (60U)

/************************** 定时器数据结构定义 **************************/
typedef struct
{
    uint16_t w1msCount;    // 1ms Tick计数器
    uint16_t w10msCount;   // 10ms Tick计数器
    uint16_t w100msCount;  // 100ms Tick计数器
    uint16_t w1secCount;   // 1秒 Tick计数器
    uint16_t w1minCount;   // 1分钟 Tick计数器
} TimerData_TypeDef;

/************************** 全局变量声明（外部可引用） **************************/
extern TimerData_TypeDef TimerData;       // 定时器计数全局变量
extern uint16_t wAppLastTimerTicks;       // 上一次调度的Tick值

/************************** 函数声明 **************************/
/**
 * @brief 获取系统当前Tick值（移植时需实现）
 * @note 需返回1ms为单位的系统Tick，范围0~65535（16位溢出）
 * @return 当前Tick值（uint16_t）
 */
uint16_t ST_GetTimerTick(void);

/**
 * @brief 计算自上次调用以来的流逝Tick数
 * @param wLastTick 上次记录的Tick值
 * @return 流逝的Tick数（uint16_t）
 */
uint16_t ST_GetElapasedTime(uint16_t wLastTick);

/**
 * @brief 1ms周期任务函数（移植时需实现具体逻辑）
 * @param wNumTicks 本次调度的累计1ms Tick数
 */
void SampleAppTask1ms(uint16_t wNumTicks);

/**
 * @brief 10ms周期任务函数（移植时需实现具体逻辑）
 * @param wNumTicks 本次调度的累计1ms Tick数（用于精准补偿）
 */
void SampleAppTask10ms(uint16_t wNumTicks);

/**
 * @brief 100ms周期任务函数（移植时需实现具体逻辑）
 */
void SampleAppTask100ms(void);

/**
 * @brief 1秒周期任务函数（移植时需实现具体逻辑）
 */
void SampleAppTask1sec(void);

/**
 * @brief 1分钟周期任务函数（移植时需实现具体逻辑）
 */
void SampleAppTask1min(void);

/**
 * @brief 主任务调度器（核心函数，需在主循环/定时器中断中调用）
 */
void SampleAppTaskScheduler(void);

#endif /* SAMPLE_APP_TASK_SCHEDULER_H */