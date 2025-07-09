#include "bsp_vl53l0x.h"


VL53L0X_Dev_t vl53l0x_dev1;//传感器1 I2C数据结构
VL53L0X_Dev_t vl53l0x_dev2;//传感器2 I2C数据结构
VL53L0X_Dev_t vl53l0x_dev3;//传感器3 I2C数据结构
VL53L0X_Dev_t vl53l0x_dev4;//传感器4 I2C数据结构
VL53L0X_Dev_t vl53l0x_dev5;//传感器5 I2C数据结构

VL53L0X_DeviceInfo_t vl53l0x_dev_info1;//传感器1 ID版本信息
VL53L0X_DeviceInfo_t vl53l0x_dev_info2;//传感器2 ID版本信息
VL53L0X_DeviceInfo_t vl53l0x_dev_info3;//传感器3 ID版本信息
VL53L0X_DeviceInfo_t vl53l0x_dev_info4;//传感器4 ID版本信息
VL53L0X_DeviceInfo_t vl53l0x_dev_info5;//传感器5 ID版本信息

uint8_t AjustOK=0;//校准标志位

//VL53L0X传感器模式参数
//0：默认;1:高精度;2:长距离;3:高速
mode_data Mode_data[]=
{
    {(FixPoint1616_t)(0.25*65536), 
	 (FixPoint1616_t)(18*65536),
	 33000,
	 14,
	 10},//默认
		
	{(FixPoint1616_t)(0.25*65536) ,
	 (FixPoint1616_t)(18*65536),
	 200000, 
	 14,
	 10},//高精度
		
    {(FixPoint1616_t)(0.1*65536) ,
	 (FixPoint1616_t)(60*65536),
	 33000,
	 18,
	 14},//长距离
	
    {(FixPoint1616_t)(0.25*65536) ,
	 (FixPoint1616_t)(32*65536),
	 20000,
	 14,
	 10},//高速
		
};

//API错误信息打印
//Status：详情查看VL53L0X_Error参数的定义
void print_pal_error(VL53L0X_Error Status)
{
	
	char buf[VL53L0X_MAX_STRING_LENGTH];
	
	VL53L0X_GetPalErrorString(Status,buf);//根据Status状态获取错误信息字符串
	
    printf("API Status: %i : %s\r\n",Status, buf);//打印状态和错误信息
}


//设置VL53L0X设备I2C地址
//dev:设备I2C参数结构体
//newaddr:设备新的I2C地址
VL53L0X_Error vl53l0x_Addr_set(VL53L0X_Dev_t *dev, uint8_t newaddr) {
    uint16_t Id;
    uint8_t FinalAddress;
    VL53L0X_Error Status = VL53L0X_ERROR_NONE;
    uint8_t original_DevAddr = dev->I2cDevAddr; // 保存一下原始结构的地址，虽然没用，但可以方便一下
    uint8_t current_CommAddr; // 当前实际用于通信的地址

    FinalAddress = newaddr;
    printf("vl53l0x_Addr_set: TargetNewAddr=0x%02X for dev struct @ %p\n",
           FinalAddress, dev);

    if (FinalAddress == original_DevAddr && original_DevAddr != 0x52) { // 如果请求的地址和dev结构体中存放的新地址相同，且不是默认地址
        printf("  New address 0x%02X is same as already stored in dev struct. Verifying communication...\n", FinalAddress);
        current_CommAddr = FinalAddress;
        Status = VL53L0X_RdWord(dev, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &Id); // 使用dev结构中的地址进行通信
        printf("  Read ID using addr 0x%02X: Status=%d, ID=0x%04X\n", current_CommAddr, Status, Id);
        if (Status == VL53L0X_ERROR_NONE && Id == 0xEEAA) {
            printf("  Communication verified with addr 0x%02X. No change needed.\n", current_CommAddr);
            return VL53L0X_ERROR_NONE;
        } else {
            printf("  Verification with addr 0x%02X FAILED. Will proceed assuming module is at default 0x52.\n", current_CommAddr);
            // 继续下面的流程，假设是0x52去操作
        }
    }

    // 此时通过XSHUT控制，目标模块处于默认地址0x52上工作
    current_CommAddr = 0x52;
    dev->I2cDevAddr = current_CommAddr; // 暂时将dev结构体中的地址设为0x52，以便API使用这个地址通信
    printf("  Setting dev->I2cDevAddr to 0x%02X for initial communication.\n", current_CommAddr);

    Status = VL53L0X_WrByte(dev, 0x88, 0x00); // Set I2C standard mode
    printf("  Set I2C standard mode using addr 0x%02X: Status=%d\n", current_CommAddr, Status);
    if (Status != VL53L0X_ERROR_NONE) {
        dev->I2cDevAddr = original_DevAddr; // 恢复dev结构体中的地址
        goto set_error_exit;
    }

    Status = VL53L0X_RdWord(dev, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &Id);
    printf("  Read ID using addr 0x%02X: Status=%d, ID=0x%04X\n", current_CommAddr, Status, Id);
    if (Status != VL53L0X_ERROR_NONE) {
        dev->I2cDevAddr = original_DevAddr; // 恢复
        goto set_error_exit;
    }

    if (Id == 0xEEAA) {
        printf("  Device ID 0xEEAA matched on addr 0x%02X. Setting new address to 0x%02X.\n", current_CommAddr, FinalAddress);
        Status = VL53L0X_SetDeviceAddress(dev, FinalAddress); // API内部会使用 dev->I2cDevAddr (即0x52) 去发送命令
        printf("  VL53L0X_SetDeviceAddress to 0x%02X (using 0x%02X to send cmd): Status=%d\n", FinalAddress, current_CommAddr, Status);
        if (Status != VL53L0X_ERROR_NONE) {
            dev->I2cDevAddr = original_DevAddr; // 恢复
            goto set_error_exit;
        }

        // 地址设置命令已发送，现在更新dev结构体的地址并验证新地址
        dev->I2cDevAddr = FinalAddress;
        printf("  Set dev->I2cDevAddr to new addr 0x%02X. Verifying...\n", dev->I2cDevAddr);
        HAL_Delay(10); // 给模块一点时间响应新地址
        Status = VL53L0X_RdWord(dev, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &Id); // 调用API会用 dev->I2cDevAddr (新地址)
        printf("  Read ID using new addr 0x%02X: Status=%d, ID=0x%04X\n", dev->I2cDevAddr, Status, Id);
        if (Status != VL53L0X_ERROR_NONE || Id != 0xEEAA) {
            printf("  Verification with new addr 0x%02X FAILED!\n", dev->I2cDevAddr);
            // 如果验证失败，dev->I2cDevAddr 依然是新的错误地址，这可能会导致后续使用这个dev结构体时出错
            // 所以考虑是否要恢复成 original_DevAddr 或者 0x52，视情况而定如何处理
            goto set_error_exit;
        }
        printf("  Verification with new addr 0x%02X successful.\n", dev->I2cDevAddr);
    } else {
        printf("  Device ID mismatch on addr 0x%02X. Expected 0xEEAA, got 0x%04X.\n", current_CommAddr, Id);
        Status = VL53L0X_ERROR_CONTROL_INTERFACE; // 或者更具体的错误
        dev->I2cDevAddr = original_DevAddr; // 恢复
        goto set_error_exit;
    }

set_error_exit:
    if (Status != VL53L0X_ERROR_NONE) {
        printf("vl53l0x_Addr_set for targetNewAddr 0x%02X FAILED! Final Status: %d\n", FinalAddress, Status);
        print_pal_error(Status);
    } else {
        printf("vl53l0x_Addr_set for targetNewAddr 0x%02X SUCCESSFUL. dev->I2cDevAddr is now 0x%02X\n", FinalAddress, dev->I2cDevAddr);
    }
    return Status;
}

//vl53l0x复位函数
//dev:设备I2C参数结构体
void vl53l0x_reset(VL53L0X_Dev_t *dev)
{
	uint8_t addr;
	addr = dev->I2cDevAddr;//保存设备原I2C地址
    
    // 根据设备指针确定使用哪个XSHUT引脚
    if (dev == &vl53l0x_dev1) {
        VL53L0X_Xshut1 = 0;
        HAL_Delay(30);
        VL53L0X_Xshut1 = 1;
    } else if (dev == &vl53l0x_dev2) {
        VL53L0X_Xshut2 = 0;
        HAL_Delay(30);
        VL53L0X_Xshut2 = 1;
    } else if (dev == &vl53l0x_dev3) {
        VL53L0X_Xshut3 = 0;
        HAL_Delay(30);
        VL53L0X_Xshut3 = 1;
    } else if (dev == &vl53l0x_dev4) {
        VL53L0X_Xshut4 = 0;
        HAL_Delay(30);
        VL53L0X_Xshut4 = 1;
    } else if (dev == &vl53l0x_dev5) {
        VL53L0X_Xshut5 = 0;
        HAL_Delay(30);
        VL53L0X_Xshut5 = 1;
    }
    
	HAL_Delay(30);	
	dev->I2cDevAddr = 0x52;
	vl53l0x_Addr_set(dev, addr);//设置VL53L0X传感器原来的I2C地址
	VL53L0X_DataInit(dev);	
}

//初始化vl53l0x传感器1
//dev:设备I2C参数结构体
VL53L0X_Error vl53l0x_init1(VL53L0X_Dev_t *dev)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	VL53L0X_Dev_t *pMyDevice = dev;
	
	pMyDevice->I2cDevAddr = VL53L0X_DEFAULT_ADDR;//I2C地址(上电默认0x52)
	pMyDevice->comms_type = 1;           //I2C通信模式
	pMyDevice->comms_speed_khz = 400;    //I2C通信速率
	
	VL53L0X_i2c_init();//初始化IIC总线
	
	// 传感器1的XSHUT引脚已在main.c中控制
	
    vl53l0x_Addr_set(pMyDevice, VL53L0X_ADDR1);//设置VL53L0X传感器I2C地址
    if(Status != VL53L0X_ERROR_NONE) goto error;
	Status = VL53L0X_DataInit(pMyDevice);//设备初始化
	if(Status != VL53L0X_ERROR_NONE) goto error;
	HAL_Delay(2);
	Status = VL53L0X_GetDeviceInfo(pMyDevice, &vl53l0x_dev_info1);//获取设备ID信息
    if(Status != VL53L0X_ERROR_NONE) goto error;
	
	error:
	if(Status != VL53L0X_ERROR_NONE)
	{
		print_pal_error(Status);//打印错误信息
		return Status;
	}
  	
	return Status;
}

//初始化vl53l0x传感器2
VL53L0X_Error vl53l0x_init2(VL53L0X_Dev_t *dev)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	VL53L0X_Dev_t *pMyDevice = dev;
	
	pMyDevice->I2cDevAddr = VL53L0X_DEFAULT_ADDR;//I2C地址(上电默认0x52)
	pMyDevice->comms_type = 1;           //I2C通信模式
	pMyDevice->comms_speed_khz = 400;    //I2C通信速率
	
	// 传感器2的XSHUT引脚已在main.c中控制
	
    vl53l0x_Addr_set(pMyDevice, VL53L0X_ADDR2);//设置VL53L0X传感器I2C地址
    if(Status != VL53L0X_ERROR_NONE) goto error;
	Status = VL53L0X_DataInit(pMyDevice);//设备初始化
	if(Status != VL53L0X_ERROR_NONE) goto error;
	HAL_Delay(2);
	Status = VL53L0X_GetDeviceInfo(pMyDevice, &vl53l0x_dev_info2);//获取设备ID信息
    if(Status != VL53L0X_ERROR_NONE) goto error;
	
	error:
	if(Status != VL53L0X_ERROR_NONE)
	{
		print_pal_error(Status);//打印错误信息
		return Status;
	}
  	
	return Status;
}

//初始化vl53l0x传感器3
VL53L0X_Error vl53l0x_init3(VL53L0X_Dev_t *dev)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	VL53L0X_Dev_t *pMyDevice = dev;
	
	pMyDevice->I2cDevAddr = VL53L0X_DEFAULT_ADDR;//I2C地址(上电默认0x52)
	pMyDevice->comms_type = 1;           //I2C通信模式
	pMyDevice->comms_speed_khz = 400;    //I2C通信速率
	
	// 传感器3的XSHUT引脚已在main.c中控制
	
    vl53l0x_Addr_set(pMyDevice, VL53L0X_ADDR3);//设置VL53L0X传感器I2C地址
    if(Status != VL53L0X_ERROR_NONE) goto error;
	Status = VL53L0X_DataInit(pMyDevice);//设备初始化
	if(Status != VL53L0X_ERROR_NONE) goto error;
	HAL_Delay(2);
	Status = VL53L0X_GetDeviceInfo(pMyDevice, &vl53l0x_dev_info3);//获取设备ID信息
    if(Status != VL53L0X_ERROR_NONE) goto error;
	
	error:
	if(Status != VL53L0X_ERROR_NONE)
	{
		print_pal_error(Status);//打印错误信息
		return Status;
	}
  	
	return Status;
}

//初始化vl53l0x传感器4
VL53L0X_Error vl53l0x_init4(VL53L0X_Dev_t *dev)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	VL53L0X_Dev_t *pMyDevice = dev;
	
	pMyDevice->I2cDevAddr = VL53L0X_DEFAULT_ADDR;//I2C地址(上电默认0x52)
	pMyDevice->comms_type = 1;           //I2C通信模式
	pMyDevice->comms_speed_khz = 400;    //I2C通信速率
	
	// 传感器4的XSHUT引脚已在main.c中控制
	
    vl53l0x_Addr_set(pMyDevice, VL53L0X_ADDR4);//设置VL53L0X传感器I2C地址
    if(Status != VL53L0X_ERROR_NONE) goto error;
	Status = VL53L0X_DataInit(pMyDevice);//设备初始化
	if(Status != VL53L0X_ERROR_NONE) goto error;
	HAL_Delay(2);
	Status = VL53L0X_GetDeviceInfo(pMyDevice, &vl53l0x_dev_info4);//获取设备ID信息
    if(Status != VL53L0X_ERROR_NONE) goto error;
	
	error:
	if(Status != VL53L0X_ERROR_NONE)
	{
		print_pal_error(Status);//打印错误信息
		return Status;
	}
  	
	return Status;
}

//初始化vl53l0x传感器5
VL53L0X_Error vl53l0x_init5(VL53L0X_Dev_t *dev)
{
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	VL53L0X_Dev_t *pMyDevice = dev;
	
	pMyDevice->I2cDevAddr = VL53L0X_DEFAULT_ADDR;//I2C地址(上电默认0x52)
	pMyDevice->comms_type = 1;           //I2C通信模式
	pMyDevice->comms_speed_khz = 400;    //I2C通信速率
	
	// 传感器5的XSHUT引脚已在main.c中控制
	
    vl53l0x_Addr_set(pMyDevice, VL53L0X_ADDR5);//设置VL53L0X传感器I2C地址
    if(Status != VL53L0X_ERROR_NONE) goto error;
	Status = VL53L0X_DataInit(pMyDevice);//设备初始化
	if(Status != VL53L0X_ERROR_NONE) goto error;
	HAL_Delay(2);
	Status = VL53L0X_GetDeviceInfo(pMyDevice, &vl53l0x_dev_info5);//获取设备ID信息
    if(Status != VL53L0X_ERROR_NONE) goto error;
	
	error:
	if(Status != VL53L0X_ERROR_NONE)
	{
		print_pal_error(Status);//打印错误信息
		return Status;
	}
  	
	return Status;
}
