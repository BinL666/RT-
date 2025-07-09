#ifndef __VL53L0_I2C_H
#define __VL53L0_I2C_H

#include "stm32f4xx.h"
#include "gpio.h"
#include "main.h"
#ifndef u8
#define u8 uint8_t
#endif
#ifndef u16
#define u16 uint16_t
#endif
#ifndef u32
#define u32 uint32_t
#endif

//�˿���ֲ
#define RCC_VL53L0x		RCC_AHB1Periph_GPIOB
#define PORT_VL53L0x 	GPIOB

#define GPIO_SDA 		GPIO_PIN_7
#define GPIO_SCL 		GPIO_PIN_6

// ���� SDA Ϊ���ģʽ
#define SDA_OUT()   { \
                        GPIO_InitTypeDef GPIO_InitStruct = {0}; \
                        GPIO_InitStruct.Pin = GPIO_SDA; \
                        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; \
                        GPIO_InitStruct.Pull = GPIO_NOPULL; \
                        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; \
                        HAL_GPIO_Init(PORT_VL53L0x, &GPIO_InitStruct); \
                    }

// ���� SDA Ϊ����ģʽ
#define SDA_IN()    { \
                        GPIO_InitTypeDef GPIO_InitStruct = {0}; \
                        GPIO_InitStruct.Pin = GPIO_SDA; \
                        GPIO_InitStruct.Mode = GPIO_MODE_INPUT; \
                        GPIO_InitStruct.Pull = GPIO_NOPULL; \
                        HAL_GPIO_Init(PORT_VL53L0x, &GPIO_InitStruct); \
                    }

// ��ȡ SDA ���ŵĵ�ƽ
#define SDA_GET()       HAL_GPIO_ReadPin(PORT_VL53L0x, GPIO_SDA)

// ���� SDA �����ƽ
#define SDA(x)          HAL_GPIO_WritePin(PORT_VL53L0x, GPIO_SDA, (x ? GPIO_PIN_SET : GPIO_PIN_RESET))

// ���� SCL �����ƽ
#define SCL(x)          HAL_GPIO_WritePin(PORT_VL53L0x, GPIO_SCL, (x ? GPIO_PIN_SET : GPIO_PIN_RESET))

//״̬
#define STATUS_OK       0x00
#define STATUS_FAIL     0x01

//IIC��������
void delay_us(uint32_t udelay);
void VL53L0X_i2c_init(void);
u8 VL53L0X_write_byte(u8 address,u8 index,u8 data);              //IICдһ��8λ����
u8 VL53L0X_write_word(u8 address,u8 index,u16 data);             //IICдһ��16λ����
u8 VL53L0X_write_dword(u8 address,u8 index,u32 data);            //IICдһ��32λ����
u8 VL53L0X_write_multi(u8 address, u8 index,u8 *pdata,u16 count);//IIC����д

u8 VL53L0X_read_byte(u8 address,u8 index,u8 *pdata);             //IIC��һ��8λ����
u8 VL53L0X_read_word(u8 address,u8 index,u16 *pdata);            //IIC��һ��16λ����
u8 VL53L0X_read_dword(u8 address,u8 index,u32 *pdata);           //IIC��һ��32λ����
u8 VL53L0X_read_multi(u8 address,u8 index,u8 *pdata,u16 count);  //IIC������


#endif 


