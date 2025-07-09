/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-05-06     RT-Thread    first version
 */

#include <rtthread.h>
 #include <rtdevice.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include "main.h"
#include "usart.h"
#include "gpio.h"
#include "tim.h"
#include "bsp_VL53L0X.h"
#include "wt61c_bsp.h"
#include "bsp_ws2812.h"
#include "usart2.h"
#include "syn6288_uart.h"
#include "syn6288.h"
 #include "processing.h"
 
 /* 线程优先级定义 */
 #define THREAD_PRIO_HAPTIC      5
 #define THREAD_PRIO_FALL_DETECT 6
 #define THREAD_PRIO_VOICE       15
 #define THREAD_PRIO_AI_DATA     16
 #define THREAD_PRIO_REMOTE_COMM 17
 #define THREAD_PRIO_LED         25
 
 /* 线程栈大小定义 (单位: 字节) */
 #define THREAD_STACK_SIZE_HAPTIC      1024
 #define THREAD_STACK_SIZE_FALL_DETECT 2048 
 #define THREAD_STACK_SIZE_VOICE       1024
 #define THREAD_STACK_SIZE_AI_DATA     1024
 #define THREAD_STACK_SIZE_REMOTE_COMM 1024
 #define THREAD_STACK_SIZE_LED         512
 
 /* 线程句柄定义 */
 static rt_thread_t haptic_thread = RT_NULL;
 static rt_thread_t fall_detect_thread = RT_NULL;
 static rt_thread_t voice_thread = RT_NULL;
 static rt_thread_t ai_data_thread = RT_NULL;
 static rt_thread_t remote_comm_thread = RT_NULL;
 static rt_thread_t led_thread = RT_NULL;
 
 /* 设备句柄定义 */
 static rt_device_t asrpro_uart_dev = RT_NULL;
 static rt_device_t k230_uart_dev = RT_NULL;
 static rt_device_t esp32_uart_dev = RT_NULL;
 
 /* 消息队列定义 */
 #define AI_MSG_QUEUE_SIZE 10
 static rt_mq_t ai_msg_queue = RT_NULL;
 
 /* 信号量定义 */
 static rt_sem_t fall_detected_sem = RT_NULL;
 
 /* 互斥量定义 */
 static rt_mutex_t vl53l0x_mutex = RT_NULL;
 
 /* 线程入口函数声明 */
 static void haptic_feedback_thread_entry(void *parameter);
 static void fall_detection_thread_entry(void *parameter);
 static void voice_command_thread_entry(void *parameter);
 static void ai_data_handler_thread_entry(void *parameter);
 static void remote_comm_thread_entry(void *parameter);
 static void led_control_thread_entry(void *parameter);
 
 /* 函数声明 */
 static void hw_init(void);
 static rt_err_t uart_rx_ind(rt_device_t dev, rt_size_t size);
 
 /**
  * 更新振动马达PWM占空比
  * @param motor_id 马达ID (0-4)
  * @param duty_cycle PWM占空比 (0-100)
  */
 void update_motor_pwm(int motor_id, int duty_cycle)
 {
     if (motor_id < 0 || motor_id >= 5)
         return;
     
     VibrationControl_t *vc = &vibration_controls[motor_id];
     
     // 限制占空比范围
     if (duty_cycle > 100)
         duty_cycle = 100;
     if (duty_cycle < 0)
         duty_cycle = 0;
     
     // 计算PWM值
     uint32_t pulse = duty_cycle;
     
     // 设置PWM占空比
     __HAL_TIM_SET_COMPARE(vc->htim, vc->channel, pulse);
 }
 
 /**
  * 读取VL53L0X传感器数据
  * @param sensor_id 传感器ID (0-4)
  * @return 距离值 (单位: mm)
  */
 int read_vl53l0x_data(int sensor_id)
 {
     int distance = 0;
     
     // 获取互斥锁，保护传感器访问
     rt_mutex_take(vl53l0x_mutex, RT_WAITING_FOREVER);
     
     // 根据传感器ID读取对应的传感器数据
     switch (sensor_id) {
         case 0:
             if (vl53l0x_data1.RangeStatus == 0) {
                 distance = vl53l0x_data1.RangeMilliMeter;
             }
             break;
         case 1:
             if (vl53l0x_data2.RangeStatus == 0) {
                 distance = vl53l0x_data2.RangeMilliMeter;
             }
             break;
         case 2:
             if (vl53l0x_data3.RangeStatus == 0) {
                 distance = vl53l0x_data3.RangeMilliMeter;
             }
             break;
         case 3:
             if (vl53l0x_data4.RangeStatus == 0) {
                 distance = vl53l0x_data4.RangeMilliMeter;
             }
             break;
         case 4:
             if (vl53l0x_data5.RangeStatus == 0) {
                 distance = vl53l0x_data5.RangeMilliMeter;
             }
             break;
         default:
             break;
     }
     
     // 释放互斥锁
     rt_mutex_release(vl53l0x_mutex);
     
     return distance;
 }
 
 /**
  * 读取IMU数据
  * @param acc 加速度数据数组 (3个元素)
  * @param gyro 陀螺仪数据数组 (3个元素)
  */
 void read_imu_data(float* acc, float* gyro)
 {
     // 获取WT61C IMU数据
     getWT61C();
     
     // 复制加速度数据
     acc[0] = fAcc[0];
     acc[1] = fAcc[1];
     acc[2] = fAcc[2];
     
     // 复制陀螺仪数据
     gyro[0] = fGyro[0];
     gyro[1] = fGyro[1];
     gyro[2] = fGyro[2];
 }
 
 /**
  * 运行摔倒检测算法
  * @param acc 加速度数据
  * @param gyro 陀螺仪数据
  * @return 1表示检测到摔倒，0表示未检测到
  */
 int run_fall_detection_algorithm(float* acc, float* gyro)
 {
     // 简单摔倒检测算法示例
     // 实际应用中应该使用更复杂的算法
     float acc_magnitude = sqrtf(acc[0]*acc[0] + acc[1]*acc[1] + acc[2]*acc[2]);
     float gyro_magnitude = sqrtf(gyro[0]*gyro[0] + gyro[1]*gyro[1] + gyro[2]*gyro[2]);
     
     // 当加速度和角速度同时超过阈值时，认为发生了摔倒
     if (acc_magnitude > 30.0f && gyro_magnitude > 300.0f) {
         return 1;
     }
     
     return 0;
 }
 
 /**
  * 触发紧急响应
  */
 void trigger_emergency_response(void)
 {
     static rt_tick_t last_trigger_time = 0;
     rt_tick_t now = rt_tick_get();
     
     // 防止频繁触发 (至少间隔5秒)
     if (now - last_trigger_time < RT_TICK_PER_SECOND * 5) {
         return;
     }
     
     last_trigger_time = now;
     
     // 发送信号量通知LED线程显示紧急模式
     rt_sem_release(fall_detected_sem);
     
     // 使用语音播报摔倒事件
     SYN_FrameInfo(2, "[v10][m0][t5]紧急情况！检测到摔倒事件！");
     
     // 发送短信通知
     Send_String(&huart1, sms2);
     
     rt_kprintf("紧急情况：检测到摔倒事件！\n");
 }
 
 /**
  * 硬件初始化
  */
 static void hw_init(void)
 {
     // 初始化HAL库
     HAL_Init();
     
     // 初始化GPIO
     MX_GPIO_Init();
     
     // 初始化UART
     MX_USART1_UART_Init();
     MX_USART2_UART_Init();
     MX_UART4_Init();
     
     // 初始化定时器
     MX_TIM10_Init();
     MX_TIM4_Init();
     MX_TIM3_Init();
     
     // 启动PWM输出
     HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
     HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
     HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
     HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
     HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
     
     // 初始化VL53L0X传感器
     VL53L0X_Init();
     
     // 初始化WT61C IMU
     WT61C_Init();
     
     // 初始化中断接收
     HAL_UART_Receive_IT(&huart2, &ucTemp, 1);
     HAL_UART_Receive_IT(&huart4, &ucTemp4, 1);
     HAL_UART_Receive_IT(&huart1, &ucTemp1, 1);
     USART1_RX_STA = 0;
                   HAL_TIM_Base_Stop(&htim10);
     
     rt_kprintf("硬件初始化完成\n");
 }
 
 /**
  * UART接收回调函数
  */
 static rt_err_t uart_rx_ind(rt_device_t dev, rt_size_t size)
 {
     // 在中断上下文中，不应做过多处理，仅发送信号量或消息通知线程即可
     // 此处为了简化，我们在线程中轮询读取
     return RT_EOK;
 }
 
 /**
  * 触觉反馈线程入口函数
  */
 static void haptic_feedback_thread_entry(void *parameter)
 {
     rt_kprintf("触觉反馈线程已启动\n");
     
     while (1)
     {
         // 1. 周期性读取5个VL53L0X传感器数据
         int sensor_data[5];
         int pwm_values[5];
         
         // 获取互斥锁，保护传感器访问
         rt_mutex_take(vl53l0x_mutex, RT_WAITING_FOREVER);
         
         // 更新所有传感器的数据
         VL53L0X_GetValue();
         
         // 读取传感器数据
         for (int i = 0; i < 5; i++)
         {
             sensor_data[i] = read_vl53l0x_data(i);
         }
         
         // 释放互斥锁
         rt_mutex_release(vl53l0x_mutex);
         
         // 2. 根据传感器数据计算PWM占空比
         for (int i = 0; i < 5; i++)
         {
             if (sensor_data[i] > 0)
             {
                 // 距离越近，振动越强
                 if (sensor_data[i] < MIN_DISTANCE)
                 {
                     pwm_values[i] = MAX_VIBRATION;
                 }
                 else if (sensor_data[i] > MAX_DISTANCE)
                 {
                     pwm_values[i] = MIN_VIBRATION;
                 }
                 else
                 {
                     // 线性映射距离到振动强度
                     pwm_values[i] = MAX_VIBRATION - (sensor_data[i] - MIN_DISTANCE) * (MAX_VIBRATION - MIN_VIBRATION) / (MAX_DISTANCE - MIN_DISTANCE);
                 }
             }
             else
             {
                 // 无效数据，不振动
                 pwm_values[i] = 0;
             }
         }
         
         // 3. 实时更新5个振动马达的PWM
         for (int i = 0; i < 5; i++)
         {
             update_motor_pwm(i, pwm_values[i]);
         }
         
         // 设置线程执行周期，例如50ms
         rt_thread_mdelay(50);
     }
 }
 
 /**
  * 摔倒检测线程入口函数
  */
 static void fall_detection_thread_entry(void *parameter)
 {
     float acc[3], gyro[3];
     rt_kprintf("摔倒检测线程已启动\n");
     
     while (1)
     {
         // 1. 实时读取WT61C IMU数据
         read_imu_data(acc, gyro);
         
         // 2. 运行摔倒检测算法
         if (run_fall_detection_algorithm(acc, gyro))
         {
             // 3. 触发紧急响应
             trigger_emergency_response();
         }
         
         // 设置检测频率，例如10ms一次
         rt_thread_mdelay(10);
     }
 }
 
 /**
  * 语音命令线程入口函数
  */
 static void voice_command_thread_entry(void *parameter)
 {
     char buffer[64];
     rt_size_t rx_len;
     rt_kprintf("语音命令线程已启动\n");
     
     // 打开UART设备
     asrpro_uart_dev = rt_device_find("uart3");
     if (asrpro_uart_dev == RT_NULL)
     {
         rt_kprintf("找不到UART3设备\n");
         return;
     }
     
     // 配置设备
     struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
     rt_device_control(asrpro_uart_dev, RT_DEVICE_CTRL_CONFIG, &config);
     
     // 设置接收回调
     rt_device_set_rx_indicate(asrpro_uart_dev, uart_rx_ind);
     
     // 打开设备
     rt_device_open(asrpro_uart_dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
     
     while (1)
     {
         // 从UART设备读取数据，设置超时时间
         rx_len = rt_device_read(asrpro_uart_dev, 0, buffer, sizeof(buffer) - 1);
         if (rx_len > 0)
         {
             buffer[rx_len] = '\0';
             rt_kprintf("接收到语音命令: %s\n", buffer);
             
             // 解析命令并执行相应操作
             if (strstr(buffer, "开灯") != RT_NULL)
             {
                 light_mode = 1;
                 rt_kprintf("执行命令: 开灯\n");
             }
             else if (strstr(buffer, "关灯") != RT_NULL)
             {
                 light_mode = 0;
                 rt_kprintf("执行命令: 关灯\n");
             }
             else if (strstr(buffer, "求助") != RT_NULL)
             {
                 askhelp_mode = 1;
                 rt_kprintf("执行命令: 求助\n");
                 Send_String(&huart1, sms);
             }
             else if (strstr(buffer, "导航") != RT_NULL)
             {
                 vl53l0_mode = 1;
                 rt_kprintf("执行命令: 导航\n");
             }
             else if (strstr(buffer, "停止") != RT_NULL)
             {
                 vl53l0_mode = 0;
                 rt_kprintf("执行命令: 停止导航\n");
             }
         }
         
         // 短暂休眠，让出CPU
         rt_thread_mdelay(20);
     }
 }
 
 /**
  * AI数据处理线程入口函数
  */
 static void ai_data_handler_thread_entry(void *parameter)
 {
     char msg_buffer[128];
     rt_kprintf("AI数据处理线程已启动\n");
     
     // 打开UART设备
     k230_uart_dev = rt_device_find("uart4");
     if (k230_uart_dev == RT_NULL)
     {
         rt_kprintf("找不到UART4设备\n");
         return;
     }
     
     // 配置设备
     struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
     rt_device_control(k230_uart_dev, RT_DEVICE_CTRL_CONFIG, &config);
     
     // 设置接收回调
     rt_device_set_rx_indicate(k230_uart_dev, uart_rx_ind);
     
     // 打开设备
     rt_device_open(k230_uart_dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
     
     while (1)
     {
         // 检查是否有新的AI识别结果
         if (message_received)
         {
             message_received = 0;
             rt_kprintf("接收到AI识别结果: %s\n", rx_buffer);
             
             // 构建语音播报消息
             rt_snprintf(msg_buffer, sizeof(msg_buffer), "%s%s", prefix, rx_buffer);
             
             // 将消息放入队列，等待语音播报
             rt_mq_send(ai_msg_queue, msg_buffer, strlen(msg_buffer) + 1);
             
             // 直接播报识别结果
             SYN_FrameInfo(2, msg_buffer);
         }
         
         // 短暂休眠，让出CPU
         rt_thread_mdelay(50);
     }
 }
 
 /**
  * 远程通信线程入口函数
  */
 static void remote_comm_thread_entry(void *parameter)
 {
     char buffer[64];
     rt_size_t rx_len;
     rt_kprintf("远程通信线程已启动\n");
     
     // 打开UART设备
     esp32_uart_dev = rt_device_find("uart5");
     if (esp32_uart_dev == RT_NULL)
     {
         rt_kprintf("找不到UART5设备\n");
         return;
     }
     
     // 配置设备
     struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
     rt_device_control(esp32_uart_dev, RT_DEVICE_CTRL_CONFIG, &config);
     
     // 设置接收回调
     rt_device_set_rx_indicate(esp32_uart_dev, uart_rx_ind);
     
     // 打开设备
     rt_device_open(esp32_uart_dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
     
     while (1)
     {
         // 从UART设备读取数据，设置超时时间
         rx_len = rt_device_read(esp32_uart_dev, 0, buffer, sizeof(buffer) - 1);
         if (rx_len > 0)
         {
             buffer[rx_len] = '\0';
             rt_kprintf("接收到远程命令: %s\n", buffer);
             
             // 处理舵机控制命令
             if (strstr(buffer, "servo") != RT_NULL)
             {
                 // 解析舵机控制命令
                 int servo_id, angle;
                 if (sscanf(buffer, "servo %d %d", &servo_id, &angle) == 2)
                 {
                     rt_kprintf("控制舵机 %d 角度 %d\n", servo_id, angle);
                     
                     // 根据舵机ID控制不同的舵机
                     if (servo_id == 1)
                     {
                         shangduoji = angle;
                     }
                     else if (servo_id == 2)
                     {
                         xiaduoji = angle;
                     }
                     
                     duojirun_flag = 1;
                 }
             }
         }
         
         // 短暂休眠，让出CPU
         rt_thread_mdelay(50);
     }
 }
 
 /**
  * LED控制线程入口函数
  */
 static void led_control_thread_entry(void *parameter)
 {
     uint8_t led_state = 0;
     uint8_t emergency_mode = 0;
     rt_kprintf("LED控制线程已启动\n");
     
     while (1)
     {
         // 检查是否有摔倒事件
         if (rt_sem_trytake(fall_detected_sem) == RT_EOK)
         {
             emergency_mode = 1;
             rt_kprintf("进入紧急LED模式\n");
         }
         
         // 根据当前系统模式控制LED灯带
         if (emergency_mode)
         {
             // 紧急模式：红色闪烁
             led_state = !led_state;
             if (led_state)
             {
                 for (int i = 0; i < 10; i++)
                 {
                     RGB_SetColor(i, RED);
                 }
             }
             else
             {
                 for (int i = 0; i < 10; i++)
                 {
                     RGB_SetColor(i, BLACK);
                 }
             }
             RGB_Refresh();
             
             // 紧急模式下闪烁更快
             rt_thread_mdelay(200);
         }
         else if (askhelp_mode)
         {
             // 求助模式：蓝色闪烁
             led_state = !led_state;
             if (led_state)
             {
                 for (int i = 0; i < 10; i++)
                 {
                     RGB_SetColor(i, BLUE);
                 }
             }
             else
             {
                 for (int i = 0; i < 10; i++)
                 {
                     RGB_SetColor(i, BLACK);
                 }
             }
             RGB_Refresh();
             
             rt_thread_mdelay(500);
         }
         else if (light_mode)
         {
             // 照明模式：白色常亮
             for (int i = 0; i < 10; i++)
             {
                 RGB_SetColor(i, WHITE);
             }
             RGB_Refresh();
             
             rt_thread_mdelay(1000);
         }
         else
         {
             // 正常模式：绿色呼吸灯效果
             static float brightness = 0.0f;
             static float step = 0.05f;
             
             brightness += step;
             if (brightness >= 1.0f || brightness <= 0.0f)
             {
                 step = -step;
             }
             
             for (int i = 0; i < 10; i++)
             {
                 uint8_t r = (uint8_t)(0 * brightness * desired_brightness * 255);
                 uint8_t g = (uint8_t)(1 * brightness * desired_brightness * 255);
                 uint8_t b = (uint8_t)(0 * brightness * desired_brightness * 255);
                 RGB_SetColor(i, r << 16 | g << 8 | b);
             }
             RGB_Refresh();
             
             rt_thread_mdelay(50);
         }
    }
}

/**
  * 主函数
  */
 int main(void)
 {
     // 硬件初始化
     hw_init();
     
     // 创建互斥量
     vl53l0x_mutex = rt_mutex_create("vl53l0x", RT_IPC_FLAG_FIFO);
     
     // 创建信号量
     fall_detected_sem = rt_sem_create("fall", 0, RT_IPC_FLAG_FIFO);
     
     // 创建消息队列
     ai_msg_queue = rt_mq_create("ai_msg", 128, AI_MSG_QUEUE_SIZE, RT_IPC_FLAG_FIFO);
     
     // 创建触觉反馈线程
     haptic_thread = rt_thread_create("haptic",
                                     haptic_feedback_thread_entry,
                                     RT_NULL,
                                     THREAD_STACK_SIZE_HAPTIC,
                                     THREAD_PRIO_HAPTIC,
                                     20);
     if (haptic_thread != RT_NULL)
     {
         rt_thread_startup(haptic_thread);
     }
     
     // 创建摔倒检测线程
     fall_detect_thread = rt_thread_create("fall",
                                          fall_detection_thread_entry,
                                          RT_NULL,
                                          THREAD_STACK_SIZE_FALL_DETECT,
                                          THREAD_PRIO_FALL_DETECT,
                                          20);
     if (fall_detect_thread != RT_NULL)
     {
         rt_thread_startup(fall_detect_thread);
     }
     
     // 创建语音命令线程
     voice_thread = rt_thread_create("voice",
                                    voice_command_thread_entry,
                                    RT_NULL,
                                    THREAD_STACK_SIZE_VOICE,
                                    THREAD_PRIO_VOICE,
                                    20);
     if (voice_thread != RT_NULL)
     {
         rt_thread_startup(voice_thread);
     }
     
     // 创建AI数据处理线程
     ai_data_thread = rt_thread_create("ai_data",
                                      ai_data_handler_thread_entry,
                                      RT_NULL,
                                      THREAD_STACK_SIZE_AI_DATA,
                                      THREAD_PRIO_AI_DATA,
                                      20);
     if (ai_data_thread != RT_NULL)
     {
         rt_thread_startup(ai_data_thread);
     }
     
     // 创建远程通信线程
     remote_comm_thread = rt_thread_create("remote",
                                          remote_comm_thread_entry,
                                          RT_NULL,
                                          THREAD_STACK_SIZE_REMOTE_COMM,
                                          THREAD_PRIO_REMOTE_COMM,
                                          20);
     if (remote_comm_thread != RT_NULL)
     {
         rt_thread_startup(remote_comm_thread);
     }
     
     // 创建LED控制线程
     led_thread = rt_thread_create("led",
                                  led_control_thread_entry,
                                  RT_NULL,
                                  THREAD_STACK_SIZE_LED,
                                  THREAD_PRIO_LED,
                                  20);
     if (led_thread != RT_NULL)
     {
         rt_thread_startup(led_thread);
     }
     
     rt_kprintf("所有线程已创建并启动\n");
     
     return RT_EOK;
 }