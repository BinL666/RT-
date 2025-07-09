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
    {0, 0, 0, 0, &htim4, TIM_CHANNEL_1}, // ä¼ æ„Ÿå™¨1å¯¹åº”TIM4_CH1
    {0, 0, 0, 0, &htim4, TIM_CHANNEL_2}, // ä¼ æ„Ÿå™¨2å¯¹åº”TIM4_CH2
    {0, 0, 0, 0, &htim4, TIM_CHANNEL_3}, // ä¼ æ„Ÿå™¨3å¯¹åº”TIM4_CH3
    {0, 0, 0, 0, &htim4, TIM_CHANNEL_4}, // ä¼ æ„Ÿå™¨4å¯¹åº”TIM4_CH4
    {0, 0, 0, 0, &htim3, TIM_CHANNEL_3}  // ä¼ æ„Ÿå™¨5å¯¹åº”TIM3_CH3
};

uint8_t bluetooth_data[PACK_SIZE2]; // ½ÓÊÕ»º³åÇø
uint8_t data_index = 0;            // ½ÓÊÕÊı¾İË÷Òı
uint8_t valid_data = 0;            // ´æ´¢½âÎö³öµÄÓĞĞ§Êı¾İ


uint8_t rx_buffer[MAX_BUFFER_SIZE];
volatile uint16_t rx_index = 0;
volatile uint8_t message_received = 0;

volatile uint8_t current_mode = 0;   // Ä¬ÈÏÎªfun2Ä£Ê½ (0: fun2, 1: fun3)

char prefix[] = "[v10][m0][t1]Ç°·½ÓĞ";
char fun2_prefix[] = "[v10][m0][t1]";
char full_message[MAX_BUFFER_SIZE + sizeof(prefix)];


/**************èŠ¯ç‰‡å‘½ä»¤å®šä¹‰*********************/
uint8_t SYN_StopCom[] = {0xFD, 0X00, 0X02, 0X02, 0XFD}; //åœæ­¢åˆæˆ
uint8_t SYN_SuspendCom[] = {0XFD, 0X00, 0X02, 0X03, 0XFC}; //æš‚åœåˆæˆ
uint8_t SYN_RecoverCom[] = {0XFD, 0X00, 0X02, 0X04, 0XFB}; //æ¢å¤åˆæˆ
uint8_t SYN_ChackCom[] = {0XFD, 0X00, 0X02, 0X21, 0XDE}; //çŠ¶æ€æŸ¥è¯¢
uint8_t SYN_PowerDownCom[] = {0XFD, 0X00, 0X02, 0X88, 0X77}; //è¿›å…¥POWER DOWN çŠ¶æ€å‘½ä»¤

#if !defined(__MICROLIB)
//ä¸ä½¿ç”¨å¾®åº“çš„æƒ…å†µä¸‹éœ€è¦å®šä¹‰è¿™ä¸ªå‡½æ•°
#if (__ARMCLIB_VERSION <= 6000000)
//å¦‚æœæ˜¯æ—©æœŸçš„AC5  å°±å®šä¹‰è¿™ä¸ªç»“æ„ä½“
struct __FILE
{
	int handle;
};
#endif

FILE __stdout;

//å®šä¹‰_sys_exit()ä»¥é¿å…ä½¿ç”¨åŠä¸»æœºæ¨¡å¼
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
        WitSerialDataIn(ucTemp);  // ½«Ä£¿éÊı¾İ´«Èë¸Ãº¯Êı½øĞĞ´¦Àí

        // ÖØĞÂÆô¶¯½ÓÊÕÖĞ¶Ï
        HAL_UART_Receive_IT(&huart2, &ucTemp, 1);
    }
		else if (huart->Instance == USART3){
			{
        // ¼ì²é°üÍ·ÓĞĞ§ĞÔ£¨½ö½ÓÊÕµÄµÚÒ»¸ö×Ö½ÚĞèÒª¼ì²é°üÍ·£©
        if(data_index == 0) {
            if(bluetooth_data[0] != PACK_HEAD2) {
                data_index = 0; // °üÍ·´íÎó£¬ÖØÖÃË÷Òı
                HAL_UART_Receive_IT(huart, &bluetooth_data[0], 1); // ÖØĞÂ½ÓÊÕ
                return;
            }
        }
        
        data_index++; // ÒÆ¶¯µ½ÏÂÒ»¸ö´æ´¢Î»ÖÃ
        
        // ¼ì²éÊÇ·ñÊÕµ½ÍêÕûÊı¾İ°ü
        if(data_index >= PACK_SIZE2)
        {
            // ÍêÕû°ü¼ì²é
            if((bluetooth_data[0] == PACK_HEAD2) && 
               (bluetooth_data[PACK_SIZE2-1] == PACK_TAIL2)) 
            {
                valid_data = bluetooth_data[1]; // ÌáÈ¡ÓĞĞ§Êı¾İ
                duojirun_flag = 0;
                // ÕâÀï¿ÉÒÔÌí¼ÓÊı¾İ´¦Àí´úÂë
                // ÀıÈç£ºreceive_1 = valid_data;
            }
            data_index = 0; // ÖØÖÃË÷Òı×¼±¸½ÓÊÕĞÂ°ü
        }
        
        // ¼ÌĞø½ÓÊÕÏÂÒ»¸ö×Ö½Ú
        HAL_UART_Receive_IT(huart, &bluetooth_data[data_index], 1);
    }
		}
    else if (huart->Instance == UART4)  // ×¢Òâ£ºHAL¿âÖĞUART4µÄÊµÀıÃûÍ¨³£ÊÇUART4
       {
         static int i = 0;
        static int fun3_state = 0; // 0: µÈ´ıÖ¡Í·1, 1: µÈ´ıÖ¡Í·2, 2: ½ÓÊÕÊı¾İÖĞ, 3: ¼ì²éÖ¡Î²
        static uint8_t fun3_buffer[MAX_BUFFER_SIZE];
        static uint16_t fun3_index = 0;
        
        // ÏÈ³¢ÊÔ½âÎöfun2¸ñÊ½
        if(fun3_state == 0 && i == 0 && ucTemp4 == pack_head_1)
        {
            k230_data[i++] = ucTemp4;
            fun3_buffer[0] = ucTemp4;
            fun3_index = 1;
            fun3_state = 1; // ½øÈëµÈ´ıÖ¡Í·2×´Ì¬
        }
        else if(fun3_state == 1 && i == 1 && ucTemp4 == pack_head_2)
        {
            k230_data[i++] = ucTemp4;
            fun3_buffer[fun3_index++] = ucTemp4;
            fun3_state = 2; // ½øÈë½ÓÊÕÊı¾İ×´Ì¬
        }
        else if(i >= 2 && i < pack_num - 1)
        {
            k230_data[i++] = ucTemp4;
            if(fun3_state == 2)
            {
                fun3_buffer[fun3_index++] = ucTemp4;
                // ·ÀÖ¹»º³åÇøÒç³ö
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
            
            // ¼ì²éÊÇ·ñÊÇfun2Êı¾İ°üÖ¡
            if(k230_data[pack_num-1] == pack_tail)
            {
                if(data_test(k230_data))
                {
                    receive_x = k230_data[2]; // »ñÈ¡x×ø±ê
                    receive_flag = k230_data[3]; // »ñÈ¡Àà±ğID
                    current_mode = 0; // fun2Ä£Ê½
                    fun3_state = 0; // ÖØÖÃfun3×´Ì¬
                    fun3_index = 0;
                }
            }
            
            // ¼ÌĞø½ÓÊÕfun3Êı¾İ
            if(fun3_state == 2)
            {
                fun3_buffer[fun3_index++] = ucTemp4;
            }
        }
        else
        {
            // ´¦Àífun3Êı¾İ°ü
            if(fun3_state == 2)
            {
                fun3_buffer[fun3_index++] = ucTemp4;
                
                // ¼ì²éÊÇ·ñ½ÓÊÕµ½fun3Ö¡Î² (0xc3)
                if(ucTemp4 == 0xc3)
                {
                    // È·±£ÖÁÉÙÓĞ3¸ö×Ö½Ú (Ö¡Í·1+Ö¡Í·2+Ö¡Î²)
                    if(fun3_index >= 3)
                    {
                        // ¼ì²éÊÇ·ñÎªfun3Êı¾İ°ü (Ö¡Í·1=0xa3, Ö¡Í·2=0xb3, Ö¡Î²=0xc3)
                        if(fun3_buffer[0] == 0xa3 && fun3_buffer[1] == 0xb3)
                        {
                            // ¿½±´ÎÄ±¾ÄÚÈİµ½rx_buffer (²»°üÀ¨Ö¡Í·ºÍÖ¡Î²)
                            memcpy(rx_buffer, &fun3_buffer[2], fun3_index - 3);
                            rx_buffer[fun3_index - 3] = '\0'; // È·±£×Ö·û´®ÓĞnull½áÎ²
                            message_received = 1; // ÉèÖÃ±êÖ¾±íÊ¾ĞÅÏ¢ÒÑ½ÓÊÕ
                            current_mode = 1; // fun3Ä£Ê½
                        }
                    }
                    fun3_state = 0; // ÖØÖÃ×´Ì¬
                    fun3_index = 0;
                }
                else if(fun3_index >= MAX_BUFFER_SIZE) // ·ÀÖ¹»º³åÇøÒç³ö
                {
                    fun3_state = 0;
                    fun3_index = 0;
                }
            }
            
            // Èç¹û²»ÊÇÔÚ´¦Àífun3Êı¾İ°ü£¬ÖØÖÃË÷Òı
            if(fun3_state == 0)
            {
                i = 0;
                
                // ¼ì²éÊÇ·ñÎªĞÂµÄfun3Êı¾İ°üµÄ¿ªÊ¼
                if(ucTemp4 == 0xa3)
                {
                    fun3_buffer[0] = ucTemp4;
                    fun3_index = 1;
                    fun3_state = 1; // ½øÈëµÈ´ıÖ¡Í·2×´Ì¬
                }
            }
        }

        HAL_UART_Receive_IT(huart, &ucTemp4, 1); // ÔÙ´Î¿ªÆô½ÓÊÕ
       }

    else if (huart->Instance == USART1)
       {
           // HAL¿âÔÚÖĞ¶Ï·şÎñÖĞ×Ô¶¯Çå³ı±êÖ¾Î»£¬ÎŞĞèÊÖ¶¯Çå³ı
           if ((USART1_RX_STA & 0x8000) == 0) // ½ÓÊÕÎ´Íê³É
           {
               if (USART1_RX_STA < USART1_REC_LEN)
               {
                   HAL_TIM_Base_Stop(&htim10);
                   __HAL_TIM_SET_COUNTER(&htim10, 0); // ÖØÖÃ³¬Ê±¶¨Ê±Æ÷
                   if (USART1_RX_STA == 0)
                   {
                       HAL_TIM_Base_Start(&htim10); // Æô¶¯¶¨Ê±Æ÷ TIM_Cmd(TIM10, ENABLE);
                   }
                   USART1_RX_BUF[USART1_RX_STA++] = ucTemp1; // ´æ´¢Êı¾İ
               }
               else
               {
                   USART1_RX_STA |= 0x8000; // Ç¿ÖÆ±ê¼Ç½ÓÊÕÍê³É
               }
           }
           HAL_UART_Receive_IT(huart, &ucTemp1, 1); // ÖØÆô½ÓÊÕ
       }
}
void VL53L0X_Init(void)
{
    printf("test\n\r");
  uint8_t mode = 0;
  VL53L0X_Error Status_main = VL53L0X_ERROR_NONE; // Ê¹Ã²Í¬Ä±Ô±APIÚ²Status

  // È·Ğ´Ú¹Ø±×´Ì¬
  VL53L0X_Xshut1 = 0;
  VL53L0X_Xshut2 = 0;
  VL53L0X_Xshut3 = 0;
  VL53L0X_Xshut4 = 0;
  VL53L0X_Xshut5 = 0;
  HAL_Delay(10);

  // Ê¼1 (vl53l0x_dev1, XSHUT Óµ PE6)
  printf("Initializing VL53L0X Sensor 1 (XSHUT on PE6, Target Addr 0x54)...\n");
  VL53L0X_Xshut1 = 1; // Ö»Ê¹Ü´1
  HAL_Delay(20);      // È´È¶

  Status_main = vl53l0x_init1(&vl53l0x_dev1); // Ëºá³¢Ô½1ÄµÖ·Îª0x54
  if (Status_main != VL53L0X_ERROR_NONE)
  {
      printf("VL53L0X1 Init Error!!! Status: %d\n\r", Status_main);
      print_pal_error(Status_main);
  } else {
      printf("VL53L0X1 OK (Address should be 0x54)\r\n");

      // Ã´1Ä£Ê½
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

  // ï¿½ï¿½Ê¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½2 (vl53l0x_dev2, XSHUT ï¿½ï¿½ï¿½Óµï¿½ PE5)
  printf("Initializing VL53L0X Sensor 2 (XSHUT on PE5, Target Addr 0x56)...\n");
  VL53L0X_Xshut2 = 1; // Ê¹ï¿½Ü´ï¿½ï¿½ï¿½ï¿½ï¿½2
  HAL_Delay(20);      // ï¿½È´ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½È¶ï¿½

  Status_main = vl53l0x_init2(&vl53l0x_dev2); // ï¿½Ëºï¿½ï¿½ï¿½ï¿½á³¢ï¿½Ô½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½2ï¿½Äµï¿½Ö·ï¿½ï¿½Îª0x56
  if (Status_main != VL53L0X_ERROR_NONE)
  {
      printf("VL53L0X2 Init Error!!! Status: %d\n\r", Status_main);
      print_pal_error(Status_main);
  } else {
      printf("VL53L0X2 OK (Address should be 0x56)\r\n");

      // ï¿½ï¿½ï¿½Ã´ï¿½ï¿½ï¿½ï¿½ï¿½2ï¿½ï¿½Ä£Ê½
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

  // ï¿½ï¿½Ê¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½3 (vl53l0x_dev3, XSHUT ï¿½ï¿½ï¿½Óµï¿½ PE4)
  printf("Initializing VL53L0X Sensor 3 (XSHUT on PE4, Target Addr 0x58)...\n");
  VL53L0X_Xshut3 = 1; // Ê¹ï¿½Ü´ï¿½ï¿½ï¿½ï¿½ï¿½3
  HAL_Delay(20);      // ï¿½È´ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½È¶ï¿½

  Status_main = vl53l0x_init3(&vl53l0x_dev3); // ï¿½Ëºï¿½ï¿½ï¿½ï¿½á³¢ï¿½Ô½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½3ï¿½Äµï¿½Ö·ï¿½ï¿½Îª0x58
  if (Status_main != VL53L0X_ERROR_NONE)
  {
      printf("VL53L0X3 Init Error!!! Status: %d\n\r", Status_main);
      print_pal_error(Status_main);
  } else {
      printf("VL53L0X3 OK (Address should be 0x58)\r\n");

      // ï¿½ï¿½ï¿½Ã´ï¿½ï¿½ï¿½ï¿½ï¿½3ï¿½ï¿½Ä£Ê½
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

  // ï¿½ï¿½Ê¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½4 (vl53l0x_dev4, XSHUT ï¿½ï¿½ï¿½Óµï¿½ PE3)
  printf("Initializing VL53L0X Sensor 4 (XSHUT on PE3, Target Addr 0x5A)...\n");
  VL53L0X_Xshut4 = 1; // Ê¹ï¿½Ü´ï¿½ï¿½ï¿½ï¿½ï¿½4
  HAL_Delay(20);      // ï¿½È´ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½È¶ï¿½

  Status_main = vl53l0x_init4(&vl53l0x_dev4); // ï¿½Ëºï¿½ï¿½ï¿½ï¿½á³¢ï¿½Ô½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½4ï¿½Äµï¿½Ö·ï¿½ï¿½Îª0x5A
  if (Status_main != VL53L0X_ERROR_NONE)
  {
      printf("VL53L0X4 Init Error!!! Status: %d\n\r", Status_main);
      print_pal_error(Status_main);
  } else {
      printf("VL53L0X4 OK (Address should be 0x5A)\r\n");

      // ï¿½ï¿½ï¿½Ã´ï¿½ï¿½ï¿½ï¿½ï¿½4ï¿½ï¿½Ä£Ê½
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

  // ï¿½ï¿½Ê¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½5 (vl53l0x_dev5, XSHUT ï¿½ï¿½ï¿½Óµï¿½ PE2)
  printf("Initializing VL53L0X Sensor 5 (XSHUT on PE2, Target Addr 0x5C)...\n");
  VL53L0X_Xshut5 = 1; // Ê¹ï¿½Ü´ï¿½ï¿½ï¿½ï¿½ï¿½5
  HAL_Delay(20);      // ï¿½È´ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½È¶ï¿½

  Status_main = vl53l0x_init5(&vl53l0x_dev5); // ï¿½Ëºï¿½ï¿½ï¿½ï¿½á³¢ï¿½Ô½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½5ï¿½Äµï¿½Ö·ï¿½ï¿½Îª0x5C
  if (Status_main != VL53L0X_ERROR_NONE)
  {
      printf("VL53L0X5 Init Error!!! Status: %d\n\r", Status_main);
      print_pal_error(Status_main);
  } else {
      printf("VL53L0X5 OK (Address should be 0x5C)\r\n");

      // ï¿½ï¿½ï¿½Ã´ï¿½ï¿½ï¿½ï¿½ï¿½5ï¿½ï¿½Ä£Ê½
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

  // ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ğ´ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ç·ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Çµï¿½ï¿½Âµï¿½Ö·ï¿½Ï±ï¿½Ê¶ï¿½ï¿½
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
    // ï¿½ï¿½È¡ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½1
       VL53L0X_Error status_dev1 = VL53L0X_PerformSingleRangingMeasurement(&vl53l0x_dev1, &vl53l0x_data1);
       if (status_dev1 == VL53L0X_ERROR_NONE)
       {
           printf("Sensor 1: %4dmm, Status: %d\r\n", vl53l0x_data1.RangeMilliMeter, vl53l0x_data1.RangeStatus);
       }
       else
       {
           printf("Sensor 1 Error: %d\r\n", status_dev1);
           print_pal_error(status_dev1); // ï¿½ï¿½Ó¡ï¿½ï¿½ï¿½ï¿½Ï¸ï¿½Ä´ï¿½ï¿½ï¿½ï¿½ï¿½Ï¢
       }

       // ï¿½ï¿½È¡ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½2
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

       // ï¿½ï¿½È¡ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½3
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

       // ï¿½ï¿½È¡ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½4
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

       // ï¿½ï¿½È¡ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½5
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
       HAL_Delay(500); // ï¿½ï¿½Ê±500ms
}


void control_vibration_by_distance(uint16_t distance, uint8_t sensor_index)
{
    uint16_t vibration_intensity = 0;
    uint16_t vibration_time = 0;
    
    // ï¿½ï¿½é´«ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ç·ï¿½ï¿½ï¿½Ğ§
    if (sensor_index >= 5) {
        return;
    }
    
    // ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ç¿ï¿½Èºï¿½Ê±ï¿½ï¿½
    if (distance <= MIN_DISTANCE) {
        // ï¿½ï¿½ï¿½ï¿½Ğ¡ï¿½ï¿½ï¿½ï¿½Ğ¡Öµï¿½ï¿½ï¿½ï¿½ï¿½Ç¿ï¿½ï¿½ï¿½ï¿½
        vibration_intensity = MAX_VIBRATION;
        vibration_time = VIBRATION_DURATION;
    } else if (distance >= MAX_DISTANCE) {
        // ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Öµï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
        vibration_intensity = MIN_VIBRATION;
        vibration_time = 0;
    } else {
        // ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ğ¼ä£¬ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
        float ratio = 1.0f - ((float)(distance - MIN_DISTANCE) / (float)(MAX_DISTANCE - MIN_DISTANCE));
        vibration_intensity = (uint16_t)(MIN_VIBRATION + ratio * (MAX_VIBRATION - MIN_VIBRATION));
        vibration_time = (uint16_t)(ratio * VIBRATION_DURATION);
    }
    
    // ï¿½ï¿½ï¿½ï¿½ï¿½Òªï¿½ï¿½ï¿½Òµï¿½Ç°Ã»ï¿½ï¿½ï¿½ğ¶¯£ï¿½ï¿½ï¿½Ê¼ï¿½ï¿½
    if (vibration_time > 0 && !vibration_controls[sensor_index].is_vibrating) {
printf("´«¸ĞÆ÷%d ¾àÀë: %dmm, ÕñÇ¿¶È: %d%%, ÕñÊ±¼ä: %dms\r\n", sensor_index + 1, distance, vibration_intensity, vibration_time);

        start_vibration(vibration_intensity, vibration_time, sensor_index);
    }
}

/**
 * @brief ï¿½ï¿½Ê¼ï¿½ï¿½
 * @param intensity ï¿½ï¿½Ç¿ï¿½È£ï¿½Õ¼ï¿½Õ±È£ï¿½
 * @param duration ï¿½ğ¶¯³ï¿½ï¿½ï¿½Ê±ï¿½ä£¬ï¿½ï¿½Î»ms
 * @param sensor_index ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½0-4ï¿½ï¿½
 */
void start_vibration(uint16_t intensity, uint16_t duration, uint8_t sensor_index)
{
    // ï¿½ï¿½é´«ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ç·ï¿½ï¿½ï¿½Ğ§
    if (sensor_index >= 5) {
        return;
    }
    
    // ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ç¿ï¿½ï¿½
    __HAL_TIM_SET_COMPARE(vibration_controls[sensor_index].htim, 
                          vibration_controls[sensor_index].channel, 
                          intensity);
    
    // ï¿½ï¿½Â¼ï¿½ğ¶¯¿ï¿½Ê¼Ê±ï¿½ï¿½Í³ï¿½ï¿½ï¿½Ê±ï¿½ï¿½
    vibration_controls[sensor_index].start_time = HAL_GetTick();
    vibration_controls[sensor_index].duration = duration;
    vibration_controls[sensor_index].is_vibrating = 1;
    vibration_controls[sensor_index].intensity = intensity;
}

/**
 * @brief ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½×´Ì¬
 * @note ï¿½ï¿½ï¿½ï¿½Ñ­ï¿½ï¿½ï¿½Ğ¶ï¿½ï¿½Úµï¿½ï¿½Ã´Ëºï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ç·ï¿½ï¿½ï¿½ÒªÍ£Ö¹ï¿½ï¿½
 */
void update_vibrations(void)
{
    uint32_t current_time = HAL_GetTick();
    
    // ï¿½ï¿½ï¿½Ã¿ï¿½ï¿½ï¿½ğ¶¯¿ï¿½ï¿½ï¿½
    for (uint8_t i = 0; i < 5; i++) {
        // ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ğ¶¯£ï¿½ï¿½ï¿½ï¿½ï¿½Ç·ï¿½ï¿½ï¿½ÒªÍ£Ö¹
        if (vibration_controls[i].is_vibrating) {
            // ï¿½ï¿½ï¿½ï¿½Ç·ñ³¬¹ï¿½ï¿½ğ¶¯³ï¿½ï¿½ï¿½Ê±ï¿½ï¿½
            if (current_time - vibration_controls[i].start_time >= vibration_controls[i].duration) {
                // Í£Ö¹ï¿½ï¿½
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
  // Ã¿ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
            if (vl53l0x_data1.RangeStatus == 0) { // ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½1ï¿½ï¿½Ğ§
                control_vibration_by_distance(vl53l0x_data1.RangeMilliMeter, 0);
            }
            
            if (vl53l0x_data2.RangeStatus == 0) { // ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½2ï¿½ï¿½Ğ§
                control_vibration_by_distance(vl53l0x_data2.RangeMilliMeter, 1);
            }
            
            if (vl53l0x_data3.RangeStatus == 0) { // ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½3ï¿½ï¿½Ğ§
                control_vibration_by_distance(vl53l0x_data3.RangeMilliMeter, 2);
            }
            
            if (vl53l0x_data4.RangeStatus == 0) { // ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½4ï¿½ï¿½Ğ§
                control_vibration_by_distance(vl53l0x_data4.RangeMilliMeter, 3);
            }
            
            if (vl53l0x_data5.RangeStatus == 0) { // ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½5ï¿½ï¿½Ğ§
                control_vibration_by_distance(vl53l0x_data5.RangeMilliMeter, 4);
            }
            
            // ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½×´Ì¬
            update_vibrations();
}

void select_yuyin()
{
    switch(receive_flag){
        case 0x01:
            snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ÈË");
            break;
        case 0x02:
            snprintf(full_message, sizeof(full_message), "%s%s", prefix, "×ÔĞĞ³µ");
            break;
        case 0x03:
            snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Æû³µ");
            break;
        case 0x04:
            snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Ä¦ÍĞ³µ");
            break;
        case 0x05:
            snprintf(full_message, sizeof(full_message), "%s%s", prefix, "·É»ú");
            break;
        case 0x06:
            snprintf(full_message, sizeof(full_message), "%s%s", prefix, "¹«¹²Æû³µ");
            break;
        case 0x07:
            snprintf(full_message, sizeof(full_message), "%s%s", prefix, "»ğ³µ");
            break;
        case 0x08:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "¿¨³µ");
        break;
    case 0x09:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "´¬");
        break;
    case 0x0A:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "½»Í¨µÆ");
        break;
    case 0x0B:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Ïû·ÀË¨");
        break;
    case 0x0C:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Í£³µ±êÖ¾");
        break;
    case 0x0D:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Í£³µÊÕ·Ñ±í");
        break;
    case 0x0E:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "³¤µÊ");
        break;
    case 0x0F:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Äñ");
        break;
        case 0x10:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Ã¨");
        break;
    case 0x11:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "¹·");
        break;
    case 0x12:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Âí");
        break;
    case 0x13:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Ñò");
        break;
    case 0x14:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Å£");
        break;
    case 0x15:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "´óÏó");
        break;
    case 0x16:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ĞÜ");
        break;
    case 0x17:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "°ßÂí");
        break;
    case 0x18:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "³¤¾±Â¹");
        break;
    case 0x19:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "±³°ü");
        break;
    case 0x1A:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ÓêÉ¡");
        break;
    case 0x1B:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ÊÖÌá°ü");
        break;
    case 0x1C:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Áì´ø");
        break;
    case 0x1D:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ÊÖÌáÏä");
        break;
    case 0x1E:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "·ÉÅÌ");
        break;
    case 0x1F:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "»¬Ñ©°å");
        break;
    case 0x20:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ÔË¶¯Çò");
        break;
    case 0x21:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "·çóİ");
        break;
    case 0x22:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "°ôÇò°ô");
        break;
    case 0x23:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "°ôÇòÊÖÌ×");
        break;
    case 0x24:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "»¬°å");
        break;
    case 0x25:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "³åÀË°å");
        break;
    case 0x26:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ÍøÇòÅÄ");
        break;
    case 0x27:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Æ¿×Ó");
        break;
    case 0x28:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "¾Æ±­");
        break;
    case 0x29:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "±­×Ó");
        break;
    case 0x2A:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "²æ×Ó");
        break;
    case 0x2B:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "µ¶×Ó");
        break;
    case 0x2C:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "É××Ó");
        break;
    case 0x2D:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Íë");
        break;
    case 0x2E:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Ïã½¶");
        break;
    case 0x2F:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Æ»¹û");
        break;
    case 0x30:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ÈıÃ÷ÖÎ");
        break;
    case 0x31:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "³È×Ó");
        break;
    case 0x32:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Î÷À¼»¨");
        break;
    case 0x33:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ºúÂÜ²·");
        break;
    case 0x34:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ÈÈ¹·");
        break;
    case 0x35:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ÅûÈø");
        break;
    case 0x36:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ÌğÌğÈ¦");
        break;
    case 0x37:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "µ°¸â");
        break;
    case 0x38:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ÒÎ×Ó");
        break;
    case 0x39:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "É³·¢");
        break;
    case 0x3A:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ÅèÔÔ");
        break;
    case 0x3B:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "´²");
        break;
    case 0x3C:
        snprintf(full_message, sizeof(full_message), "%s%s", prefix, "²Í×À");
                break;
        case 0x3D:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ÂíÍ°");
                break;
        case 0x3E:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "µçÊÓ");
                break;
        case 0x3F:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "±Ê¼Ç±¾µçÄÔ");
                break;
        case 0x40:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Êó±ê");
                break;
        case 0x41:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Ò£¿ØÆ÷");
                break;
        case 0x42:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "¼üÅÌ");
                break;
        case 0x43:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ÊÖ»ú");
                break;
        case 0x44:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Î¢²¨Â¯");
                break;
        case 0x45:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "¿¾Ïä");
                break;
        case 0x46:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "¿¾Ãæ°ü»ú");
                break;
        case 0x47:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Ë®²Û");
                break;
        case 0x48:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "±ùÏä");
                break;
        case 0x49:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Êé");
                break;
        case 0x4A:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ÖÓ±í");
                break;
        case 0x4B:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "»¨Æ¿");
                break;
        case 0x4C:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "¼ôµ¶");
                break;
        case 0x4D:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "Ì©µÏĞÜ");
                break;
        case 0x4E:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "´µ·ç»ú");
                break;
        case 0x4F:
                snprintf(full_message, sizeof(full_message), "%s%s", prefix, "ÑÀË¢");
                break;
    }
}

void Send_String(UART_HandleTypeDef *huart, char *str) {
  // ¼ÆËã×Ö·û´®³¤¶È£¨²»°üÀ¨ÖÕÖ¹·û£©
  uint16_t len = 0;
  while (str[len] != '\0') len++;
  
  // ·¢ËÍÊı¾İ
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
				message_received = 0; // ÖØÖÃ±êÖ¾Î»
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
		SYN_FrameInfo(2,(uint8_t *)"[v10][m0][t1]ÄãºÃ£¬ÎÒÊÇÃ¤ÈË£¬ÎÒĞèÒª°ïÖú");
		HAL_Delay(4000);
	}
	if(notice_mode)
	{
		SYN_FrameInfo(2,(uint8_t *)"[v10][m0][t1]ÄãºÃ£¬ÎÒÊÇÃ¤ÈË£¬Çë×¢Òâ±ÜÈÃ");
		HAL_Delay(4000);
	}
}

void buletooth_process(void)
{
	if(duojirun_flag == 0)
	{
		if(valid_data == 0x01)//ÉÏ
		{
			shangduoji -= 10;
			__HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_1, shangduoji);
			duojirun_flag = 1;
		}
		else if(valid_data == 0x02)//ÏÂ
		{
			shangduoji += 10;
			__HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_1, shangduoji);
			duojirun_flag = 1;
		}
		else if(valid_data == 0x03)//×ó
		{
			xiaduoji += 10;
			__HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_2, xiaduoji);
			duojirun_flag = 1;
		}
		else if(valid_data == 0x04)//ÓÒ
		{
			xiaduoji -= 10;
			__HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_2, xiaduoji);
			duojirun_flag = 1;
		}
		else if(valid_data == 0x05)
		{
			SYN_FrameInfo(2,(uint8_t *)"[v10][m0][t1]ÒÉËÆË¤µ¹ÁË£¬ÒÑÏò½ô¼±ÁªÏµÈËÇóÖú");
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
