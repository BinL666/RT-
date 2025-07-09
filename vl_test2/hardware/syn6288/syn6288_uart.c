#include "syn6288_uart.h"

uint8_t k230_data[pack_num] = {0};
uint8_t receive_x = 0; //图像x坐标
uint8_t receive_flag = 0; //图像y坐标
void USART1_SendData(uint8_t ucch)
{
    HAL_UART_Transmit(&huart1, &ucch, 1, HAL_MAX_DELAY);
}

void USART1_SendString(uint8_t *DAT, uint8_t len)
{
    uint8_t i;
    for(i = 0; i < len; i++)
    {
        USART1_SendData(*DAT++);
    }
}

#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误
uint8_t USART1_RX_BUF[USART1_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，    接收完成标志
uint16_t USART1_RX_STA = 0;     //接收状态标记

void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}
#endif

void UART4_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart4);
}

int data_test(uint8_t data[])
{
    if(data[0]!=pack_head_1) return 0; //帧头
    if(data[1]!=pack_head_2) return 0; //帧头
    if(data[pack_num-1]!=pack_tail) return 0;//帧尾
    //检验数据合理性
    return 1;
}
