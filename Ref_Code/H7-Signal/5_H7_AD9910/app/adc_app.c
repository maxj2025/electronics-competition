#include "adc_app.h"

//__attribute__((section(".RAM_D1"), aligned(32))) uint16_t ADC_data[ADC_LEN];

//#define HIST_BINS     512
//#define BIN_WIDTH     (ADC_MAX_CODE / HIST_BINS)

//static inline float adc_to_v(uint16_t val)
//{
//    return val * ADC_VREF / (float)ADC_MAX_CODE;
//}

//volatile uint8_t adc_con_flag = 0;
//void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
//{
//	if(hadc->Instance==ADC1)
//	{
//		if(adc_con_flag==0)
//			adc_con_flag = 1;
//	}
//}	

//void adc_get_vpp(adc_waveform_info_t *info, const uint16_t *data, uint32_t len)
//{
//    if (info == NULL || data == NULL || len == 0)
//        return;

//    uint16_t vmax = 0;
//    uint16_t vmin = 65535;
//    uint64_t sum = 0;

//    for (uint32_t i = 0; i < len; i++) {
//        uint16_t v = data[i];
//        sum += v;

//        if (v > vmax)
//            vmax = v;

//        if (v < vmin)
//            vmin = v;
//    }

//    info->max_adc = vmax;
//    info->min_adc = vmin;
//    info->vmax = adc_to_v(vmax);
//    info->vmin = adc_to_v(vmin);
//    info->vpp  = info->vmax - info->vmin;
//    info->vavg = ((float)sum / len) * ADC_VREF / (float)ADC_MAX_CODE;

//}


//void adc_get_vpp_robust(adc_waveform_info_t *info, const uint16_t *data,
//                        uint32_t len, float discard_pct)
//{
//	
//    if (info == NULL || data == NULL || len == 0)
//        return;

//    if (discard_pct <= 0.0f) {
//        adc_get_vpp(info, data, len);
//        return;
//    }

//    if (discard_pct >= 50.0f)
//        discard_pct = 49.0f;

//    uint32_t hist[HIST_BINS];
//    uint32_t i;

//    for (i = 0; i < HIST_BINS; i++)
//        hist[i] = 0;

//    uint64_t sum = 0;

//    for (i = 0; i < len; i++) {
//        uint16_t v  = data[i];
//        uint32_t bin = v / BIN_WIDTH;

//        if (bin >= HIST_BINS)
//            bin = HIST_BINS - 1;

//        hist[bin]++;
//        sum += v;
//    }


//    uint32_t discard_n = (uint32_t)(len * discard_pct / 100.0f);

//    if (discard_n == 0)
//        discard_n = 1;


//    uint32_t cum = 0;
//    uint16_t lo_val = 0;

//    for (i = 0; i < HIST_BINS; i++) {
//        uint32_t prev = cum;
//        cum += hist[i];

//        if (cum >= discard_n) {
//            uint32_t offset = discard_n - prev;
//            uint32_t n      = hist[i];

//            if (n == 0)
//                n = 1;

//            lo_val = (uint16_t)(i * BIN_WIDTH + offset * BIN_WIDTH / n);
//            break;
//        }
//    }


//    cum = 0;
//    uint16_t hi_val = 65535;

//    for (i = HIST_BINS; i-- > 0; ) {
//        uint32_t prev = cum;
//        cum += hist[i];

//        if (cum >= discard_n) {
//            uint32_t offset = discard_n - prev;
//            uint32_t n      = hist[i];

//            if (n == 0)
//                n = 1;

//            uint32_t v = (i + 1) * BIN_WIDTH - offset * BIN_WIDTH / n;

//            if (v > 65535)
//                v = 65535;

//            hi_val = (uint16_t)v;
//            break;
//        }
//    }

//    info->max_adc = hi_val;
//    info->min_adc = lo_val;
//    info->vmax = adc_to_v(hi_val);
//    info->vmin = adc_to_v(lo_val);
//    info->vpp  = info->vmax - info->vmin;
//    info->vavg = ((float)sum / len) * ADC_VREF / (float)ADC_MAX_CODE;

//}


//void adc_get_vpp_auto(adc_waveform_info_t *info, const uint16_t *data,
//                      uint32_t len)
//{
//    if (info == NULL || data == NULL || len == 0)
//        return;

//    adc_waveform_info_t basic;
//    adc_waveform_info_t robust;

//    adc_get_vpp(&basic, data, len);
//    adc_get_vpp_robust(&robust, data, len, 3.0f);

//    if (basic.vpp > robust.vpp * 1.08f)
//        *info = robust;
//    else
//        *info = basic;
//		

//}



void adc_proc(void)
{
	
}
//{
//		adc_waveform_info_t info;

//		 if(adc_con_flag==1)
//		 {
//			
//			HAL_ADC_Stop_DMA(&hadc1);
//			SCB_InvalidateDCache_by_Addr((uint32_t *)&ADC_data, ADC_LEN*sizeof(uint16_t));
//			adc_get_vpp_auto(&info, ADC_data, ADC_LEN);

//			adc_con_flag = 0;
//			HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADC_data, ADC_LEN);
//		 }
//}

