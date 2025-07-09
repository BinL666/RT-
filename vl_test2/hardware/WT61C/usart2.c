#include "stdio.h"
#include "wit_c_sdk.h"
#include "usart2.h"
#include "usart.h"
uint8_t ucTemp;

void uart2_init(uint32_t __Baud)
{
    // 1. ����GPIO��USART2ʱ��
    __HAL_RCC_GPIOD_CLK_ENABLE();    // ʹ��GPIODʱ��
    __HAL_RCC_USART2_CLK_ENABLE();   // ʹ��USART2ʱ��
    
    // 2. ����GPIO����ΪUSART2���ù���
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // TX�������� (PD5)
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
    // RX�������� (PD6)
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
    // 3. ����USART2����
    huart2.Instance = USART2;
    huart2.Init.BaudRate = __Baud;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    
    // 4. ��ʼ��USART2
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        // ��ʼ��������
        Error_Handler();
    }
    
    // 5. ʹ�ܽ����ж�
    HAL_UART_Receive_IT(&huart2, &ucTemp, 1);  // ucTempΪȫ�ֱ��������ڴ洢���յ�������
}

/******** ����1 �жϷ����� ***********/
// �жϽ��ջص�����

void Uart2Send(unsigned char *p_data, unsigned int uiSize)
{
    // ʹ��HAL�������ʽ���ͺ���
    HAL_UART_Transmit(&huart2, p_data, uiSize, HAL_MAX_DELAY);
    
    // ����ʹ���жϷ�ʽ���ͣ���������
    // HAL_UART_Transmit_IT(&huart2, p_data, uiSize);
    
    // ����ʹ��DMA��ʽ���ͣ����Ч��
    // HAL_UART_Transmit_DMA(&huart2, p_data, uiSize);
}



