#include "interface.h"
uint8_t PcMsgBuf[100] = {0};
uint8_t PcMsgBufIndex = 0;
uint8_t PcMsgBufFinishFlag = 0;

GRBL_METH GrblLaser;
void DisableTimeInter(void){
	__HAL_TIM_DISABLE(&htim1);
}
void EnableTimeInter(void){
	__HAL_TIM_ENABLE(&htim1);
}
//设置定时器中断周期 单位ms，会返回实际中断间隔时间
double SetTimeInterMs(double timems){
	// 1us一次计数,最短能1us一次中断，最长65535us一次中断
	uint16_t i = (uint16_t)(timems*1000);
	double ret = timems - (double)i*1000.0;

	__HAL_TIM_SET_AUTORELOAD(&htim1,i);

	return ret;
}
// 对于io口需要输出高电平之后再拉低，这个时间应该交给比较捕获中断做，就是溢出中断发生之后，在x个计数周期之后发生溢出中断
// 设置一个在定时器中断发生 timeus 之后的另外一个中断。
void SetTimeCompInterUs(double timeus){
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, timeus);
}
void printPgmString(uint8_t *buf){
	HAL_UART_Transmit(&huart1,buf,strlen(buf),0);
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
	if(GPIO_PIN_RESET == HAL_GPIO_ReadPin(TOUCH_Z_PORT,TOUCH_Z_PIN)){
		return 1;
	}
	return 0;
}

void spindle_run(int32_t direction, uint32_t rpm){
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, rpm);
}
void spindle_stop(void){
	__HAL_TIM_DISABLE(&htim2);
}
// 从串口得到数据包,一般来说Gcode和配置code都是一行字符，所以一个数据包是以\r\n结束
// 数据指针指向接受缓存，当得到数据之后，复制数据到这个缓存区域
// maxlen代表的是缓冲区长度
// 返回-1:没有数据
// 返回>0:复制之后的数据长度（可能实际数据长度比maxlen大，但是返回maxlen）
int8_t ReadCmd(uint8_t*buf,uint8_t maxlen){
	uint8_t i = 0;
	if(PcMsgBufFinishFlag == 1){
		i = PcMsgBufIndex > maxlen ? maxlen:PcMsgBufIndex;
		memcpy(buf,PcMsgBuf,i);
		return i;
	}
	return -1;
}


void XAxisPwmL(void){PWM_X_PORT->BSRR = (uint32_t)PWM_X_PIN << 16u;}
void XAxisPwmH(void){PWM_X_PORT->BSRR = PWM_X_PIN;}
void XAxisDirBack(void){DIR_X_PORT->BSRR = (uint32_t)DIR_X_PIN << 16u;}
void XAxisDirGo(void){DIR_X_PORT->BSRR = DIR_X_PIN;}
void YAxisPwmL(void){PWM_Y_PORT->BSRR = (uint32_t)PWM_Y_PIN << 16u;}
void YAxisPwmH(void){PWM_Y_PORT->BSRR = PWM_Y_PIN;}
void YAxisDirBack(void){DIR_Y_PORT->BSRR = (uint32_t)DIR_Y_PIN << 16u;}
void YAxisDirGo(void){DIR_Y_PORT->BSRR = DIR_Y_PIN;}
void ZAxisPwmL(void){PWM_Z_PORT->BSRR = (uint32_t)PWM_Z_PIN << 16u;}
void ZAxisPwmH(void){PWM_Z_PORT->BSRR = PWM_Z_PIN;}
void ZAxisDirBack(void){DIR_Z_PORT->BSRR = (uint32_t)DIR_Z_PIN << 16u;}
void ZAxisDirGo(void){DIR_Z_PORT->BSRR = DIR_Z_PIN;}