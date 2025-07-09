#ifndef __VL53L0X_GEN_H
#define __VL53L0X_GEN_H

#include "bsp_vl53l0x.h"

/* 定义vu16类型 */
#ifndef vu16
#define vu16 uint16_t
#endif

extern VL53L0X_RangingMeasurementData_t vl53l0x_data1;
extern VL53L0X_RangingMeasurementData_t vl53l0x_data2;
extern VL53L0X_RangingMeasurementData_t vl53l0x_data3;
extern VL53L0X_RangingMeasurementData_t vl53l0x_data4;
extern VL53L0X_RangingMeasurementData_t vl53l0x_data5;

extern uint16_t Distance_data[5];

VL53L0X_Error vl53l0x_set_mode(VL53L0X_Dev_t *dev,u8 mode);
void vl53l0x_general_test(VL53L0X_Dev_t *dev);

//普通测量模式
void vl53l0x_general_start(VL53L0X_Dev_t *dev,u8 mode);
#endif


