#ifndef _PROCESSING_H_
#define _PROCESSING_H_
 
#include "usart.h"
#include "main.h"
#include "tim.h"
#include <stdio.h>
#include <string.h>

#include "stm32f4xx.h"
#include "wt61c_bsp.h"
#include "bsp_ws2812.h"
#include "bsp_VL53L0X.h"
#include "syn6288.h"
#include "syn6288_uart.h"

extern VL53L0X_Dev_t vl53l0x_dev1;
extern VL53L0X_Dev_t vl53l0x_dev2;
extern VL53L0X_Dev_t vl53l0x_dev3;
extern VL53L0X_Dev_t vl53l0x_dev4;
extern VL53L0X_Dev_t vl53l0x_dev5;

#define PACK_HEAD2 0x33   // °üÍ·
#define PACK_TAIL2 0x66   // °üÎ²
#define PACK_SIZE2 3      // ×Ü°ü³¤¶È

extern uint8_t bluetooth_data[PACK_SIZE2]; // ½ÓÊÕ»º³åÇø
extern uint8_t data_index;            // ½ÓÊÕÊý¾ÝË÷Òý
extern uint8_t valid_data;            // ´æ´¢½âÎö³öµÄÓÐÐ§Êý¾Ý

// ï¿½ï¿½ï¿½ï¿½5ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ä²ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ý½á¹¹
extern VL53L0X_RangingMeasurementData_t vl53l0x_data1;
extern VL53L0X_RangingMeasurementData_t vl53l0x_data2;
extern VL53L0X_RangingMeasurementData_t vl53l0x_data3;
extern VL53L0X_RangingMeasurementData_t vl53l0x_data4;
extern VL53L0X_RangingMeasurementData_t vl53l0x_data5;

// ï¿½ï¿½ï¿½ï¿½ï¿½ð¶¯¿ï¿½ï¿½ï¿½ï¿½ï¿½Ø³ï¿½ï¿½ï¿½
#define MIN_DISTANCE 100   // ï¿½ï¿½Ð¡ï¿½ï¿½ï¿½ë£¬ï¿½ï¿½Î»mmï¿½ï¿½Ð¡ï¿½Ú´Ë¾ï¿½ï¿½ï¿½Ê±ï¿½ï¿½ï¿½ï¿½Ç¿
#define MAX_DISTANCE 2000  // ï¿½ï¿½ï¿½ï¿½ï¿½ë£¬ï¿½ï¿½Î»mmï¿½ï¿½ï¿½ï¿½ï¿½Ú´Ë¾ï¿½ï¿½ï¿½Ê±ï¿½ï¿½ï¿½ï¿½
#define MAX_VIBRATION 100  // ï¿½ï¿½ï¿½ï¿½ï¿½Ç¿ï¿½È£ï¿½Õ¼ï¿½Õ±È£ï¿½
#define MIN_VIBRATION 0    // ï¿½ï¿½Ð¡ï¿½ï¿½Ç¿ï¿½È£ï¿½Õ¼ï¿½Õ±È£ï¿½
#define VIBRATION_DURATION 2000  // ï¿½ð¶¯³ï¿½ï¿½ï¿½Ê±ï¿½ä£¬ï¿½ï¿½Î»ms

typedef struct {
    uint32_t start_time;    // ï¿½ð¶¯¿ï¿½Ê¼Ê±ï¿½ï¿½
    uint32_t duration;      // ï¿½ð¶¯³ï¿½ï¿½ï¿½Ê±ï¿½ï¿½
    uint8_t is_vibrating;   // ï¿½Ç·ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
    uint16_t intensity;     // ï¿½ï¿½Ç¿ï¿½ï¿½
    TIM_HandleTypeDef *htim;// ï¿½ï¿½Ê±ï¿½ï¿½ï¿½ï¿½ï¿½
    uint32_t channel;       // ï¿½ï¿½Ê±ï¿½ï¿½Í¨ï¿½ï¿½
} VibrationControl_t;

extern VibrationControl_t vibration_controls[5];

extern uint8_t ucTemp4;
extern uint8_t ucTemp1;
extern uint8_t light_mode;
extern uint8_t vl53l0_mode;
extern uint8_t sendmessage_mode;
extern uint8_t askhelp_mode;
extern uint8_t notice_mode;

#define MAX_BUFFER_SIZE 200
extern uint8_t rx_buffer[MAX_BUFFER_SIZE];
extern volatile uint16_t rx_index;
extern volatile uint8_t message_received;

extern volatile uint8_t current_mode;   // Ä£Ê½ (0: fun2, 1: fun3)

extern char prefix[];
extern char fun2_prefix[];
extern char full_message[];
/**************Ð¾Æ¬*********************/
extern uint8_t SYN_StopCom[]; //Ö¹Í£Ï³
extern uint8_t SYN_SuspendCom[]; //Í£Ï³
extern uint8_t SYN_RecoverCom[]; //Ö¸Ï³
extern uint8_t SYN_ChackCom[]; //×´Ì¬Ñ¯
extern uint8_t SYN_PowerDownCom[]; //POWER DOWN ×´Ì¬

extern uint8_t k230_data[pack_num];
extern uint8_t receive_x; //Í¼ï¿½ï¿½xï¿½ï¿½ï¿½ï¿½
extern uint8_t receive_flag; //Í¼ï¿½ï¿½yï¿½ï¿½ï¿½ï¿½

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void VL53L0X_Init(void);
void VL53L0X_GetValue(void);
void control_vibration_by_distance(uint16_t distance, uint8_t sensor_index);
void start_vibration(uint16_t intensity, uint16_t duration, uint8_t sensor_index);
void update_vibrations(void);

void VL53L0_process(void);
void select_yuyin(void);
void Send_String(UART_HandleTypeDef *huart, char *str);
void k230_run(void);

void get_current_mode(void);
void mode_processing(void);
void buletooth_process(void);

#endif
