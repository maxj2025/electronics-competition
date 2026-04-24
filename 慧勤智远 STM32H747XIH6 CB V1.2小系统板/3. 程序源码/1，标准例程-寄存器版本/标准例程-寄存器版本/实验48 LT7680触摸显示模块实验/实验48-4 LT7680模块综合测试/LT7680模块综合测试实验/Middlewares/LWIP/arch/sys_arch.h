/**
 ****************************************************************************************************
 * @file        sys_arch.h
 * @version     V1.0
 * @brief       lwip-系统层接口驱动 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    GD32H759IMT6小系统板
 *
 ****************************************************************************************************
 */
 
#ifndef __ARCH_SYS_ARCH_H__
#define __ARCH_SYS_ARCH_H__ 
#include "arch/cc.h"
#include "os.h"


#ifdef SYS_ARCH_GLOBALS
#define SYS_ARCH_EXT
#else
#define SYS_ARCH_EXT extern
#endif
 
#define MAX_QUEUES              10              /* 消息邮箱的数量 */
#define MAX_QUEUE_ENTRIES       20              /* 每个消息邮箱的大小 */

/* LWIP消息邮箱结构体 */
typedef struct {
    OS_EVENT*   pQ;                             /* UCOS中指向事件控制块的指针 */
    void*       pvQEntries[MAX_QUEUE_ENTRIES];  /* 消息队列 MAX_QUEUE_ENTRIES消息队列中最多消息数 */
} TQ_DESCR, *PQ_DESCR;


typedef OS_EVENT *sys_sem_t;    /* LWIP使用的信号量 */
typedef OS_EVENT *sys_mutex_t;  /* LWIP使用的互斥信号量 */
typedef PQ_DESCR sys_mbox_t;    /* LWIP使用的消息邮箱,其实就是UCOS中的消息队列 */
typedef INT8U sys_thread_t;     /* 线程ID,也就是任务优先级 */
typedef INT8U sys_prot_t;

#endif 






























