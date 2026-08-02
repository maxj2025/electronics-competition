#include "scheduler.h"

uint8_t task_num = 0;
typedef struct 
{
	void (*task_func)(void);
	 uint32_t rate_ms;
	 uint32_t last_ms;
}task_t;


task_t scheduler_task[] = 
{
	{uart_proc,10,0}

};


void scheduler_Init(void)
{
	task_num = sizeof(scheduler_task)/sizeof(task_t);
}




void scheduler_run(void)
{
	uint32_t tick_temp = 0;
	for(uint8_t i= 0;i<task_num;i++)
	{
		tick_temp = uwTick;
		if(tick_temp>=scheduler_task[i].last_ms+scheduler_task[i].rate_ms)
		{
			scheduler_task[i].task_func();
			scheduler_task[i].last_ms = tick_temp;
		}
	}
}
