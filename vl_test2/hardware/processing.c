#include "processing.h"

uint8_t ucTemp4;
uint8_t ucTemp1;
uint8_t light_mode;
uint8_t vl53l0_mode;
uint8_t sendmessage_mode;
uint8_t askhelp_mode;
uint8_t notice_mode;
uint8_t send_message_falg = 0;
uint8_t duojirun_flag = 0;
uint16_t shangduoji = 50;
uint16_t xiaduoji = 60;

char sms[] = "config,set,sms,17396454714,E4BDA0E5A5BDEFBC8CE699BAE883BDE79BB2E5B8BDE58AA9E6898BE68F90E98692EFBC8CE682A8E79A84E69C8BE58F8BE99C80E8A681E5B8AEE58AA9E38082\r\n";
char sms2[] = "config,set,sms,17396454714,E4BDA0E5A5BDEFBC8CE699BAE883BDE79BB2E5B8BDE58AA9E6898BE68F90E98692EFBC8CE682A8E79A84E69C8BE58F8BE69194E58092E4BA86EFBC8CE8AFB7E58F8AE697B6E5B8AEE58AA9E38082\r\n";

VibrationControl_t vibration_controls[5] = {
    {0, 0, 0, 0, &htim4, TIM_CHANNEL_1}, // 传感器1对应TIM4_CH1
    {0, 0, 0, 0, &htim4, TIM_CHANNEL_2}, // 传感器2对应TIM4_CH2
    {0, 0, 0, 0, &htim4, TIM_CHANNEL_3}, // 传感器3对应TIM4_CH3
    {0, 0, 0, 0, &htim4, TIM_CHANNEL_4}, // 传感器4对应TIM4_CH4
    {0, 0, 0, 0, &htim3, TIM_CHANNEL_3}  // 传感器5对应TIM3_CH3
};

uint8_t bluetooth_data[PACK_SIZE2]; // ���ջ�����
uint8_t data_index = 0;            // ������������
uint8_t valid_data = 0;            // �洢����������Ч����


uint8_t rx_buffer[MAX_BUFFER_SIZE];
volatile uint16_t rx_index = 0;
volatile uint8_t message_received = 0;

volatile uint8_t current_mode = 0;   // Ĭ��Ϊfun2ģʽ (0: fun2, 1: fun3)

char prefix[] = "[v10][m0][t1]ǰ����";
char fun2_prefix[] = "[v10][m0][t1]";
char full_message[MAX_BUFFER_SIZE + sizeof(prefix)];


/**************芯片命令定义*********************/
uint8_t SYN_StopCom[] = {0xFD, 0X00, 0X02, 0X02, 0XFD}; //停止合成
uint8_t SYN_SuspendCom[] = {0XFD, 0X00, 0X02, 0X03, 0XFC}; //暂停合成
uint8_t SYN_RecoverCom[] = {0XFD, 0X00, 0X02, 0X04, 0XFB}; //恢复合成
uint8_t SYN_ChackCom[] = {0XFD, 0X00, 0X02, 0X21, 0XDE}; //状态查询
uint8_t SYN_PowerDownCom[] = {0XFD, 0X00, 0X02, 0X88, 0X77}; //进入POWER DOWN 状态命令

#if !defined(__MICROLIB)
//不使用微库的情况下需要定义这个函数
#if (__ARMCLIB_VERSION <= 6000000)
//如果是早期的AC5  就定义这个结构体
struct __FILE
{
	int handle;
};
#endif

FILE __stdout;

//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
	x = x;
}
#endif

int fputc(int ch,FILE *stream)
{
	HAL_UART_Transmit(&huart1,( uint8_t *)&ch,1,0xFFFF);
	return ch;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART2)
    {
        WitSerialDataIn(ucTemp);  // ��ģ�����ݴ���ú������д���

        // �������������ж�
        HAL_UART_Receive_IT(&huart2, &ucTemp, 1);
    }
		else if (huart->Instance == USART3){
			{
        // ����ͷ��Ч�ԣ������յĵ�һ���ֽ���Ҫ����ͷ��
        if(data_index == 0) {
            if(bluetooth_data[0] != PACK_HEAD2) {
                data_index = 0; // ��ͷ������������
                HAL_UART_Receive_IT(huart, &bluetooth_data[0], 1); // ���½���
                return;
            }
        }
        
        data_index++; // �ƶ�����һ���洢λ��
        
        // ����Ƿ��յ��������ݰ�
        if(data_index >= PACK_SIZE2)
        {
            // ���������
            if((bluetooth_data[0] == PACK_HEAD2) && 
               (bluetooth_data[PACK_SIZE2-1] == PACK_TAIL2)) 
            {
                valid_data = bluetooth_data[1]; // ��ȡ��Ч����
                duojirun_flag = 0;
                // �������������ݴ������
                // ���磺receive_1 = valid_data;
            }
            data_index = 0; // ��������׼�������°�
        }
        
        // ����������һ���ֽ�
        HAL_UART_Receive_IT(huart, &bluetooth_data[data_index], 1);
    }
		}
    else if (huart->Instance == UART4)  // ע�⣺HAL����UART4��ʵ����ͨ����UART4
       {
         static int i = 0;
        static int fun3_state = 0; // 0: �ȴ�֡ͷ1, 1: �ȴ�֡ͷ2, 2: ����������, 3: ���֡β
        static uint8_t fun3_buffer[MAX_BUFFER_SIZE];
        static uint16_t fun3_index = 0;
        
        // �ȳ��Խ���fun2��ʽ
        if(fun3_state == 0 && i == 0 && ucTemp4 == pack_head_1)
        {
            k230_data[i++] = ucTemp4;
            fun3_buffer[0] = ucTemp4;
            fun3_index = 1;
            fun3_state = 1; // ����ȴ�֡ͷ2״̬
        }
        else if(fun3_state == 1 && i == 1 && ucTemp4 == pack_head_2)
        {
            k230_data[i++] = ucTemp4;
            fun3_buffer[fun3_index++] = ucTemp4;
            fun3_state = 2; // �����������״̬
        }
        else if(i >= 2 && i < pack_num - 1)
        {
            k230_data[i++] = ucTemp4;
            if(fun3_state == 2)
            {
                fun3_buffer[fun3_index++] = ucTemp4;
                // ��ֹ���������
                if(fun3_index >= MAX_BUFFER_SIZE)
                {
                    fun3_index = 0;
                    fun3_state = 0;
                }
            }
        }
        else if(i == pack_num - 1)
        {
            k230_data[i] = ucTemp4;
            i = 0;
            
            // ����Ƿ���fun2���ݰ�֡
            if(k230_data[pack_num-1] == pack_tail)
            {
                if(data_test(k230_data))
                {
                    receive_x = k230_data[2]; // ��ȡx����
                    receive_flag = k230_data[3]; // ��ȡ���ID
                    current_mode = 0; // fun2ģʽ
                    fun3_state = 0; // ����fun3״̬
                    fun3_index = 0;
                }
            }
            
            // ��������fun3����
            if(fun3_state == 2)
            {
                fun3_buffer[fun3_index++] = ucTemp4;
            }
        }
        else
        {
            // ����fun3���ݰ�
            if(fun3_state == 2)
            {
                fun3_buffer[fun3_index++] = ucTemp4;
                
                // ����Ƿ���յ�fun3֡β (0xc3)
                if(ucTemp4 == 0xc3)
                {
                    // ȷ��������3���ֽ� (֡ͷ1+֡ͷ2+֡β)
                    if(fun3_index >= 3)
                    {
                        // ����Ƿ�Ϊfun3���ݰ� (֡ͷ1=0xa3, ֡ͷ2=0xb3, ֡β=0xc3)
                        if(fun3_buffer[0] == 0xa3 && fun3_buffer[1] == 0xb3)
                        {
                            // �����ı����ݵ�rx_buffer (������֡ͷ��֡β)
                            memcpy(rx_buffer, &fun3_buffer[2], fun3_index - 3);
                            rx_buffer[fun3_index - 3] = '\0'; // ȷ���ַ�����null��β
                            message_received = 1; // ���ñ�־��ʾ��Ϣ�ѽ���
                            current_mode = 1; // fun3ģʽ
                        }
                    }
                    fun3_state = 0; // ����״̬
                    fun3_index = 0;
                }
                else if(fun3_index >= MAX_BUFFER_SIZE) // ��ֹ���������
                {
                    fun3_state = 0;
                    fun3_index = 0;
                }
            }
            
            // ��������ڴ���fun3���ݰ�����������
            if(fun3_state == 0)
            {
                i = 0;
                
                // ����Ƿ�Ϊ�µ�fun3���ݰ��Ŀ�ʼ
                if(ucTemp4 == 0xa3)
                {
                    fun3_buffer[0] = ucTemp4;
                    fun3_index = 1;
                    fun3_state = 1; // ����ȴ�֡ͷ2״̬
                }
            }
        }

        HAL_UART_Receive_IT(huart, &ucTemp4, 1); // �ٴο�������
       }

    else if (huart->Instance == USART1)
       {
           // HAL�����жϷ������Զ������־λ�������ֶ����
           if ((USART1_RX_STA & 0x8000) == 0) // ����δ���
           {
               if (USART1_RX_STA < USART1_REC_LEN)
               {
                   HAL_TIM_Base_Stop(&htim10);
                   __HAL_TIM_SET_COUNTER(&htim10, 0); // ���ó�ʱ��ʱ��
                   if (USART1_RX_STA == 0)
                   {
                       HAL_TIM_Base_Start(&htim10); // ������ʱ�� TIM_Cmd(TIM10, ENABLE);
                   }
                   USART1_RX_BUF[USART1_RX_STA++] = ucTemp1; // �洢����
               }
               else
               {
                   USART1_RX_STA |= 0x8000; // ǿ�Ʊ�ǽ������
               }
           }
           HAL_UART_Receive_IT(huart, &ucTemp1, 1); // ��������
       }
}
void VL53L0X_Init(void)
{
    printf("test\n\r");
  uint8_t mode = 0;
  VL53L0X_Error Status_main = VL53L0X_ERROR_NONE; // ʹòͬıԱAPIڲStatus

  // ȷдڹر״̬
  VL53L0X_Xshut1 = 0;
  VL53L0X_Xshut2 = 0;
  VL53L0X_Xshut3 = 0;
  VL53L0X_Xshut4 = 0;
  VL53L0X_Xshut5 = 0;
  HAL_Delay(10);

  // ʼ1 (vl53l0x_dev1, XSHUT ӵ PE6)
  printf("Initializing VL53L0X Sensor 1 (XSHUT on PE6, Target Addr 0x54)...\n");
  VL53L0X_Xshut1 = 1; // ֻʹܴ1
  HAL_Delay(20);      // ȴȶ

  Status_main = vl53l0x_init1(&vl53l0x_dev1); // ˺᳢Խ1ĵַΪ0x54
  if (Status_main != VL53L0X_ERROR_NONE)
  {
      printf("VL53L0X1 Init Error!!! Status: %d\n\r", Status_main);
      print_pal_error(Status_main);
  } else {
      printf("VL53L0X1 OK (Address should be 0x54)\r\n");

      // ô1ģʽ
      if (vl53l0x_dev1.I2cDevAddr == VL53L0X_ADDR1) {
          printf("Setting mode for sensor 1...\n");
          Status_main = vl53l0x_set_mode(&vl53l0x_dev1, mode);
          if (Status_main != VL53L0X_ERROR_NONE) {
              printf("Mode Set Error Sensor 1: %d\r\n", Status_main);
              print_pal_error(Status_main);
          } else {
              printf("Sensor 1 mode set successfully\r\n");
          }
      }
  }

  // ��ʼ��������2 (vl53l0x_dev2, XSHUT ���ӵ� PE5)
  printf("Initializing VL53L0X Sensor 2 (XSHUT on PE5, Target Addr 0x56)...\n");
  VL53L0X_Xshut2 = 1; // ʹ�ܴ�����2
  HAL_Delay(20);      // �ȴ��������ȶ�

  Status_main = vl53l0x_init2(&vl53l0x_dev2); // �˺����᳢�Խ�������2�ĵ�ַ��Ϊ0x56
  if (Status_main != VL53L0X_ERROR_NONE)
  {
      printf("VL53L0X2 Init Error!!! Status: %d\n\r", Status_main);
      print_pal_error(Status_main);
  } else {
      printf("VL53L0X2 OK (Address should be 0x56)\r\n");

      // ���ô�����2��ģʽ
      if (vl53l0x_dev2.I2cDevAddr == VL53L0X_ADDR2) {
          printf("Setting mode for sensor 2...\n");
          Status_main = vl53l0x_set_mode(&vl53l0x_dev2, mode);
          if (Status_main != VL53L0X_ERROR_NONE) {
              printf("Mode Set Error Sensor 2: %d\r\n", Status_main);
              print_pal_error(Status_main);
          } else {
              printf("Sensor 2 mode set successfully\r\n");
          }
      }
  }

  // ��ʼ��������3 (vl53l0x_dev3, XSHUT ���ӵ� PE4)
  printf("Initializing VL53L0X Sensor 3 (XSHUT on PE4, Target Addr 0x58)...\n");
  VL53L0X_Xshut3 = 1; // ʹ�ܴ�����3
  HAL_Delay(20);      // �ȴ��������ȶ�

  Status_main = vl53l0x_init3(&vl53l0x_dev3); // �˺����᳢�Խ�������3�ĵ�ַ��Ϊ0x58
  if (Status_main != VL53L0X_ERROR_NONE)
  {
      printf("VL53L0X3 Init Error!!! Status: %d\n\r", Status_main);
      print_pal_error(Status_main);
  } else {
      printf("VL53L0X3 OK (Address should be 0x58)\r\n");

      // ���ô�����3��ģʽ
      if (vl53l0x_dev3.I2cDevAddr == VL53L0X_ADDR3) {
          printf("Setting mode for sensor 3...\n");
          Status_main = vl53l0x_set_mode(&vl53l0x_dev3, mode);
          if (Status_main != VL53L0X_ERROR_NONE) {
              printf("Mode Set Error Sensor 3: %d\r\n", Status_main);
              print_pal_error(Status_main);
          } else {
              printf("Sensor 3 mode set successfully\r\n");
          }
      }
  }

  // ��ʼ��������4 (vl53l0x_dev4, XSHUT ���ӵ� PE3)
  printf("Initializing VL53L0X Sensor 4 (XSHUT on PE3, Target Addr 0x5A)...\n");
  VL53L0X_Xshut4 = 1; // ʹ�ܴ�����4
  HAL_Delay(20);      // �ȴ��������ȶ�

  Status_main = vl53l0x_init4(&vl53l0x_dev4); // �˺����᳢�Խ�������4�ĵ�ַ��Ϊ0x5A
  if (Status_main != VL53L0X_ERROR_NONE)
  {
      printf("VL53L0X4 Init Error!!! Status: %d\n\r", Status_main);
      print_pal_error(Status_main);
  } else {
      printf("VL53L0X4 OK (Address should be 0x5A)\r\n");

      // ���ô�����4��ģʽ
      if (vl53l0x_dev4.I2cDevAddr == VL53L0X_ADDR4) {
          printf("Setting mode for sensor 4...\n");
          Status_main = vl53l0x_set_mode(&vl53l0x_dev4, mode);
          if (Status_main != VL53L0X_ERROR_NONE) {
              printf("Mode Set Error Sensor 4: %d\r\n", Status_main);
              print_pal_error(Status_main);
          } else {
              printf("Sensor 4 mode set successfully\r\n");
          }
      }
  }

  // ��ʼ��������5 (vl53l0x_dev5, XSHUT ���ӵ� PE2)
  printf("Initializing VL53L0X Sensor 5 (XSHUT on PE2, Target Addr 0x5C)...\n");
  VL53L0X_Xshut5 = 1; // ʹ�ܴ�����5
  HAL_Delay(20);      // �ȴ��������ȶ�

  Status_main = vl53l0x_init5(&vl53l0x_dev5); // �˺����᳢�Խ�������5�ĵ�ַ��Ϊ0x5C
  if (Status_main != VL53L0X_ERROR_NONE)
  {
      printf("VL53L0X5 Init Error!!! Status: %d\n\r", Status_main);
      print_pal_error(Status_main);
  } else {
      printf("VL53L0X5 OK (Address should be 0x5C)\r\n");

      // ���ô�����5��ģʽ
      if (vl53l0x_dev5.I2cDevAddr == VL53L0X_ADDR5) {
          printf("Setting mode for sensor 5...\n");
          Status_main = vl53l0x_set_mode(&vl53l0x_dev5, mode);
          if (Status_main != VL53L0X_ERROR_NONE) {
              printf("Mode Set Error Sensor 5: %d\r\n", Status_main);
              print_pal_error(Status_main);
          } else {
              printf("Sensor 5 mode set successfully\r\n");
          }
      }
  }

  // ������д������Ƿ��������ǵ��µ�ַ�ϱ�ʶ��
  uint16_t id_check = 0;

  if (vl53l0x_dev1.I2cDevAddr == VL53L0X_ADDR1) {
      VL53L0X_RdWord(&vl53l0x_dev1, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &id_check);
      printf("Sensor 1 ID check on 0x%02X: 0x%04X\n", vl53l0x_dev1.I2cDevAddr, id_check);
  }

  if (vl53l0x_dev2.I2cDevAddr == VL53L0X_ADDR2) {
      VL53L0X_RdWord(&vl53l0x_dev2, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &id_check);
      printf("Sensor 2 ID check on 0x%02X: 0x%04X\n", vl53l0x_dev2.I2cDevAddr, id_check);
  }

  if (vl53l0x_dev3.I2cDevAddr == VL53L0X_ADDR3) {
      VL53L0X_RdWord(&vl53l0x_dev3, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &id_check);
      printf("Sensor 3 ID check on 0x%02X: 0x%04X\n", vl53l0x_dev3.I2cDevAddr, id_check);
  }

  if (vl53l0x_dev4.I2cDevAddr == VL53L0X_ADDR4) {
      VL53L0X_RdWord(&vl53l0x_dev4, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &id_check);
      printf("Sensor 4 ID check on 0x%02X: 0x%04X\n", vl53l0x_dev4.I2cDevAddr, id_check);
  }

  if (vl53l0x_dev5.I2cDevAddr == VL53L0X_ADDR5) {
      VL53L0X_RdWord(&vl53l0x_dev5, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &id_check);
      printf("Sensor 5 ID check on 0x%02X: 0x%04X\n", vl53l0x_dev5.I2cDevAddr, id_check);
  }

  printf("All sensors initialized and configured.\r\n");
}

void VL53L0X_GetValue(void)
{
    // ��ȡ������1
       VL53L0X_Error status_dev1 = VL53L0X_PerformSingleRangingMeasurement(&vl53l0x_dev1, &vl53l0x_data1);
       if (status_dev1 == VL53L0X_ERROR_NONE)
       {
           printf("Sensor 1: %4dmm, Status: %d\r\n", vl53l0x_data1.RangeMilliMeter, vl53l0x_data1.RangeStatus);
       }
       else
       {
           printf("Sensor 1 Error: %d\r\n", status_dev1);
           print_pal_error(status_dev1); // ��ӡ����ϸ�Ĵ�����Ϣ
       }

       // ��ȡ������2
       VL53L0X_Error status_dev2 = VL53L0X_PerformSingleRangingMeasurement(&vl53l0x_dev2, &vl53l0x_data2);
       if (status_dev2 == VL53L0X_ERROR_NONE)
       {
         printf("Sensor 2: %4dmm, Status: %d\r\n", vl53l0x_data2.RangeMilliMeter, vl53l0x_data2.RangeStatus);
       }
       else
       {
         printf("Sensor 2 Error: %d\r\n", status_dev2);
         print_pal_error(status_dev2);
       }

       // ��ȡ������3
       VL53L0X_Error status_dev3 = VL53L0X_PerformSingleRangingMeasurement(&vl53l0x_dev3, &vl53l0x_data3);
       if (status_dev3 == VL53L0X_ERROR_NONE)
       {
         printf("Sensor 3: %4dmm, Status: %d\r\n", vl53l0x_data3.RangeMilliMeter, vl53l0x_data3.RangeStatus);
       }
       else
       {
         printf("Sensor 3 Error: %d\r\n", status_dev3);
         print_pal_error(status_dev3);
       }

       // ��ȡ������4
       VL53L0X_Error status_dev4 = VL53L0X_PerformSingleRangingMeasurement(&vl53l0x_dev4, &vl53l0x_data4);
       if (status_dev4 == VL53L0X_ERROR_NONE)
       {
         printf("Sensor 4: %4dmm, Status: %d\r\n", vl53l0x_data4.RangeMilliMeter, vl53l0x_data4.RangeStatus);
       }
       else
       {
         printf("Sensor 4 Error: %d\r\n", status_dev4);
         print_pal_error(status_dev4);
       }

       // ��ȡ������5
       VL53L0X_Error status_dev5 = VL53L0X_PerformSingleRangingMeasurement(&vl53l0x_dev5, &vl53l0x_data5);
       if (status_dev5 == VL53L0X_ERROR_NONE)
       {
         printf("Sensor 5: %4dmm, Status: %d\r\n", vl53l0x_data5.RangeMilliMeter, vl53l0x_data5.RangeStatus);
       }
       else
       {
         printf("Sensor 5 Error: %d\r\n", status_dev5);
         print_pal_error(status_dev5);
       }

       printf("------------------------------\r\n");
       HAL_Delay(500); // ��ʱ500ms
}


void control_vibration_by_distance(uint16_t distance, uint8_t sensor_index)
{
    uint16_t vibration_intensity = 0;
    uint16_t vibration_time = 0;
    
    // ��鴫���������Ƿ���Ч
    if (sensor_index >= 5) {
        return;
    }
    
    // ������ǿ�Ⱥ�ʱ��
    if (distance <= MIN_DISTANCE) {
        // ����С����Сֵ�����ǿ����
        vibration_intensity = MAX_VIBRATION;
        vibration_time = VIBRATION_DURATION;
    } else if (distance >= MAX_DISTANCE) {
        // ����������ֵ������
        vibration_intensity = MIN_VIBRATION;
        vibration_time = 0;
    } else {
        // �������м䣬����������
        float ratio = 1.0f - ((float)(distance - MIN_DISTANCE) / (float)(MAX_DISTANCE - MIN_DISTANCE));
        vibration_intensity = (uint16_t)(MIN_VIBRATION + ratio * (MAX_VIBRATION - MIN_VIBRATION));
        vibration_time = (uint16_t)(ratio * VIBRATION_DURATION);
    }
    
    // �����Ҫ���ҵ�ǰû���𶯣���ʼ��
    if (vibration_time > 0 && !vibration_controls[sensor_index].is_vibrating) {
printf("������%d ����: %dmm, ��ǿ��: %d%%, ��ʱ��: %dms\r\n", sensor_index + 1, distance, vibration_intensity, vibration_time);

        start_vibration(vibration_intensity, vibration_time, sensor_index);
    }
}

/**
 * @brief ��ʼ��
 * @param intensity ��ǿ�ȣ�ռ�ձȣ�
 * @param duration �𶯳���ʱ�䣬��λms
 * @param sensor_index ������������0-4��
 */
void start_vibration(uint16_t intensity, uint16_t duration, uint8_t sensor_index)
{
    // ��鴫���������Ƿ���Ч
    if (sensor_index >= 5) {
        return;
    }
    
    // ������ǿ��
    __HAL_TIM_SET_COMPARE(vibration_controls[sensor_index].htim, 
                          vibration_controls[sensor_index].channel, 
                          intensity);
    
    // ��¼�𶯿�ʼʱ��ͳ���ʱ��
    vibration_controls[sensor_index].start_time = HAL_GetTick();
    vibration_controls[sensor_index].duration = duration;
    vibration_controls[sensor_index].is_vibrating = 1;
    vibration_controls[sensor_index].intensity = intensity;
}

/**
 * @brief ����������״̬
 * @note ����ѭ���ж��ڵ��ô˺���������Ƿ���Ҫֹͣ��
 */
void update_vibrations(void)
{
    uint32_t current_time = HAL_GetTick();
    
    // ���ÿ���𶯿���
    for (uint8_t i = 0; i < 5; i++) {
        // ��������𶯣�����Ƿ���Ҫֹͣ
        if (vibration_controls[i].is_vibrating) {
            // ����Ƿ񳬹��𶯳���ʱ��
            if (current_time - vibration_controls[i].start_time >= vibration_controls[i].duration) {
                // ֹͣ��
                __HAL_TIM_SET_COMPARE(vibration_controls[i].htim, 
                                     vibration_controls[i].channel, 
                                     0);
                vibration_controls[i].is_vibrating = 0;
            }
        }
    }
}

void VL53L0_process(void)
{
	VL53L0X_GetValue(); 
  // ÿ������������������
            if (vl53l0x_data1.RangeStatus == 0) { // ������1��Ч
                control_vibration_by_distance(vl53l0x_data1.RangeMilliMeter, 0);
            }
            
            if (vl53l0x_data2.RangeStatus == 0) { // ������2��Ч
                control_vibration_by_distance(vl53l0x_data2.RangeMilliMeter, 1);
            }
            
            if (vl53l0x_data3.RangeStatus == 0) { // ������3��Ч
                control_vibration_by_distance(vl53l0x_data3.RangeMilliMeter, 2);
            }
            
            if (vl53l0x_data4.RangeStatus == 0) { // ������4��Ч
                control_vibration_by_distance(vl53l0x_data4.RangeMilliMeter, 3);
            }
            
            if (vl53l0x_data5.RangeStatus == 0) { // ������5��Ч
                control_vibration_by_distance(vl53l0x_data5.RangeMilliMeter, 4);
            }
            
            // ����������״̬
            update_vibrations();
}

void select_yuyin()
{
    switch(receive_flag){
        case 0x01:
            snprintf(full_message, sizeof(full_message), "%s%s", prefix, "��");
            break;
        case 0x02:
            snprintf(full_message, sizeof(full_message), "%s%s", prefix, "���г�");
            break;
        case 0x03:
            snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
            break;
        case 0x04:
            snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Ħ�г�");
            break;
        case 0x05:
            snprintf(full_message, sizeof(full_message), "%s%s", prefix, "�ɻ�");
            break;
        case 0x06:
            snprintf(full_message, sizeof(full_message), "%s%s", prefix, "��������");
            break;
        case 0x07:
            snprintf(full_message, sizeof(full_message), "%s%s", prefix, "��");
            break;
        case 0x08:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
        break;
    case 0x09:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "��");
        break;
    case 0x0A:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "��ͨ��");
        break;
    case 0x0B:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����˨");
        break;
    case 0x0C:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ͣ����־");
        break;
    case 0x0D:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ͣ���շѱ�");
        break;
    case 0x0E:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
        break;
    case 0x0F:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "��");
        break;
        case 0x10:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "è");
        break;
    case 0x11:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "��");
        break;
    case 0x12:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "��");
        break;
    case 0x13:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "��");
        break;
    case 0x14:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ţ");
        break;
    case 0x15:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
        break;
    case 0x16:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "��");
        break;
    case 0x17:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
        break;
    case 0x18:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����¹");
        break;
    case 0x19:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
        break;
    case 0x1A:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "��ɡ");
        break;
    case 0x1B:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "�����");
        break;
    case 0x1C:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "���");
        break;
    case 0x1D:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "������");
        break;
    case 0x1E:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
        break;
    case 0x1F:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "��ѩ��");
        break;
    case 0x20:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "�˶���");
        break;
    case 0x21:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
        break;
    case 0x22:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "�����");
        break;
    case 0x23:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "��������");
        break;
    case 0x24:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
        break;
    case 0x25:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "���˰�");
        break;
    case 0x26:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "������");
        break;
    case 0x27:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ƿ��");
        break;
    case 0x28:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "�Ʊ�");
        break;
    case 0x29:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
        break;
    case 0x2A:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
        break;
    case 0x2B:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
        break;
    case 0x2C:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
        break;
    case 0x2D:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "��");
        break;
    case 0x2E:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "�㽶");
        break;
    case 0x2F:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ƻ��");
        break;
    case 0x30:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "������");
        break;
    case 0x31:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
        break;
    case 0x32:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "������");
        break;
    case 0x33:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "���ܲ�");
        break;
    case 0x34:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "�ȹ�");
        break;
    case 0x35:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
        break;
    case 0x36:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����Ȧ");
        break;
    case 0x37:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
        break;
    case 0x38:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
        break;
    case 0x39:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ɳ��");
        break;
    case 0x3A:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
        break;
    case 0x3B:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "��");
        break;
    case 0x3C:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
                break;
        case 0x3D:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "��Ͱ");
                break;
        case 0x3E:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
                break;
        case 0x3F:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "�ʼǱ�����");
                break;
        case 0x40:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "���");
                break;
        case 0x41:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ң����");
                break;
        case 0x42:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
                break;
        case 0x43:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "�ֻ�");
                break;
        case 0x44:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "΢��¯");
                break;
        case 0x45:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
                break;
        case 0x46:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "�������");
                break;
        case 0x47:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ˮ��");
                break;
        case 0x48:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
                break;
        case 0x49:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "��");
                break;
        case 0x4A:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "�ӱ�");
                break;
        case 0x4B:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "��ƿ");
                break;
        case 0x4C:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "����");
                break;
        case 0x4D:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "̩����");
                break;
        case 0x4E:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "�����");
                break;
        case 0x4F:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "��ˢ");
                break;
    }
}

void Send_String(UART_HandleTypeDef *huart, char *str) {
  // �����ַ������ȣ���������ֹ����
  uint16_t len = 0;
  while (str[len] != '\0') len++;
  
  // ��������
  HAL_UART_Transmit(huart, (uint8_t *)str, len, HAL_MAX_DELAY);
}

void k230_run(void)
{
	if(current_mode == 0)
		{
			if(receive_flag != 0)
			{
				select_yuyin();
				if(receive_flag != 0)
				{
					SYN_FrameInfo(2, (uint8_t *)full_message);
				}
				receive_flag = 0;
			}
		}
		else if(current_mode == 1)
		{
			if(message_received)
			{
				strcpy(full_message, fun2_prefix);
				strcat(full_message, rx_buffer);
				SYN_FrameInfo(2, (uint8_t *)full_message);
				message_received = 0; // ���ñ�־λ
			}
		}
}


void get_current_mode(void)
{
    if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_11) == 0 && HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_10) == 0)
        light_mode = 1;
    else if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_11) == 0 && HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_10) == 1)
        light_mode = 2;
    else if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_11) == 1 && HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_10) == 0)
        light_mode = 3;
    else if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_11) == 1 && HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_10) == 1)
			light_mode = 4;
		if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_7))
        light_mode = 6;
    if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_8))
        light_mode = 5;
		
    if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15)) vl53l0_mode = 1; else vl53l0_mode = 0;
		if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14)) sendmessage_mode = 1; else sendmessage_mode = 0;
		if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_13)) askhelp_mode = 1; else askhelp_mode = 0;
		if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12)) notice_mode = 1; else notice_mode = 0;
}

void mode_processing(void)
{
	if(light_mode == 1) 
	{
		HAL_Delay(1000);
		for(int led = 0; led < WS2812_NUMBERS; led++)
        {
            rgb_SetColor(led, BLACK);
        }
        rgb_SendArray();
				HAL_Delay(1000);
				light_mode =10;
	}
	else if(light_mode == 2) ws2812_set_mode(MODE_WHITE_LOW);
	else if(light_mode == 3) ws2812_set_mode(MODE_WHITE_MID);
	else if(light_mode == 4) ws2812_set_mode(MODE_WHITE_HIGH);
	else if(light_mode == 5) ws2812_set_mode(MODE_BLINK_BREATH);
	else if(light_mode == 6) ws2812_set_mode(MODE_COLOR_FLOW);
	
	
	if(vl53l0_mode) VL53L0_process();
	else
	{
		 __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, 0);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);
	}
	if(sendmessage_mode && send_message_falg == 0)  
	{
		Send_String(&huart6, sms); 
		send_message_falg =1;
	}
	if(askhelp_mode) 
	{
		SYN_FrameInfo(2,(uint8_t *)"[v10][m0][t1]��ã�����ä�ˣ�����Ҫ����");
		HAL_Delay(4000);
	}
	if(notice_mode)
	{
		SYN_FrameInfo(2,(uint8_t *)"[v10][m0][t1]��ã�����ä�ˣ���ע�����");
		HAL_Delay(4000);
	}
}

void buletooth_process(void)
{
	if(duojirun_flag == 0)
	{
		if(valid_data == 0x01)//��
		{
			shangduoji -= 10;
			__HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_1, shangduoji);
			duojirun_flag = 1;
		}
		else if(valid_data == 0x02)//��
		{
			shangduoji += 10;
			__HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_1, shangduoji);
			duojirun_flag = 1;
		}
		else if(valid_data == 0x03)//��
		{
			xiaduoji += 10;
			__HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_2, xiaduoji);
			duojirun_flag = 1;
		}
		else if(valid_data == 0x04)//��
		{
			xiaduoji -= 10;
			__HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_2, xiaduoji);
			duojirun_flag = 1;
		}
		else if(valid_data == 0x05)
		{
			SYN_FrameInfo(2,(uint8_t *)"[v10][m0][t1]����ˤ���ˣ����������ϵ������");
			Send_String(&huart6, sms2); 
			ws2812_set_mode(MODE_BLINK_BREATH);
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
			HAL_Delay(5000);
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);
			duojirun_flag = 1;
		}
		else if(valid_data == 0x06)
		{
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
			HAL_Delay(5000);
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);
		}
	}
}
