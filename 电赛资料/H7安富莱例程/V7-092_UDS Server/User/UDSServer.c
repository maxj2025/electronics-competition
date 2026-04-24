/*
*********************************************************************************************************
*
*	模块名称 : UDS
*	文件名称 : UDSServer.c
*	版    本 : V1.0
*	说    明 : UDS Server测试
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2024-04-29   Eric2013     1. CMSIS软包版本 V5.7.0
*                                         2. HAL库版本 V1.9.0
*                                         
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"
#include "stdbool.h"
#include "iso14229.h"
#include "uds_params.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#if UDS_TP == UDS_TP_ISOTP_C
#include "isotp-c/isotp.h"
#include "isotp-c_on_socketcan.h"
#endif

static uint8_t fn(UDSServer_t *srv, UDSServerEvent_t ev, const void *arg);
static bool serverWantsExit = false;
static UDSServer_t srv;

uint32_t AddrOffset;
__IO uint32_t AppAddr;  

static const UDSServerConfig_t cfg = {
    .fn = fn,
#if UDS_TP == UDS_TP_ISOTP_SOCKET
    .if_name = "vcan0",
#endif
    .phys_send_id = SERVER_SEND_ID,
    .phys_recv_id = SERVER_PHYS_RECV_ID,
    .func_recv_id = SERVER_FUNC_RECV_ID,
};

static struct RWDBIData {
    uint8_t d1;
    int8_t d2;
    uint16_t d3;
    int16_t d4;
} myData = {0};

/*
*********************************************************************************************************
*	处理函数
*********************************************************************************************************
*/
static void mockECUReset(enum UDSECUResetType resetType) {
    printf("Resetting ECU (type: %d)\n", resetType);
    switch (resetType) {
    case kHardReset:
		__NVIC_SystemReset();
		 printf("kHardReset\n");
		break;
    case kSoftReset:
		printf("kSoftReset\n");
        UDSServerDeInit(&srv);
        UDSServerInit(&srv, &cfg);
        break;
	
    case kEnableRapidPowerShutDown:
		printf("kEnableRapidPowerShutDown\n");
        UDSServerDeInit(&srv);
        UDSServerInit(&srv, &cfg);
        break;
	
    default:
        printf("unknown reset type %d\n", resetType);
        break;
    }
}

uint8_t sendbuf[150] = {0x88};
static uint8_t RDBI(UDSServer_t *srv, UDSRDBIArgs_t *r) 
{
    //static const uint8_t msg[] = "I'm a UDS server    ";
    switch (r->dataId) {
    case 0x1:
        return r->copy(srv, &myData.d1, sizeof(myData.d1));
    case 0x8:
        //return r->copy(srv, (void *)msg, sizeof(msg));
		return r->copy(srv, (void *)sendbuf, sizeof(sendbuf));
    default:
        return kRequestOutOfRange;
    }
	
    return kPositiveResponse;
}

static uint8_t RDBA(UDSServer_t *srv, UDSReadMemByAddrArgs_t *r) {
	r->copy(srv, (void *)r->memAddr, r->memSize);
    return kPositiveResponse;
}

static uint8_t WDBI(UDSServer_t *srv, UDSWDBIArgs_t *r) {
    switch (r->dataId) {
    case 0x1:
        if (r->len != sizeof(myData.d1)) {
            return kIncorrectMessageLengthOrInvalidFormat;
        }
        myData.d1 = r->data[0];
        break;
    default:
        return kRequestOutOfRange;
    }
    return kPositiveResponse;
}
 
/*
*********************************************************************************************************
*	函 数 名: fn
*	功能说明: 回调处理
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static uint8_t fn(UDSServer_t *srv, UDSServerEvent_t ev, const void *arg) {
    switch (ev) {
	/* 0x10服务 */
    case UDS_SRV_EVT_DiagSessCtrl: { 
        UDSDiagSessCtrlArgs_t *r = (UDSDiagSessCtrlArgs_t *)arg;
        switch (r->type) {
		case kSafetySystemDiagnostic:
        case kDefaultSession:
            return kPositiveResponse;
        case kProgrammingSession:
        case kExtendedDiagnostic:
            if (srv->securityLevel > 0) {
                return kPositiveResponse;
            } else {
                return kSecurityAccessDenied;
            }
            break;
        default:
            return kSubFunctionNotSupported;
        }
    }
	
	/* 0x11服务 */
    case UDS_SRV_EVT_EcuReset: { 
			UDSECUResetArgs_t *r = (UDSECUResetArgs_t *)arg;
			printf("got ECUReset request of type %x\n", r->type);
			switch (r->type) {
				case kHardReset:
				case kSoftReset:
				case kEnableRapidPowerShutDown:
				return kPositiveResponse;
				break;
				default:
				return kSubFunctionNotSupported;
			}
			break;
		}
	
	/* 0x22服务 */	
    case UDS_SRV_EVT_ReadDataByIdent: 
        return RDBI(srv, (UDSRDBIArgs_t *)arg);

	/* 0x23服务 */
    case UDS_SRV_EVT_ReadMemByAddr: 
        return RDBA(srv, (UDSReadMemByAddrArgs_t *)arg);
	
	/* 0x27服务 */
    case UDS_SRV_EVT_SecAccessRequestSeed: {
        const uint8_t seed[] = {0x11, 0x22, 0x33, 0x44};
        UDSSecAccessRequestSeedArgs_t *r = (UDSSecAccessRequestSeedArgs_t *)arg;
        return r->copySeed(srv, seed, sizeof(seed));
    }
	
	/* 0x27服务 */
    case UDS_SRV_EVT_SecAccessValidateKey:
        return kPositiveResponse;
	
	/* 0x28服务 */
	case UDS_SRV_EVT_CommCtrl:
		return kPositiveResponse;
	
	/* 0x2E服务 */
    case UDS_SRV_EVT_WriteDataByIdent: 
        return WDBI(srv, (UDSWDBIArgs_t *)arg);
	
	/* 0x31服务 */
    case UDS_SRV_EVT_RoutineCtrl: {    
        UDSRoutineCtrlArgs_t *r = (UDSRoutineCtrlArgs_t *)arg;
        if (RID_TERMINATE_PROCESS == r->id) {
            serverWantsExit = true;
            return kPositiveResponse;
        } else {
            return kRequestOutOfRange;
        }
        break;
    }
	
	/* 0x34服务 */
	case UDS_SRV_EVT_RequestDownload:{ 
			printf("UDS_SRV_EVT_RequestDownload\r\n");
			UDSRequestDownloadArgs_t *r = (UDSRequestDownloadArgs_t *)arg;
			AppAddr = (uint32_t)(r->addr);
			memset((void *)AppAddr, 0, r->size+100);
			AddrOffset = 0;
			return kPositiveResponse;
		}
	
	/* 0x36服务 */
	case UDS_SRV_EVT_TransferData:{
			UDSTransferDataArgs_t *r = (UDSTransferDataArgs_t *)arg;
			memcpy((char *)(AppAddr+AddrOffset), &(r->data[0]), r->len);
			AddrOffset = AddrOffset + r->len;
			printf("UDS_SRV_EVT_TransferData = %d\r\n", AddrOffset);
			return kPositiveResponse;
		}
	
	/* 0x37服务 */
	case UDS_SRV_EVT_RequestTransferExit:{
			printf("UDS_SRV_EVT_RequestTransferExit\r\n");
			//UDSRequestTransferExitArgs_t *r = (UDSRequestTransferExitArgs_t *)arg;
			return kPositiveResponse;
		}
	
	/* 会话溢出 */
    case UDS_SRV_EVT_SessionTimeout:
        printf("server session timed out!\n");
        UDSServerDeInit(srv);
        UDSServerInit(srv, &cfg);
        break;
	
    case UDS_SRV_EVT_DoScheduledReset:
        printf("powering down!\n");
        mockECUReset(*((enum UDSECUResetType *)arg));
        break;
	
    default:
        printf("Unhandled event: %d\n", ev);
        return kServiceNotSupported;
    }
    return kGeneralProgrammingFailure;
}

/*
*********************************************************************************************************
*	函 数 名: UDSTest
*	功能说明: --
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void UDSTest(void) {
	
	bsp_StartAutoTimer(0, 500);	          	/* 启动1个500ms的自动重装的定时器 */
	
	memset(sendbuf, 0x88, sizeof(sendbuf));
	
	bsp_DelayMS(1000);
    UDSServerInit(&srv, &cfg);
	
    printf("server up, polling . . .\n");
	
    while (1) 
	{
		UDSServerPoll(&srv);
	#if UDS_TP == UDS_TP_ISOTP_C
		SocketCANRecv((UDSTpIsoTpC_t *)srv.tp, cfg.phys_recv_id);
	#endif	
    
		/* 判断定时器超时时间 */
		if (bsp_CheckTimer(0))	
		{            
			/* 每隔500ms 进来一次 */  
			bsp_LedToggle(2);
		}
    }
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
