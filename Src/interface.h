#ifndef INTERFACE
#define INTERFACE
#include <stdint.h>
#include <string.h>

#include "stm32f1xx_hal.h"
#include "GrblMeth.h"

#define TOUCH_X_PORT GPIOB
#define TOUCH_X_PIN 	GPIO_PIN_1

#define TOUCH_Y_PORT GPIOB
#define TOUCH_Y_PIN 	GPIO_PIN_2

#define TOUCH_Z_PORT GPIOB
#define TOUCH_Z_PIN 	GPIO_PIN_10


#define PWM_X_PORT GPIOB
#define PWM_X_PIN 	GPIO_PIN_7
#define DIR_X_PORT GPIOA
#define DIR_X_PIN 	GPIO_PIN_7

#define PWM_Y_PORT GPIOB
#define PWM_Y_PIN 	GPIO_PIN_6
#define DIR_Y_PORT GPIOA
#define DIR_Y_PIN 	GPIO_PIN_3

#define PWM_Z_PORT GPIOA
#define PWM_Z_PIN GPIO_PIN_4
#define DIR_Z_PORT GPIOB
#define DIR_Z_PIN GPIO_PIN_0

void DisableTimeInter(void);
void EnableTimeInter(void);
double SetTimeInterMs(double timems);
void SetTimeCompInterUs(double timeus);
void printPgmString(uint8_t *buf);
uint8_t IsTouchX(void);
uint8_t IsTouchY(void);
uint8_t IsTouchZ(void);
void spindle_run(int32_t direction, uint32_t rpm);
void spindle_stop(void);
int8_t ReadCmd(uint8_t*buf,uint8_t maxlen);
void XAxisPwmL(void);
void XAxisPwmH(void);
void XAxisDirBack(void);
void XAxisDirGo(void);
void YAxisPwmL(void);
void YAxisPwmH(void);
void YAxisDirBack(void);
void YAxisDirGo(void);
void ZAxisPwmL(void);
void ZAxisPwmH(void);
void ZAxisDirBack(void);
void ZAxisDirGo(void);
extern uint8_t PcMsgBuf[100];
extern uint8_t PcMsgBufIndex;
extern uint8_t PcMsgBufFinishFlag;
extern GRBL_METH GrblLaser;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;

extern UART_HandleTypeDef huart1;
#endif