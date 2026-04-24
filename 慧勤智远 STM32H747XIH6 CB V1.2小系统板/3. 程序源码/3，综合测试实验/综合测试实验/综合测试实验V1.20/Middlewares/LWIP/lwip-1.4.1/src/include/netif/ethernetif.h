/**
 ****************************************************************************************************
 * @file        ethernetif.h
 * @version     V1.0
 * @brief       lwip-网络接口驱动 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    GD32H759IMT6小系统板
 *
 ****************************************************************************************************
 */
 
#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__
#include "lwip/err.h"
#include "lwip/netif.h"


/* 网卡的名字 */
#define IFNAME0 'e'
#define IFNAME1 'n'


err_t ethernetif_init(struct netif *netif);
err_t ethernetif_input(struct netif *netif);
#endif
