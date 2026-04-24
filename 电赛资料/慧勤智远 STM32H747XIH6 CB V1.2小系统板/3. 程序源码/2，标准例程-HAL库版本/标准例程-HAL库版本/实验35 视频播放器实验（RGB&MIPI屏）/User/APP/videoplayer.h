/**
 ****************************************************************************************************
 * @file        videoplayer.h
 * @version     V1.0
 * @brief       视频播放器 应用代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    慧勤智远 STM32开发板
 *
 ****************************************************************************************************
 */
 
#ifndef __VIDEOPLAYER_H
#define __VIDEOPLAYER_H

#include "./MJPEG/avi.h"
#include "./SYSTEM/sys/sys.h"
#include "./FATFS/source/ff.h"


#define AVI_AUDIO_BUF_SIZE (1024 * 5)   /* 定义avi解码时,音频buf大小 */
#define AVI_VIDEO_BUF_SIZE (1024 * 260) /* 定义avi解码时,视频buf大小.一般等于AVI_MAX_FRAME_SIZE的大小 */

/******************************************************************************************/

/* 函数声明 */
uint16_t video_get_tnum(char *path);                                 /* 得到path路径下，目标文件的总数 */
void video_bmsg_show(uint8_t *name, uint16_t index, uint16_t total); /* 显示视频基本信息 */
void video_info_show(AVI_INFO *aviinfo);                             /* 显示当前视频文件的相关信息 */
void video_time_show(FIL *favi, AVI_INFO *aviinfo);                  /* 显示当前播放时间 */
uint8_t video_play_mjpeg(uint8_t *pname);                            /* 播放一个mjpeg文件 */
void video_play(void);                                               /* 播放视频 */

#endif




