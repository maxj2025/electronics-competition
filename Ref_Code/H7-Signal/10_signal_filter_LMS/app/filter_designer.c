#include "filter_designer.h"

////基于2阶节直接1型实例化结构体 
//arm_biquad_casd_df1_inst_f32 S;

////二阶节个数
//uint8_t numStages = 5;


//系数,中间多了有一列1，最后两个符合是反的




//float32_t pState[10] = {0.0f};

//void filter_init(void)
//{
//    for(int i = 0; i < 20; i++)
//    {
//        pState[i] = 0.0f;
//    }
//    arm_biquad_cascade_df1_init_f32(&S, numStages, pCoeffs, pState);
//}

void df1_proc(float32_t *pSrc, float32_t *pDst, uint32_t blockSize)
{
    arm_biquad_cascade_df1_f32(&S, pSrc, pDst, blockSize);
}


