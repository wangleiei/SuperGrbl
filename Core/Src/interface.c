#include "interface.h"
uint8_t PcMsgBuf[400] = {0};
uint16_t PcMsgBufIndex = 0;
uint8_t PcMsgBufFinishFlag = 0;
GRBL_METH GrblLaser;
W25QCS_BASE W25q64Dev;
/*---------------------------grbl需要接口函数开始---------------------------*/
void DisableTimeInter(void){
	__HAL_TIM_DISABLE(&htim1);
}
void EnableTimeInter(void){
	__HAL_TIM_ENABLE(&htim1);
}
// 设置定时器中断周期 单位ms，会返回实际中断间隔时间,
// 如果设置的周期与上一次的一样，就不设置，但是返回要正确
// 传入参数一定是大于0的
// 希望能支持的时间在1us-1s以内，不支持
double SetTimeInterMs(double timems){
	static double last_timems = 0;
	// 1us一次计数,最短能1us一次中断，最长65.535ms一次中断
	int32_t count_us = (int32_t)(timems*1000);
	double ret = 0.0;

	if(count_us >= 60000){
		count_us = 60000;
	}
	ret = timems - (double)count_us*1000.0;//
	if(last_timems != timems){
		__HAL_TIM_SET_AUTORELOAD(&htim1,(uint16_t)count_us);	
		last_timems = timems;
	}
	return ret;
}
// 对于io口需要输出高电平之后再拉低，这个时间应该交给比较捕获中断做，就是溢出中断发生之后，在x个计数周期之后发生溢出中断
// 设置一个在定时器中断发生 timeus 之后的另外一个中断。
void SetTimeCompInterUs(double timeus){
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, timeus);
}
void printPgmString(uint8_t *buf){
	HAL_UART_Transmit(&huart1,buf,strlen(buf),1000);
}
void PrintLog(uint8_t*buf,uint8_t len){
	HAL_UART_Transmit(&huart1,buf,len,1000);	
}
uint8_t IsTouchX(void){
	if(GPIO_PIN_RESET == HAL_GPIO_ReadPin(TOUCH_X_PORT,TOUCH_X_PIN)){
		return 1;
	}
	return 0;
}
uint8_t IsTouchY(void){
	if(GPIO_PIN_RESET == HAL_GPIO_ReadPin(TOUCH_Y_PORT,TOUCH_Y_PIN)){
		return 1;
	}
	return 0;
}
uint8_t IsTouchZ(void){
	// if(GPIO_PIN_RESET == HAL_GPIO_ReadPin(TOUCH_Z_PORT,TOUCH_Z_PIN)){
	// 	return 1;
	// }
	return 0;
}

void spindle_run(int32_t direction, uint32_t rpm){
	// __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, rpm);
}
void spindle_stop(void){
	__HAL_TIM_DISABLE(&htim2);
}
// 0-100的功率控制。0：是关闭意思
void LaserControl(uint8_t percent){
	static uint16_t laset_time_count = 0;
	uint16_t time_count = 0;
	if(0 == percent){
  		TIM_CCxChannelCmd(htim2.Instance, TIM_CHANNEL_1, TIM_CCx_DISABLE);
  		return;
	}else if(percent >= 100){
		time_count = 998;
		// __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1,998);
	}else{
		time_count = percent*10;
		// __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1,percent*10);
	}
	if(laset_time_count != time_count){
		laset_time_count = time_count;
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1,time_count);
	}
	TIM_CCxChannelCmd(htim2.Instance, TIM_CHANNEL_1, TIM_CCx_ENABLE);	
}

// 0-100的功率控制。0：是关闭意思
void AirPushControl(uint8_t percent){
	static uint16_t laset_time_count = 0;
	uint16_t time_count = 0;
	if(0 == percent){
  		TIM_CCxChannelCmd(htim2.Instance, TIM_CHANNEL_3, TIM_CCx_DISABLE);
  		return;
	}else if(percent >= 100){
		time_count = 998;
	}else{
		time_count = percent*10;
	}
	if(laset_time_count != time_count){
		laset_time_count = time_count;
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3,time_count);
	}
	TIM_CCxChannelCmd(htim2.Instance, TIM_CHANNEL_3, TIM_CCx_ENABLE);	
}



void XAxisPwmL(void){PWM_X_PORT->BSRR = (uint32_t)PWM_X_PIN << 16u;}
void XAxisPwmH(void){PWM_X_PORT->BSRR = PWM_X_PIN;}
void XAxisDirBack(void){DIR_X_PORT->BSRR = (uint32_t)DIR_X_PIN << 16u;}
void XAxisDirGo(void){DIR_X_PORT->BSRR = DIR_X_PIN;}//高电
void YAxisPwmL(void){PWM_Y_PORT->BSRR = (uint32_t)PWM_Y_PIN << 16u;}
void YAxisPwmH(void){PWM_Y_PORT->BSRR = PWM_Y_PIN;}
void YAxisDirBack(void){DIR_Y_PORT->BSRR = (uint32_t)DIR_Y_PIN << 16u;}
void YAxisDirGo(void){DIR_Y_PORT->BSRR = DIR_Y_PIN;}//高电
void ZAxisPwmL(void){/*PWM_Z_PORT->BSRR = (uint32_t)PWM_Z_PIN << 16u;*/}
void ZAxisPwmH(void){/*PWM_Z_PORT->BSRR = PWM_Z_PIN;*/}
void ZAxisDirBack(void){/*DIR_Z_PORT->BSRR = (uint32_t)DIR_Z_PIN << 16u;*/}
void ZAxisDirGo(void){/*DIR_Z_PORT->BSRR = DIR_Z_PIN;*/}
/*---------------------------grbl需要接口函数结束---------------------------*/


  
void FlashCsL(void){
	HAL_GPIO_WritePin(FLASH_CS_PORT,FLASH_CS_PIN, GPIO_PIN_RESET);
}
void FlashCsH(void){
	HAL_GPIO_WritePin(FLASH_CS_PORT,FLASH_CS_PIN, GPIO_PIN_SET);
}
uint8_t W25QCS_SPI_RW(uint8_t a_u8){
	uint8_t cout = 0;
	uint8_t t_buf[1] = {a_u8},r_buf[1] = {0};
	// HAL_SPI_TransmitReceive(&hspi2,t_buf,r_buf,1,1000);
	while(!__HAL_SPI_GET_FLAG(&hspi2,SPI_FLAG_TXE)){
		// 等到这个发送缓存为空，才能发送数据
	}
	hspi2.Instance->DR = a_u8;
	while(!__HAL_SPI_GET_FLAG(&hspi2,SPI_FLAG_TXE)){
		;//等待数据发往移位寄存器
	}
	
	while(cout++ <200);//等待数据真的发送完毕，这个和速率有关

	while(!__HAL_SPI_GET_FLAG(&hspi2,SPI_FLAG_RXNE)){
		;//等数据到达接受缓冲区
	}

	r_buf[0] = hspi2.Instance->DR;
	return r_buf[0];
}


