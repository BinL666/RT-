#include "vl53l0x_i2c.h"
#include "stm32f4xx.h"


//VL53L0X I2C��ʼ��
void VL53L0X_i2c_init(void)
{
    // ��ʼ�� SDA �� SCL Ϊ���������Ĭ�ϸߵ�ƽ��
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = GPIO_SDA | GPIO_SCL;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    
    HAL_GPIO_Init(PORT_VL53L0x, &GPIO_InitStruct);
    
    // ���ó�ʼ״̬��I2C ����ʱ SDA �� SCL Ϊ�ߵ�ƽ��
    HAL_GPIO_WritePin(PORT_VL53L0x, GPIO_SDA, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PORT_VL53L0x, GPIO_SCL, GPIO_PIN_SET);
}

/******************************************************************
 * �� �� �� �ƣ�IIC_Start
 * �� �� ˵ ����IIC��ʼʱ��
 * �� �� �� �Σ���
 * �� �� �� �أ���
 * ��       �ߣ�LC
 * ��       ע����
******************************************************************/
void IIC_Start(void)
{
	SDA_OUT();
	
	SDA(1);
	delay_us(5);
	SCL(1); 
	delay_us(5);
	
	SDA(0);
	delay_us(5);
	SCL(0);
	delay_us(5);
	               
}
/******************************************************************
 * �� �� �� �ƣ�IIC_Stop
 * �� �� ˵ ����IICֹͣ�ź�
 * �� �� �� �Σ���
 * �� �� �� �أ���
 * ��       �ߣ�LC
 * ��       ע����
******************************************************************/
void IIC_Stop(void)
{
	SDA_OUT();
	SCL(0);
	SDA(0);
	
	SCL(1);
	delay_us(5);
	SDA(1);
	delay_us(5);
	
}

/******************************************************************
 * �� �� �� �ƣ�IIC_Send_Ack
 * �� �� ˵ ������������Ӧ����߷�Ӧ���ź�
 * �� �� �� �Σ�0����Ӧ��  1���ͷ�Ӧ��
 * �� �� �� �أ���
 * ��       �ߣ�LC
 * ��       ע����
******************************************************************/
void IIC_Send_Ack(unsigned char ack)
{
	SDA_OUT();
	SCL(0);
	SDA(0);
	delay_us(5);
	if(!ack) SDA(0);
	else	 SDA(1);
	SCL(1);
	delay_us(5);
	SCL(0);
	SDA(1);
}


/******************************************************************
 * �� �� �� �ƣ�I2C_WaitAck
 * �� �� ˵ �����ȴ��ӻ�Ӧ��
 * �� �� �� �Σ���
 * �� �� �� �أ�0��Ӧ��  1��ʱ��Ӧ��
 * ��       �ߣ�LC
 * ��       ע����
******************************************************************/
unsigned char I2C_WaitAck(void)
{
	
	char ack = 0;
	unsigned char ack_flag = 10;
	SCL(0);
	SDA(1);
	SDA_IN();
	delay_us(5);
	SCL(1);
    delay_us(5);

	while( (SDA_GET()==1) && ( ack_flag ) )
	{
		ack_flag--;
		delay_us(5);
	}
	
	if( ack_flag <= 0 )
	{
		IIC_Stop();
		return 1;
	}
	else
	{
		SCL(0);
		SDA_OUT();
	}
	return ack;
}

/******************************************************************
 * �� �� �� �ƣ�Send_Byte
 * �� �� ˵ ����д��һ���ֽ�
 * �� �� �� �Σ�datҪд�˵�����
 * �� �� �� �أ���
 * ��       �ߣ�LC
 * ��       ע����
******************************************************************/
void Send_Byte(uint8_t dat)
{
	int i = 0;
	SDA_OUT();
	SCL(0);//����ʱ�ӿ�ʼ���ݴ���
	
	for( i = 0; i < 8; i++ )
	{
		SDA( (dat & 0x80) >> 7 );
		delay_us(1);
		SCL(1);
		delay_us(5);
		SCL(0);
		delay_us(5);
		dat<<=1;
	}	
}

/******************************************************************
 * �� �� �� �ƣ�Read_Byte
 * �� �� ˵ ����IIC��ʱ��
 * �� �� �� �Σ�ack=1ʱ������ACK��ack=0������nACK   
 * �� �� �� �أ�����������
 * ��       �ߣ�LC
 * ��       ע����
******************************************************************/
unsigned char Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA����Ϊ����
	for(i=0;i<8;i++ )
	{
		 SCL(0);
		delay_us(4);
        SCL(1);
		receive<<=1;
		if( SDA_GET() )receive++;   
	  delay_us(4); //1
	}					 
	if (!ack)
		IIC_Send_Ack(1);//����nACK
	else
		IIC_Send_Ack(0); //����ACK   
	return receive;
}


//IICдһ���ֽ�����
u8 VL_IIC_Write_1Byte(u8 SlaveAddress, u8 REG_Address,u8 REG_data)
{
	IIC_Start();
	Send_Byte(SlaveAddress);
	if(I2C_WaitAck())
	{
		IIC_Stop();//�ͷ�����
		return 1;//ûӦ�����˳�

	}
	Send_Byte(REG_Address);
	I2C_WaitAck();	
	Send_Byte(REG_data);
	I2C_WaitAck();	
	IIC_Stop();

	return 0;
}

//IIC��һ���ֽ�����
u8 VL_IIC_Read_1Byte(u8 SlaveAddress, u8 REG_Address,u8 *REG_data)
{
	IIC_Start();
	Send_Byte(SlaveAddress);//��д����
	if(I2C_WaitAck())
	{
		 IIC_Stop();//�ͷ�����
		 return 1;//ûӦ�����˳�
	}		
	Send_Byte(REG_Address);
	I2C_WaitAck();
	IIC_Start(); 
	Send_Byte(SlaveAddress|0x01);//��������
	I2C_WaitAck();
	*REG_data = Read_Byte(0);
	IIC_Stop();

	return 0;
}

//IICдn�ֽ�����
u8 VL_IIC_Write_nByte(u8 SlaveAddress, u8 REG_Address,u16 len, u8 *buf)
{
	IIC_Start();
	Send_Byte(SlaveAddress);//��д����
	if(I2C_WaitAck()) 
	{
		IIC_Stop();//�ͷ�����
		return 1;//ûӦ�����˳�
	}
	Send_Byte(REG_Address);
	I2C_WaitAck();
	while(len--)
	{
		Send_Byte(*buf++);//����buff������
		I2C_WaitAck();	
	}
	IIC_Stop();//�ͷ�����

	return 0;
	
}

//IIC��n�ֽ�����
u8 VL_IIC_Read_nByte(u8 SlaveAddress, u8 REG_Address,u16 len,u8 *buf)
{
	IIC_Start();
	Send_Byte(SlaveAddress);//��д����
	if(I2C_WaitAck()) 
	{
		IIC_Stop();//�ͷ�����
		return 1;//ûӦ�����˳�
	}
	Send_Byte(REG_Address);
	I2C_WaitAck();

	IIC_Start();
	Send_Byte(SlaveAddress|0x01);//��������
	I2C_WaitAck();
	while(len)
	{
		if(len==1)
		{
			*buf = Read_Byte(0);
		}
		else
		{
			*buf = Read_Byte(1);
		}
		buf++;
		len--;
	}
	IIC_Stop();//�ͷ�����

	return 0;
	
}

//VL53L0X д�������
//address:��ַ
//index:ƫ�Ƶ�ַ
//pdata:����ָ��
//count:���� ���65535
u8 VL53L0X_write_multi(u8 address, u8 index,u8 *pdata,u16 count)
{
	u8 status = STATUS_OK;

	if(VL_IIC_Write_nByte(address,index,count,pdata))
	{
	   status  = STATUS_FAIL;
	}
	return status;
}


//VL53L0X ���������
//address:��ַ
//index:ƫ�Ƶ�ַ
//pdata:����ָ��
//count:���� ���65535
u8 VL53L0X_read_multi(u8 address,u8 index,u8 *pdata,u16 count)
{
	u8 status = STATUS_OK;

	if(VL_IIC_Read_nByte(address,index,count,pdata))
	{
	  status  = STATUS_FAIL;
	}

	return status;
}

//VL53L0X д1������(���ֽ�)
//address:��ַ
//index:ƫ�Ƶ�ַ
//data:����(8λ)
u8 VL53L0X_write_byte(u8 address,u8 index,u8 data)
{
	u8 status = STATUS_OK;

	status = VL53L0X_write_multi(address,index,&data,1);

	return status;
}

//VL53L0X д1������(˫�ֽ�)
//address:��ַ
//index:ƫ�Ƶ�ַ
//data:����(16λ)
u8 VL53L0X_write_word(u8 address,u8 index,u16 data)
{
	u8 status = STATUS_OK;
	
	u8 buffer[2];
	
	//��16λ���ݲ�ֳ�8λ
	buffer[0] = (u8)(data>>8);//�߰�λ
	buffer[1] = (u8)(data&0xff);//�Ͱ�λ
	
	if(index%2==1)
	{  
		//����ͨ�Ų��ܴ���Է�2�ֽڶ���Ĵ������ֽ�
		status = VL53L0X_write_multi(address,index,&buffer[0],1);
		status = VL53L0X_write_multi(address,index,&buffer[0],1);
	}else
	{
		status = VL53L0X_write_multi(address,index,buffer,2);
	}
	
	return status;
}

//VL53L0X д1������(���ֽ�)
//address:��ַ
//index:ƫ�Ƶ�ַ
//data:����(32λ)
u8 VL53L0X_write_dword(u8 address,u8 index,u32 data)
{
	
    u8 status = STATUS_OK;

    u8 buffer[4];	
	
	//��32λ���ݲ�ֳ�8λ
	buffer[0] = (u8)(data>>24);
	buffer[1] = (u8)((data&0xff0000)>>16);
	buffer[2] = (u8)((data&0xff00)>>8);
	buffer[3] = (u8)(data&0xff);
	
	status = VL53L0X_write_multi(address,index,buffer,4);
	
	return status;
	
}


//VL53L0X ��1������(���ֽ�)
//address:��ַ
//index:ƫ�Ƶ�ַ
//data:����(8λ)
u8 VL53L0X_read_byte(u8 address,u8 index,u8 *pdata)
{
	u8 status = STATUS_OK;
	 
	status = VL53L0X_read_multi(address,index,pdata,1);
	
	return status;
	 
}

//VL53L0X ��������(˫�ֽ�)
//address:��ַ
//index:ƫ�Ƶ�ַ
//data:����(16λ)
u8 VL53L0X_read_word(u8 address,u8 index,u16 *pdata)
{
	u8 status = STATUS_OK;
	
	u8 buffer[2];
	
	status = VL53L0X_read_multi(address,index,buffer,2);
	
	*pdata = ((u16)buffer[0]<<8)+(u16)buffer[1];
	
	return status;
	
}

//VL53L0X ��1������(���ֽ�)
//address:��ַ
//index:ƫ�Ƶ�ַ
//data:����(32λ)
u8 VL53L0X_read_dword(u8 address,u8 index,u32 *pdata)
{
	u8 status = STATUS_OK;
	
	u8 buffer[4];
	
	status = VL53L0X_read_multi(address,index,buffer,4);
	
	*pdata = ((u32)buffer[0]<<24)+((u32)buffer[1]<<16)+((u32)buffer[2]<<8)+((u32)buffer[3]);
	
	return status;
	
}
