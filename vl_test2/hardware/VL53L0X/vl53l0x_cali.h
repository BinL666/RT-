#ifndef __VL53L0X_CAIL_H
#define __VL53L0X_CAIL_H

#include "bsp_vl53l0x.h"
#include <rtdbg.h>

//vl53l0x������У׼��Ϣ�ṹ�嶨��
typedef struct packed_1
{
	uint8_t  adjustok;                    //У׼�ɹ���־��0XAA����У׼;������δУ׼
	uint8_t  isApertureSpads;             //ApertureSpadsֵ
	uint8_t  VhvSettings;                 //VhvSettingsֵ
	uint8_t  PhaseCal;                    //PhaseCalֵ
	uint32_t XTalkCalDistance;            //XTalkCalDistanceֵ
	uint32_t XTalkCompensationRateMegaCps;//XTalkCompensationRateMegaCpsֵ
	uint32_t CalDistanceMilliMeter;       //CalDistanceMilliMeterֵ
	int32_t  OffsetMicroMeter;            //OffsetMicroMeterֵ
	uint32_t refSpadCount;                //refSpadCountֵ
	
}_vl53l0x_adjust;

extern _vl53l0x_adjust Vl53l0x_data;

void vl53l0x_calibration_test(VL53L0X_Dev_t *dev);

#endif
