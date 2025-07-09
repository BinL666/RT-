#ifndef __SYN6288_UART_H
#define __SYN6288_UART_H
#include "usart.h"

#include "stm32f4xx.h"

#define USART1_REC_LEN              200     //定义最大接收字节数 200
#define EN_USART1_RX            1       //使能（1）/禁止（0）串口1接收

extern uint8_t  USART1_RX_BUF[USART1_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符
extern uint16_t USART1_RX_STA;              //接收状态标记

#define pack_head_1 0xa3
#define pack_head_2 0xb3
#define pack_tail   0x5a

#define useful_bites 2
#define pack_num    useful_bites + 3

void USART1_SendString(uint8_t *DAT, uint8_t len);
void USART1_SendData(uint8_t ucch);
void UART4_IRQHandler(void);
int data_test(uint8_t data[]);

#endif

