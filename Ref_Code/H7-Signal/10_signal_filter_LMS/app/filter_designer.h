#ifndef __FILTER_DESIGNER_H
#define __FILTER_DESIGNER_H
#include "bspsysteam.h"

void filter_init(void);
void df1_proc(float32_t *pSrc, float32_t *pDst, uint32_t blockSize);

#endif

