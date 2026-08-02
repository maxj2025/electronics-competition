#include "sample_app_task_scheduler.h"

/************************** 全局变量定义 **************************/
TimerData_TypeDef TimerData = {0};  // 定时器计数初始化
uint16_t wAppLastTimerTicks = 0;    // 上次Tick值初始化

/************************** 核心调度函数实现 **************************/
void SampleAppTaskScheduler(void)
{
    uint16_t wSecondsToAdd;
    uint16_t wNumTicks = 0 ;

    // 获取自上次调度以来流逝的Tick数（1ms为单位）
    wNumTicks = ST_GetElapasedTime(wAppLastTimerTicks);
    // 更新上次Tick值（16位溢出处理）
    wAppLastTimerTicks += wNumTicks;
    wAppLastTimerTicks &= 0xffffu;

    // 有流逝的Tick才执行任务
    if (wNumTicks)
    {
        // 1ms周期任务处理
        if (1U <= wNumTicks)
        {
            TimerData.w1msCount += wNumTicks;
            SampleAppTask1ms(wNumTicks);

            // 10ms周期任务触发（累计1ms Tick达到阈值）
            if (NUM_1MS_COUNTS_FOR_10MS <= TimerData.w1msCount)
            {
                wNumTicks = TimerData.w1msCount;
                // 更新10ms计数器（取整）
                TimerData.w10msCount += (TimerData.w1msCount / NUM_1MS_COUNTS_FOR_10MS);
                // 重置1ms计数器（保留余数，避免累计误差）
                TimerData.w1msCount %= NUM_1MS_COUNTS_FOR_10MS;
                SampleAppTask10ms(wNumTicks);

                // 100ms周期任务触发（累计10ms Tick达到阈值）
                if (NUM_10MS_COUNTS_FOR_100MS <= TimerData.w10msCount)
                {
                    // 更新100ms计数器（取整）
                    TimerData.w100msCount += (TimerData.w10msCount / NUM_10MS_COUNTS_FOR_100MS);
                    // 重置10ms计数器（保留余数）
                    TimerData.w10msCount %= NUM_10MS_COUNTS_FOR_100MS;

                    SampleAppTask100ms();

                    // 1秒周期任务触发（累计100ms Tick达到阈值）
                    if (NUM_100MS_COUNTS_FOR_1SEC <= TimerData.w100msCount)
                    {
                        wSecondsToAdd = (TimerData.w100msCount / NUM_100MS_COUNTS_FOR_1SEC);
                        // 重置100ms计数器（保留余数）
                        TimerData.w100msCount %= NUM_100MS_COUNTS_FOR_1SEC;
                        TimerData.w1secCount += wSecondsToAdd;

                        SampleAppTask1sec();

                        // 1分钟周期任务触发（累计1秒 Tick达到阈值）
                        if (NUM_1SEC_COUNTS_FOR_1MIN <= TimerData.w1secCount)
                        {
                            // 更新1分钟计数器（取整）
                            TimerData.w1minCount += (TimerData.w1secCount / NUM_1SEC_COUNTS_FOR_1MIN);
                            // 重置1秒计数器（保留余数）
                            TimerData.w1secCount %= NUM_1SEC_COUNTS_FOR_1MIN;

                            SampleAppTask1min();
                        }
                    }
                }
            }
        }
    }
}

/************************** 辅助函数实现 **************************/
uint16_t ST_GetElapasedTime(uint16_t wLastTick)
{
    uint16_t ticks = ST_GetTimerTick();
    uint16_t wElapsedTime;

    // 处理16位Tick溢出情况
    if (ticks >= wLastTick)
    {
        wElapsedTime = ticks - wLastTick;
    }
    else
    {
        /* 计数器溢出：例如从65535跳回0，计算实际流逝值 */
        wElapsedTime = ticks + (65536UL - wLastTick);
    }

    return wElapsedTime;
}

/************************** 空实现（移植时需替换为实际逻辑） **************************/
__attribute__((weak)) uint16_t ST_GetTimerTick(void)
{
    // 【移植点1】替换为硬件定时器的1ms Tick读取函数
    // 示例：return SysTick->VAL / (SystemCoreClock / 1000);
    return 0;
}

__attribute__((weak)) void SampleAppTask1ms(uint16_t wNumTicks)
{
    // 【移植点2】实现1ms周期任务（如按键扫描、PWM刷新等）
    (void)wNumTicks; // 避免未使用参数警告
}

__attribute__((weak)) void SampleAppTask10ms(uint16_t wNumTicks)
{
    // 【移植点3】实现10ms周期任务（如状态机处理、数据采集等）
    (void)wNumTicks;
}

__attribute__((weak)) void SampleAppTask100ms(void)
{
    // 【移植点4】实现100ms周期任务（如通信超时检测、LED闪烁等）
}

__attribute__((weak)) void SampleAppTask1sec(void)
{
    // 【移植点5】实现1秒周期任务（如电量检测、参数保存等）
}

__attribute__((weak)) void SampleAppTask1min(void)
{
    // 【移植点6】实现1分钟周期任务（如日志打印、故障统计等）
}