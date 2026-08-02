#ifndef __BLUE_APP_H
#define __BLUE_APP_H
#include "bspsysteam.h"
void blue_Init(void);
void blue_disc(void);
void blue_reset(void);
void blue_send_data(uint16_t *data,uint16_t len);
void blue_send_str(char *str);
#endif

