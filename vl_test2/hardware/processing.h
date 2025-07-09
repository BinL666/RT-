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

#define PACK_HEAD2 0x33   // ��ͷ
#define PACK_TAIL2 0x66   // ��β
#define PACK_SIZE2 3      // �ܰ�����

extern uint8_t bluetooth_data[PACK_SIZE2]; // ���ջ�����
extern uint8_t data_index;            // ������������
extern uint8_t valid_data;            // �洢����������Ч����

// ����5���������Ĳ������ݽṹ
extern VL53L0X_RangingMeasurementData_t vl53l0x_data1;
extern VL53L0X_RangingMeasurementData_t vl53l0x_data2;
extern VL53L0X_RangingMeasurementData_t vl53l0x_data3;
extern VL53L0X_RangingMeasurementData_t vl53l0x_data4;
extern VL53L0X_RangingMeasurementData_t vl53l0x_data5;

// �����𶯿�����س���
#define MIN_DISTANCE 100   // ��С���룬��λmm��С�ڴ˾���ʱ����ǿ
#define MAX_DISTANCE 2000  // �����룬��λmm�����ڴ˾���ʱ����
#define MAX_VIBRATION 100  // �����ǿ�ȣ�ռ�ձȣ�
#define MIN_VIBRATION 0    // ��С��ǿ�ȣ�ռ�ձȣ�
#define VIBRATION_DURATION 2000  // �𶯳���ʱ�䣬��λms

typedef struct {
    uint32_t start_time;    // �𶯿�ʼʱ��
    uint32_t duration;      // �𶯳���ʱ��
    uint8_t is_vibrating;   // �Ƿ�������
    uint16_t intensity;     // ��ǿ��
    TIM_HandleTypeDef *htim;// ��ʱ�����
    uint32_t channel;       // ��ʱ��ͨ��
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

extern volatile uint8_t current_mode;   // ģʽ (0: fun2, 1: fun3)

extern char prefix[];
extern char fun2_prefix[];
extern char full_message[];
/**************оƬ*********************/
extern uint8_t SYN_StopCom[]; //ֹͣϳ
extern uint8_t SYN_SuspendCom[]; //ͣϳ
extern uint8_t SYN_RecoverCom[]; //ָϳ
extern uint8_t SYN_ChackCom[]; //״̬ѯ
extern uint8_t SYN_PowerDownCom[]; //POWER DOWN ״̬

extern uint8_t k230_data[pack_num];
extern uint8_t receive_x; //ͼ��x����
extern uint8_t receive_flag; //ͼ��y����

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
