#ifndef __WT61C_BSP_H__
#define __WT61C_BSP_H__

#include "stm32f4xx.h"
#include <stdio.h>
#include "usart2.h"
#include "wit_c_sdk.h"
#include <string.h>

//static void CmdProcess(void);
static void AutoScanSensor(void);
static void SensorUartSend(uint8_t *p_data, uint32_t uiSize);
static void SensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum);
static void Delayms(uint16_t ucMs);
void WT61C_Init(void);
void getWT61C(void);

extern float fAcc[3], fGyro[3], fAngle[3];

#endif
