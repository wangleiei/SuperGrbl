#ifndef MAINSERVER
#define MAINSERVER

// #define DEBUG_PWM

#include "interface.h"
#define GRBL_PAR_ADDR 0//该flash地址用于存放grbl的参数
#define GCODE_FILE_CONFIG	4096		//用来表示g代码的相关参数,必须是4096的整数倍
#define GCODE_FILE_INDEX (4096*2)//g代码存放在flash中地址偏移,必须是4096的整数倍

void DataInit(void);
void MainServer(void);

typedef struct {
	uint32_t GFileLen;
}GCODE_FILE_PAR;
#endif