#ifndef __USART2_H__
 #define __USART2_H__
 
 #include "stm32f4xx.h"
 extern uint8_t ucTemp;
 
 
 //�ⲿ�ɵ��ú���������
void uart2_init(uint32_t __Baud);
void Uart2Send(unsigned char *p_data, unsigned int uiSize);

 
 
 
 
 
#endif
