/*
 * 蓝朋电子硬件开发板配套扩展板硬件例程全部资源
 * 淘宝网址：www.lckfb.com
 * 技术支持驻扎论坛，任何技术问题欢迎随时交流学习
 * 一起共进！
 */

#ifndef _BSP_WS2812_H_
#define _BSP_WS2812_H_
 
#include "stm32f4xx.h"
 
#define RCC_DIN     RCC_AHB1Periph_GPIOD
#define PORT_DIN    GPIOD
#define GPIO_DIN    GPIO_PIN_1

 
//用户修改参数：
//#define WS2812_FREQUENCY
#define RGB_PIN_L()          HAL_GPIO_WritePin(PORT_DIN, GPIO_DIN, GPIO_PIN_RESET)  //控制彩灯引脚，需要定义为强推输出引脚
#define RGB_PIN_H()          HAL_GPIO_WritePin(PORT_DIN, GPIO_DIN, GPIO_PIN_SET)    //控制彩灯引脚，需要定义为强推输出引脚
#define WS2812_MAX        10                        //彩灯最大数
#define WS2812_NUMBERS    10                        //彩灯个数


// 颜色定义使用0xRRGGBB格式 (确认为RGB顺序)
// 注意：WS2812B芯片实际使用GRB顺序，在rgb_SetColor函数中已做处理
// 确保所有颜色都使用完整的32位格式，前缀0x00，例如0x00FF0000
#define RED               0x00FF0000                  //红色
#define GREEN             0x0000FF00                  //绿色
#define BLUE              0x000000FF                  //蓝色
#define BLACK             0x00000000                  //黑色(关闭)
#define WHITE             0x00FFFFFF                  //白色

// 添加更多常用颜色定义
#define YELLOW            0x00FFFF00                  //黄色(红+绿)
#define CYAN              0x0000FFFF                  //青色(绿+蓝)
#define MAGENTA           0x00FF00FF                  //洋红(红+蓝)
#define ORANGE            0x00FFA500                  //橙色
#define PURPLE            0x00800080                  //紫色


//8.3 -8  0.000000083 
//4.16 -9 0.00000000416
void Ws2812b_WriteByte(unsigned char byte);//输入一个字节数据，@12.000MHz,高电平每个周期大约83ns,低电平约为76ns；                                                      
void setLedCount(unsigned char count);//设置彩灯数目，范围0-25.                                                           
unsigned char getLedCount();//彩灯数目查询函数；                                                                     
void rgb_SetColor(unsigned char LedId, unsigned long color);//设置彩灯颜色                                     
void rgb_SetRGB(unsigned char LedId, unsigned long red, unsigned long green, unsigned long blue);//设置彩灯颜色
void rgb_SendArray();//发送彩灯数据   

void RGB_LED_Write1(void);
void RGB_LED_Reset(void);
void delay_us(uint32_t _us);
void WS2812_Test_Single_Color(unsigned long color);
void WS2812_Test_Data_Line(void);
int WS2812_Detect_Count(void);
void WS2812_Final_Reset(void); // 添加最终重置函数声明
unsigned long apply_brightness(unsigned long original_color, float brightness_factor);

#endif




