#ifndef __VL53L0X_H
#define __VL53L0X_H

#include "vl53l0x_api.h"
#include "vl53l0x_platform.h"
#include "vl53l0x_gen.h"
#include "vl53l0x_cali.h"
#include "vl53l0x_it.h"
#include "main.h"
#include "stm32f4xx.h"

// λ�������ĺ궨�壬ֱ��ʹ�����ṩ��BIT_ADDR��
#define BIT_ADDR(byte_offset,bitnum)  (volatile unsigned long*)(0x42000000 + (byte_offset * 32) + (bitnum * 4))

// ����GPIOB�Ĵ�����λ����������ַ
#define GPIOA_OCTL_OFFSET ((GPIOA_BASE + 0x14) - 0x40000000)
#define GPIOB_OCTL_OFFSET ((GPIOB_BASE + 0x14) - 0x40000000)
#define GPIOC_OCTL_OFFSET ((GPIOC_BASE + 0x14) - 0x40000000)
#define GPIOD_OCTL_OFFSET ((GPIOD_BASE + 0x14) - 0x40000000)
#define GPIOE_OCTL_OFFSET ((GPIOE_BASE + 0x14) - 0x40000000)
#define GPIOF_OCTL_OFFSET ((GPIOF_BASE + 0x14) - 0x40000000)
#define GPIOG_OCTL_OFFSET ((GPIOG_BASE + 0x14) - 0x40000000)

#define GPIOA_ISTAT_OFFSET ((GPIOA_BASE + 0x10) - 0x40000000)
#define GPIOB_ISTAT_OFFSET ((GPIOB_BASE + 0x10) - 0x40000000)
#define GPIOC_ISTAT_OFFSET ((GPIOC_BASE + 0x10) - 0x40000000)
#define GPIOD_ISTAT_OFFSET ((GPIOD_BASE + 0x10) - 0x40000000)
#define GPIOE_ISTAT_OFFSET ((GPIOE_BASE + 0x10) - 0x40000000)
#define GPIOF_ISTAT_OFFSET ((GPIOF_BASE + 0x10) - 0x40000000)
#define GPIOG_ISTAT_OFFSET ((GPIOG_BASE + 0x10) - 0x40000000)


// ��������������
#define PAin(n)     *(BIT_ADDR(GPIOA_ISTAT_OFFSET,n))   // PA����
#define PBin(n)     *(BIT_ADDR(GPIOB_ISTAT_OFFSET,n))   // PB����
#define PCin(n)     *(BIT_ADDR(GPIOC_ISTAT_OFFSET,n))   // PC����
#define PDin(n)     *(BIT_ADDR(GPIOD_ISTAT_OFFSET,n))   // PD����
#define PEin(n)     *(BIT_ADDR(GPIOE_ISTAT_OFFSET,n))   // PE����
#define PFin(n)     *(BIT_ADDR(GPIOF_ISTAT_OFFSET,n))   // PF����
#define PGin(n)     *(BIT_ADDR(GPIOG_ISTAT_OFFSET,n))   // PG����

#define PAout(n)    *(BIT_ADDR(GPIOA_OCTL_OFFSET,n))    // PA���
#define PBout(n)    *(BIT_ADDR(GPIOB_OCTL_OFFSET,n))    // PB���
#define PCout(n)    *(BIT_ADDR(GPIOC_OCTL_OFFSET,n))    // PC���
#define PDout(n)    *(BIT_ADDR(GPIOD_OCTL_OFFSET,n))    // PD���
#define PEout(n)    *(BIT_ADDR(GPIOE_OCTL_OFFSET,n))    // PE���
#define PFout(n)    *(BIT_ADDR(GPIOF_OCTL_OFFSET,n))    // PF���
#define PGout(n)    *(BIT_ADDR(GPIOG_OCTL_OFFSET,n))    // PG���

//VL53L0X默认I2C地址
#define VL53L0X_DEFAULT_ADDR 0x52

//5个传感器的I2C地址
#define VL53L0X_ADDR1 0x54
#define VL53L0X_ADDR2 0x56
#define VL53L0X_ADDR3 0x58
#define VL53L0X_ADDR4 0x5A
#define VL53L0X_ADDR5 0x5C

//5个传感器的XSHUT引脚定义
#define VL53L0X_Xshut1 PEout(6)
#define VL53L0X_Xshut2 PEout(5)
#define VL53L0X_Xshut3 PEout(4)
#define VL53L0X_Xshut4 PEout(3)
#define VL53L0X_Xshut5 PEout(2)

//ʹ2.8V IOƽģʽ
#define USE_I2C_2V8  1

//ģʽ
#define Default_Mode   0// Ĭ
#define HIGH_ACCURACY  1//߾
#define LONG_RANGE     2//
#define HIGH_SPEED     3//

//vl53l0xģʽò
typedef struct packed_2
{
	FixPoint1616_t signalLimit;    //Signalֵ 
	FixPoint1616_t sigmaLimit;     //Sigmalֵ
	uint32_t timingBudget;         //ʱ
	uint8_t preRangeVcselPeriod ;  //VCSEL
	uint8_t finalRangeVcselPeriod ;//VCSELڷΧ
	
}mode_data;


extern mode_data Mode_data[];
extern uint8_t AjustOK;

//初始化函数声明
VL53L0X_Error vl53l0x_init1(VL53L0X_Dev_t *dev);
VL53L0X_Error vl53l0x_init2(VL53L0X_Dev_t *dev);
VL53L0X_Error vl53l0x_init3(VL53L0X_Dev_t *dev);
VL53L0X_Error vl53l0x_init4(VL53L0X_Dev_t *dev);
VL53L0X_Error vl53l0x_init5(VL53L0X_Dev_t *dev);

void print_pal_error(VL53L0X_Error Status);//Ϣӡ
void mode_string(u8 mode,char *buf);//ģʽַʾ
void vl53l0x_test(void);//vl53l0x
void vl53l0x_reset(VL53L0X_Dev_t *dev);//vl53l0xλ

void vl53l0x_info(void);//ȡvl53l0x豸IDϢ
void One_measurement(u8 mode);//ȡһβ
#endif

