#include "mainserver.h"
void UsartDeal(void);

void DataInit(void){
	
	GrblInit(&GrblLaser,
		// 从串口得到数据包,一般来说Gcode和配置code都是一行字符，所以一个数据包是以\r\n结束
		// 数据指针指向接受缓存，当得到数据之后，复制数据到这个缓存区域
		// maxlen代表的是缓冲区长度
		// 返回-1:没有数据
		// 返回>0:复制之后的数据长度（可能实际数据长度比maxlen大，但是返回maxlen）
		ReadCmd,
		XAxisPwmL,
		XAxisPwmH,
		XAxisDirBack,
		XAxisDirGo,
		YAxisPwmL,
		YAxisPwmH,
		YAxisDirBack,
		YAxisDirGo,
		ZAxisPwmL,
		ZAxisPwmH,
		ZAxisDirBack,
		ZAxisDirGo,
		DisableTimeInter,
		EnableTimeInter,
		SetTimeInterMs,
		// 对于io口需要输出高电平之后再拉低，这个时间应该交给比较捕获中断做，就是溢出中断发生之后，在x个计数周期之后发生溢出中断
		SetTimeCompInterUs,
		// 打印日志使用
		printPgmString,
		ReadNoMissingData,
		SaveNoMissingData,
		// 限位开关函数
		// 1：触碰到行程开关
		// 0：没有触碰到行程开关
		IsTouchX,
		IsTouchY,
		IsTouchZ,
		// 主轴雕刻使用
		spindle_run,
		spindle_stop);
	W25Q_Init(&W25q64Dev,
		nap,
		nap,
		// spi时序要求，时钟平时是低，上升沿捕获，高位在前，8字节数据长度,
		W25QCS_SPI_RW,
		HAL_Delay,
		8,
		W25Q);
	
}
void MainServer(void){
	UsartDeal();
	SpProcess(&GrblLaser);
}
void UsartDeal(void){
	// 0：还没有接受Gcode代码
	// 1：准备接受Gcode代码
	// 2：启动测量
	// 3：grbl参数设置，串口发送的命令直接由grbl模块处理
	static uint8_t data_flag = 0;

	if(PcMsgBufFinishFlag == 1){
		if(data_flag == 0){
			if(strstr((uint8_t*)PcMsgBufIndex,"ready to rec gcode") != 0){
				data_flag = 1;
			}else if(strstr((uint8_t*)PcMsgBufIndex,"start grbl") != 0){
				data_flag = 2;
			}else if(strstr((uint8_t*)PcMsgBufIndex,"set grbl parm") != 0){
				data_flag = 3;
			}
		}else if(data_flag == 1){
			if(strstr((uint8_t*)PcMsgBufIndex,"gcode trans over") != 0){
				data_flag = 0;
			}else{
				static uint32_t w_index = 4096;
				W25QCS_Write(&W25q64Dev,w_index,PcMsgBuf,PcMsgBufIndex);
				w_index += PcMsgBufIndex;

				if((w_index > (4096+20)) && (strstr((uint8_t*)PcMsgBufIndex,"%\r\n") != 0)){
					w_index = 0;
				}

			}
		}else if(data_flag == 2){
			
		}

		PcMsgBufIndex = 0;
		PcMsgBufFinishFlag = 0;
		memset(PcMsgBuf,0,sizeof(PcMsgBuf));
	}
}