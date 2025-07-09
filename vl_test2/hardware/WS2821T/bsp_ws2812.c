/*
 * ������������Ӳ�������������չ����Ӳ�����Ϲ���ȫ����Դ
 * �����������www.lckfb.com
 * ����֧�ֳ�פ��̳���κμ������⻶ӭ��ʱ����ѧϰ
 * ������̳��https://oshwhub.com/forum
 * ��עbilibili�˺ţ������������塿���������ǵ����¶�̬��
 * ��������׬Ǯ���������й�����ʦΪ����
 * 
 Change Logs:
 * Date           Author       Notes
 * 2024-03-22     LCKFB-LP    first version
 */
#include "bsp_ws2812.h"
#include "stdio.h"
#include "math.h"


unsigned char LedsArray[WS2812_MAX * 3];      //������ɫ���ݴ洢����
unsigned int  ledsCount   = WS2812_NUMBERS;   //����ʵ�ʲʵ�Ĭ�ϸ���
unsigned int  nbLedsBytes = WS2812_NUMBERS*3; //����ʵ�ʲʵ���ɫ���ݸ���

/******************************************************************
 * 函 数 名 称：WS2812_GPIO_Init
 * 功 能 说 明：WS2812初始化
 * 形 参 说 明：无
 * 返 回 值 说 明：无
 * 作       者：LC
 * 备       注：无
******************************************************************/

/******************************************************************
 * 函 数 名 称：rgb_SetColor
 * 功 能 说 明：设置LED颜色
 * 形 参 说 明：LedId：LED编号  color：颜色
 * 返 回 值 说 明：无
 * 作       者：LC
 * 备       注：无
******************************************************************/

void rgb_SetColor(unsigned char LedId, unsigned long color)
{
    if( LedId > ledsCount )
    {
        return;    //to avoid overflow
    }
    
    // Debug output to verify color value
    printf("DEBUG: rgb_SetColor LED %d with color 0x%08lX\r\n", LedId, color);
    
    // Extract RGB components for debugging
    unsigned char r = (color >> 16) & 0xFF;
    unsigned char g = (color >> 8) & 0xFF;
    unsigned char b = color & 0xFF;
    printf("DEBUG: Original RGB: R=%d, G=%d, B=%d\r\n", r, g, b);
    
    // Changing from RGB to GRB order as WS2812B expects GRB
    // Old code:
    // LedsArray[LedId * 3]     = (color>>8)&0xff;  // G
    // LedsArray[LedId * 3 + 1] = (color>>16)&0xff; // R
    // LedsArray[LedId * 3 + 2] = (color>>0)&0xff;  // B
    
    // New GRB ordering
    LedsArray[LedId * 3]     = (color>>8)&0xff;  // G
    LedsArray[LedId * 3 + 1] = (color>>16)&0xff; // R
    LedsArray[LedId * 3 + 2] = (color>>0)&0xff;  // B
    
    // Debug output to verify array values
    printf("DEBUG: LedsArray[%d*3+0]=%d (G), LedsArray[%d*3+1]=%d (R), LedsArray[%d*3+2]=%d (B)\r\n",
           LedId, LedsArray[LedId * 3], LedId, LedsArray[LedId * 3 + 1], LedId, LedsArray[LedId * 3 + 2]);
}

/******************************************************************
 *   函 数 名 称：rgb_SetRGB
 * 功 能 说 明：设置LED颜色(原色)
 * 形 参 说 明：LedId：LED编号  red：红色  green：绿色  blue：蓝色
 * 返 回 值 说 明：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void rgb_SetRGB(unsigned char LedId, unsigned long red, unsigned long green, unsigned long blue)
{
    unsigned long Color=red<<16|green<<8|blue;
    rgb_SetColor(LedId,Color);
}

/******************************************************************
 * 函 数 名 称：rgb_SendArray
 * 功 能 说 明：发送数据
 * 形 参 说 明：无
 * 返 回 值 说 明：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void rgb_SendArray(void)
{
    unsigned int i;
    //发送数据
    printf("DEBUG: rgb_SendArray - Sending data for %d LEDs (%d bytes)\r\n", ledsCount, nbLedsBytes);
    
    // 禁用中断以确保整个发送过程不被打断
    __disable_irq();
    
    // 首先发送一次完整重置，确保WS2812B处于准备接收状态
    RGB_PIN_L();
    delay_us(100); // 超过50us的重置时间，确保所有灯珠都能正确识别复位信号
    
    // 发送数据
    for(i=0; i<nbLedsBytes; i++) {
        Ws2812b_WriteByte(LedsArray[i]);
    }
    
    // 发送完所有数据后，添加一个至少50us的低电平，以确保锁存
    RGB_PIN_L();
    delay_us(60);
    
    // 最后添加一个短脉冲以稳定总线
    RGB_PIN_H();
    delay_us(1);
    RGB_PIN_L();
    
    // 恢复中断
    __enable_irq();
    
    // 给一个额外的延时确保数据稳定
    delay_us(50);
    
    printf("DEBUG: rgb_SendArray - Data transmission complete\r\n");
}

/******************************************************************
 * 函 数 名 称：RGB_LED_Reset
 * 功 能 说 明：复位ws2812
 * 形 参 说 明：无
 * 返 回 值 说 明：无
 * 作       者：LC
 * 备       注：低电平280us左右
******************************************************************/
void RGB_LED_Reset(void)
{
    printf("DEBUG: RGB_LED_Reset - Setting pin LOW for reset\r\n");
    
    // 禁用中断以确保精确时序
    __disable_irq();
    
    // WS2812B的重置需要>50us的低电平（官方规格）
    // 为确保可靠性，我们使用100us作为重置时间
    RGB_PIN_L();
    delay_us(100); // 更长的重置时间，确保所有灯珠能够正确复位
    
    // 在重置后添加一个短暂的高电平脉冲，然后再给一个低电平，以稳定总线状态
    RGB_PIN_H();
    delay_us(1);
    RGB_PIN_L();
    delay_us(1);
    
    // 恢复中断
    __enable_irq();
    
    printf("DEBUG: RGB_LED_Reset - Reset complete\r\n");
}

/******************************************************************
 * 函 数 名 称：Ws2812b_WriteByte
 * 功 能 说 明：WS2812写单字节
 * 形 参 说 明：byte写入字节
 * 返 回 值 说 明：无
 * 作       者：LC
 * 备       注：1高电平 = 高电平580ns~1us    低电平220ns~420ns
 *              0高电平 = 高电平220ns~380ns  低电平580ns~1us
******************************************************************/
void Ws2812b_WriteByte(unsigned char byte)
{
    int i = 0;
    
    // Add debug for every 10th byte to avoid flooding console
    static int byte_count = 0;
    if(byte_count % 10 == 0) {
        printf("DEBUG: Ws2812b_WriteByte - Sending byte 0x%02X (count: %d)\r\n", byte, byte_count);
    }
    byte_count++;
    
    // WS2812B时序要求（修正严格时序）：
    // 0码: 高电平0.4us±0.15us, 低电平0.85us±0.15us
    // 1码: 高电平0.8us±0.15us, 低电平0.45us±0.15us
    // 芯片重置: 低电平>50us
    
    // 禁用中断以确保精确时序
    __disable_irq();
    
    for(i = 0; i < 8; i++)
    {
        if(byte & (0x80 >> i)) // 发送1
        {
            RGB_PIN_H();
            
            // 高电平需要0.8us (800ns)，根据WS2812B规格
            // 增加NOP指令数量以延长高电平时间
            __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); 
            __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
            __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
            __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
            __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
            __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
            __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
            
            RGB_PIN_L(); 
            
            // 低电平需要0.45us (450ns)
            __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
            __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
            __NOP(); __NOP(); __NOP(); __NOP();
        }
        else // 发送0
        {
            RGB_PIN_H();
            
            // 高电平需要0.4us (400ns)
            __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); 
            __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
            __NOP(); __NOP(); __NOP(); __NOP();
            
            RGB_PIN_L();
            
            // 低电平需要0.85us (850ns)
            __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
            __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
            __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
            __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
            __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
            __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        }
    }
    
    // 恢复中断
    __enable_irq();
}
   
// ɫ
// original_color: 初始颜色值 (例如 0xRRGGBB)
// brightness_factor: 亮度因子 (0.0 到 1.0)
unsigned long apply_brightness(unsigned long original_color, float brightness_factor) {
    unsigned char r, g, b;
    
    // 从 original_color 提取 R, G, B 分量
    r = (original_color >> 16) & 0xFF;
    g = (original_color >> 8) & 0xFF;
    b = original_color & 0xFF;
    
    // 应用亮度因子调整 R, G, B 分量
    // 注意：如果需要转换为整数，确保转换后的值在0-255范围内
    r = (unsigned char)(r * brightness_factor);
    g = (unsigned char)(g * brightness_factor);
    b = (unsigned char)(b * brightness_factor);
    
    // 确保值在0-255范围内 (当然，如果 brightness_factor <= 1.0 一般不会超出范围)
    // r = (r > 255) ? 255 : r; // 可选的保守处理
    // g = (g > 255) ? 255 : g;
    // b = (b > 255) ? 255 : b;

    // 将 R, G, B 分量合并为最终的颜色值
    return ((unsigned long)r << 16) | ((unsigned long)g << 8) | b;
}

void delay_us(uint32_t _us)
{
    rt_hw_us_delay(_us);
}

// 添加一个简单的颜色测试函数
void WS2812_Test_Single_Color(unsigned long color) 
{
    printf("Testing single color: 0x%08lX\r\n", color);
    
    // 提取RGB分量
    unsigned char r = (color >> 16) & 0xFF;
    unsigned char g = (color >> 8) & 0xFF;
    unsigned char b = color & 0xFF;
    printf("RGB components: R=%d, G=%d, B=%d\r\n", r, g, b);
    
    // 清除所有LED
    for(int i = 0; i < WS2812_NUMBERS; i++) {
        LedsArray[i * 3] = 0;     // G
        LedsArray[i * 3 + 1] = 0; // R
        LedsArray[i * 3 + 2] = 0; // B
    }
    
    // 发送清除数据
    printf("Clearing all LEDs...\r\n");
    rgb_SendArray();
    delay_us(100000); // 100ms delay
    
    // 设置所有LED为指定颜色
    for(int i = 0; i < WS2812_NUMBERS; i++) {
        // WS2812使用GRB顺序
        LedsArray[i * 3] = g;     // G
        LedsArray[i * 3 + 1] = r; // R
        LedsArray[i * 3 + 2] = b; // B
        
        printf("LED %d: G=%d, R=%d, B=%d\r\n", i, g, r, b);
    }
    
    // 连续发送3次以确保稳定
    printf("Sending color data (1st attempt)...\r\n");
    rgb_SendArray();
    delay_us(100000); // 100ms delay
    
    printf("Sending color data (2nd attempt)...\r\n");
    rgb_SendArray();
    delay_us(100000); // 100ms delay
    
    printf("Sending color data (3rd attempt)...\r\n");
    rgb_SendArray();
    delay_us(100000); // 100ms delay
    
    printf("Single color test complete\r\n");
}

// 添加LED数据线测试功能，用于检测硬件连接性
void WS2812_Test_Data_Line(void)
{
    printf("Testing WS2812B data line connectivity...\n");
    
    // 先清除所有LED
    for(int i = 0; i < WS2812_NUMBERS; i++) {
        rgb_SetColor(i, BLACK);
    }
    rgb_SendArray();
    delay_us(100000); // 100ms delay
    
    printf("Sending test pulses to data line...\n");
    
    // 发送10个高电平脉冲，检测数据线能否正常工作
    for(int i = 0; i < 10; i++) {
        RGB_PIN_H();
        delay_us(100);
        RGB_PIN_L();
        delay_us(100);
    }
    
    // 发送重置信号
    delay_us(500);
    
    // 测试每个LED逐个点亮，然后全亮
    printf("Testing each LED individually...\n");
    
    // 先测试红色
    for(int i = 0; i < WS2812_NUMBERS; i++) {
        // 清除所有LED
        for(int j = 0; j < WS2812_NUMBERS; j++) {
            rgb_SetColor(j, BLACK);
        }
        // 只点亮当前LED
        rgb_SetColor(i, RED);
        rgb_SendArray();
        delay_us(300000); // 300ms
    }
    
    // 全部点亮为红色
    for(int i = 0; i < WS2812_NUMBERS; i++) {
        rgb_SetColor(i, RED);
    }
    rgb_SendArray();
    delay_us(500000); // 500ms
    
    printf("Data line test complete\n");
}

// 添加检测实际连接的WS2812B数量的函数
int WS2812_Detect_Count(void)
{
    printf("开始检测WS2812B灯珠...\n");
    
    // 清除所有灯
    for(int i = 0; i < WS2812_MAX; i++) {
        rgb_SetColor(i, BLACK);
    }
    rgb_SendArray();
    delay_us(100000); // 延时100ms
    
    // 逐个测试每个LED
    for(int i = 0; i < WS2812_MAX; i++) {
        // 清除所有LED
        for(int j = 0; j < WS2812_MAX; j++) {
            rgb_SetColor(j, BLACK);
        }
        
        // 设置当前LED为红色
        rgb_SetColor(i, RED);
        rgb_SendArray();
        
        printf("测试LED %d\n", i);
        delay_us(200000); // 延时200ms
    }
    
    // 所有LED交替显示红绿蓝白
    printf("测试所有LED交替显示\n");
    
    // 红色
    for(int i = 0; i < WS2812_MAX; i++) {
        rgb_SetColor(i, RED);
    }
    rgb_SendArray();
    delay_us(500000);
    
    // 绿色
    for(int i = 0; i < WS2812_MAX; i++) {
        rgb_SetColor(i, GREEN);
    }
    rgb_SendArray();
    delay_us(500000);
    
    // 蓝色
    for(int i = 0; i < WS2812_MAX; i++) {
        rgb_SetColor(i, BLUE);
    }
    rgb_SendArray();
    delay_us(500000);
    
    // 白色
    for(int i = 0; i < WS2812_MAX; i++) {
        rgb_SetColor(i, WHITE);
    }
    rgb_SendArray();
    delay_us(500000);
    
    // 关闭所有LED
    for(int i = 0; i < WS2812_MAX; i++) {
        rgb_SetColor(i, BLACK);
    }
    rgb_SendArray();
    
    printf("WS2812B灯珠测试完成，配置的数量: %d\n", WS2812_NUMBERS);
    return WS2812_NUMBERS; // 返回配置的灯珠数量
}

// 清理重复的WS2812_Test_Data_Line和WS2812_Detect_Count函数定义
// 保留第一个定义，删除后续的重复定义
// 结束前添加一个最终的重置函数
void WS2812_Final_Reset(void)
{
    printf("执行最终重置...\n");
    
    // 禁用中断
    __disable_irq();
    
    // 发送更长的重置信号
    RGB_PIN_L();
    delay_us(300); // 超过标准重置时间
    
    // 恢复中断
    __enable_irq();
    
    printf("重置完成\n");
}
     
