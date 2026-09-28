#include "modbus_tcp.h"
#include "main.h"
#include "tool.h"
#include "sys_setting.h"

//寄存器列表
static uint8_t reg_buff[REG_MAX*2]; 			            
 
/* MB错误代码 */
#define RSP_ERR_CMD        0X01   /* 不支持的功能码 */
#define RSP_ERR_REG_ADDR   0X02	  /* 寄存器地址错误 */
#define RSP_ERR_VALUE      0X03   /* 数据值域错误 */
#define RSP_ERR_WRITE      0X04	  /* 写入失败 */

static uint16_t mbs_tcp_send_ackerr(uint8_t *rx_buf, uint8_t *tx_buf, uint8_t err_code)
{
	tx_buf[0] = rx_buf[0];
	tx_buf[1] = rx_buf[1];
	tx_buf[2] = rx_buf[2];
	tx_buf[3] = rx_buf[3];
	tx_buf[4] = 0X00;
	tx_buf[5] = 0X03;
	tx_buf[6] = rx_buf[6];
	tx_buf[7] = rx_buf[6] | 0X80;
	tx_buf[8] = err_code;

    return 9;
}

static uint16_t mbs_rtu_send_ack_err(uint8_t *rx_cmd, uint8_t *ack_buf, uint8_t err_code)
{
	uint16_t crc;

	ack_buf[0] = rx_cmd[0];
	ack_buf[1] = rx_cmd[1] | 0x80;
	ack_buf[2] = err_code;

	crc = calc_crc16(ack_buf, 3);

	ack_buf[3] = crc >> 8;
	ack_buf[4] = crc;

	return 5;
}

uint16_t mbs_tcp_01(uint8_t *rx_buf, uint16_t rx_size, uint8_t *tx_buf)
{
	return  mbs_tcp_send_ackerr(rx_buf, tx_buf, RSP_ERR_REG_ADDR);
}

uint16_t mbs_tcp_02(uint8_t *rx_buf, uint16_t rx_size, uint8_t *tx_buf)
{
    return  mbs_tcp_send_ackerr(rx_buf, tx_buf, RSP_ERR_REG_ADDR);
}

uint16_t mbs_tcp_03(uint8_t *rx_buf, uint16_t rx_size,uint8_t *tx_buf)
{
   uint16_t reg, value_num;
   uint16_t us_count;
   uint32_t end;

   /*if(rx_buf[6] != 0x01)
   {
	   return 0;
   }*/
   reg = buf_ab_2_uint16(&rx_buf[8]);
   value_num = buf_ab_2_uint16(&rx_buf[10]);
   end = reg + value_num;
   //if((reg + value_num) > REG_MAX)
   if(end > REG_MAX)
   {
	  return mbs_tcp_send_ackerr(rx_buf, tx_buf, RSP_ERR_VALUE);  /*寄存器越界*/
   }
   if(value_num > 0x7D)
   {
        value_num = 0x7D;
   }
   value_num *= 2;

   for(us_count = 0; us_count < value_num; us_count++)
   {
	   tx_buf[us_count + 9] = reg_buff[us_count + reg * 2];
   }
   tx_buf[0] = rx_buf[0];
   tx_buf[1] = rx_buf[1];
   tx_buf[2] = rx_buf[2];
   tx_buf[3] = rx_buf[3];
   tx_buf[4] = (value_num + 3) >> 8;
   tx_buf[5] = (value_num + 3);
   tx_buf[6] = rx_buf[6];
   tx_buf[7] = 3;
   tx_buf[8] = value_num;

   return (9 + value_num);
}

uint16_t mbs_tcp_04(uint8_t *rx_buf, uint16_t rx_size, uint8_t *tx_buf)
{
	return  mbs_tcp_send_ackerr(rx_buf, tx_buf, RSP_ERR_REG_ADDR);
}

uint16_t mbs_tcp_05(uint8_t *rx_buf, uint16_t rx_size, uint8_t *tx_buf)
{
	return  mbs_tcp_send_ackerr(rx_buf, tx_buf, RSP_ERR_REG_ADDR);
}

uint16_t mbs_tcp_06(uint8_t *rx_buf, uint16_t rx_size, uint8_t *tx_buf)
{
	return  mbs_tcp_send_ackerr(rx_buf, tx_buf, RSP_ERR_REG_ADDR);
}
uint16_t mbs_tcp_10(uint8_t *rx_buf, uint16_t rx_size, uint8_t *tx_buf)
{
	return  mbs_tcp_send_ackerr(rx_buf, tx_buf, RSP_ERR_REG_ADDR);
}

uint16_t mbs_tcp_app(uint8_t *rx_buf, uint16_t rx_size ,uint8_t *tx_buf)
{
	uint16_t tx_size;

	switch(rx_buf[7])
	{
		case 0X01:  tx_size = mbs_tcp_01(rx_buf, rx_size, tx_buf);break; 	//读取线圈状态

		case 0X02:  tx_size = mbs_tcp_02(rx_buf, rx_size, tx_buf);break;	//读取输入状态

		case 0X03:  tx_size = mbs_tcp_03(rx_buf, rx_size, tx_buf);break;	//读取保持寄存器
			
		case 0X04:  tx_size = mbs_tcp_04(rx_buf, rx_size, tx_buf);break;	//读取输入寄存器

		case 0X05:  tx_size = mbs_tcp_05(rx_buf, rx_size, tx_buf);break;	//强制单线圈设置

		case 0X06:  tx_size = mbs_tcp_06(rx_buf, rx_size, tx_buf);break;	//写单个保存寄存器

		case 0X10:  tx_size = mbs_tcp_10(rx_buf, rx_size, tx_buf);break;    //写多个保存寄存器

		default:    tx_size = mbs_tcp_send_ackerr(rx_buf, tx_buf, RSP_ERR_CMD);break;
	}
	return tx_size;
}

uint16_t mbs_tcp_poll(uint8_t *tcp_buf,uint16_t tcp_size, uint8_t *tx_buf)
{
    if(tcp_size >= 12 && (tcp_buf[6] == config_get_dev_addr() || tcp_buf[6] == 255) && tcp_buf[5] == 6)
 //if(tcp_size >= 12 && tcp_buf[6] == 0x01 && tcp_buf[5] == 6)
  {
    return mbs_tcp_app(tcp_buf, tcp_size, tx_buf);
  }
  return 0;
}

/*01功能码为读继电器输出*/
uint16_t mbs_rtu_01(uint8_t *rx_cmd, uint16_t cmd_size, uint8_t *ack_buf)
{
	return mbs_rtu_send_ack_err(rx_cmd, ack_buf, RSP_ERR_REG_ADDR);
}

/* 读开关量输入 */
uint16_t mbs_rtu_02(uint8_t *rx_cmd, uint16_t cmd_size, uint8_t *ack_buf)
{
	return mbs_rtu_send_ack_err(rx_cmd, ack_buf, RSP_ERR_REG_ADDR);
}

/* 读取03寄存器,计数值、频率和dac */
uint16_t mbs_rtu_03(uint8_t *rx_cmd, uint16_t cmd_size, uint8_t *ack_buf)
{
	uint16_t reg, value_num, cnt, crc;
	
	ack_buf[0] = rx_cmd[0];
	ack_buf[1] = rx_cmd[1];
	
	reg = buf_ab_2_uint16(&rx_cmd[2]);
	value_num = buf_ab_2_uint16(&rx_cmd[4]);

    if((reg + value_num) > REG_MAX)
    {
        return mbs_rtu_send_ack_err(rx_cmd, ack_buf, RSP_ERR_VALUE); 
    }
    value_num *= 2;
    ack_buf[2] = value_num;
    for(cnt = 0; cnt < value_num; cnt++)
    {
        ack_buf[3 + cnt] = reg_buff[cnt + reg * 2];
    }
	crc = calc_crc16(ack_buf, value_num + 3);
	ack_buf[value_num + 3] = crc >> 8;
	ack_buf[value_num + 4] = crc;
	return value_num + 5;
}

/* 读取模拟量 */
uint16_t mbs_rtu_04(uint8_t *rx_cmd, uint16_t cmd_size, uint8_t *ack_buf)
{
	return mbs_rtu_send_ack_err(rx_cmd, ack_buf, RSP_ERR_REG_ADDR);
}

uint16_t mbs_rtu_05(uint8_t *rx_cmd, uint16_t cmd_size, uint8_t *ack_buf)
{
	return mbs_rtu_send_ack_err(rx_cmd, ack_buf, RSP_ERR_REG_ADDR);
}

uint16_t mbs_rtu_06(uint8_t *rx_cmd, uint16_t cmd_size, uint8_t *ack_buf)
{
	uint16_t reg_addr;
	//uint16_t reg_num;
	
	uint8_t *tmp;
	uint16_t tmp_16;

	reg_addr  = buf_ab_2_uint16(&rx_cmd[2]);
	
	if(reg_addr > REG_MAX || reg_addr < REG_ADDR)
	{
		return mbs_rtu_send_ack_err(rx_cmd, ack_buf, RSP_ERR_VALUE);
	}
	
	tmp = &rx_cmd[4];
	
	if(reg_addr == REG_ADDR) 												//设备地址
	{
		tmp_16 = buf_ab_2_uint16(tmp);
		if(tmp_16 > 255)
		{
			tmp_16 = 255;	
		}
		if(tmp_16 < 1)
		{
			tmp_16 = 1;
		}
		config_set_dev_addr(tmp_16);
		mbs_write_reg_16(&tmp_16, REG_ADDR);   
	}
	memcpy(ack_buf, rx_cmd, 8);	
	system_write_setting(); 
	return 8;
}

uint16_t mbs_rtu_0f(uint8_t *rx_cmd, uint16_t cmd_size, uint8_t *ack_buf)
{
	return mbs_rtu_send_ack_err(rx_cmd, ack_buf, RSP_ERR_REG_ADDR);
}

uint16_t mbs_rtu_10(uint8_t *rx_cmd, uint16_t cmd_size, uint8_t *ack_buf)
{
    return mbs_rtu_send_ack_err(rx_cmd, ack_buf, RSP_ERR_REG_ADDR);
}

uint16_t mbrtu_proc(uint8_t *rx_cmd, uint16_t cmd_size, uint8_t *ack_buf)
{
	uint16_t ack_size = 0;
	
	if(calc_crc16(rx_cmd, cmd_size) != 0)
	{
		return 0;
	}
	if(config_get_dev_addr() == rx_cmd[0] || rx_cmd[0] == 0xff)
	{
		switch(rx_cmd[1])
		{
			case 0x01:
				ack_size = mbs_rtu_01(rx_cmd, cmd_size, ack_buf);break;

			case 0x02:
				ack_size = mbs_rtu_02(rx_cmd, cmd_size, ack_buf);break;

			case 0x03:
				ack_size = mbs_rtu_03(rx_cmd, cmd_size, ack_buf);break;

			case 0x04:
				ack_size = mbs_rtu_04(rx_cmd, cmd_size, ack_buf);break;
			
			case 0x05:
				ack_size = mbs_rtu_05(rx_cmd, cmd_size, ack_buf);break;
			
			case 0x06:
				ack_size = mbs_rtu_06(rx_cmd, cmd_size, ack_buf);break;
			
			case 0x0f:
				ack_size = mbs_rtu_0f(rx_cmd, cmd_size, ack_buf);break;
			
			case 0x10:
				ack_size = mbs_rtu_10(rx_cmd, cmd_size, ack_buf);break;

			default:
				ack_size = mbs_rtu_send_ack_err(rx_cmd, ack_buf, RSP_ERR_CMD);break;
		}
	}
	return ack_size;
}

void mbs_write_reg_float(float *value, uint16_t reg_addr)
{
	uint8_t *p_buf;
	uint8_t *temp;

    p_buf = &reg_buff[reg_addr * 2];
	temp = (uint8_t *)value;
	p_buf[3] = *temp;temp++;
	p_buf[2] = *temp;temp++;
	p_buf[1] = *temp;temp++;
	p_buf[0] = *temp;
}

void mbs_write_reg_16(uint16_t *value, uint16_t reg_addr)
{
	uint8_t *p_buf;
	uint8_t *temp;
	
	p_buf = &reg_buff[reg_addr * 2];
	temp = (uint8_t *)value;
	p_buf[1] = *temp;temp++;
	p_buf[0] = *temp;
}

void mbs_write_reg_int(uint32_t *value, uint16_t reg_addr)
{
	uint8_t *p_buf;
	uint8_t *temp;

	p_buf= &reg_buff[reg_addr * 2];
	temp = (uint8_t *)value;
	p_buf[3] = *temp;temp++;
	p_buf[2] = *temp;temp++;
	p_buf[1] = *temp;temp++;
	p_buf[0] = *temp;
}

//MODBUS数据初始化
void mbs_data_init(void)
{
	uint16_t tmp_16 = 0;

    memset(reg_buff,0,sizeof(reg_buff));
    
	tmp_16 = config_get_dev_addr();
	mbs_write_reg_16(&tmp_16, REG_ADDR);
}
