#include "mainserver.h"
static int8_t ReadCmd(uint8_t*buf,uint8_t maxlen);
static void SetGcodeParm(uint32_t GFileLen);
static void GetGcodeParm(GCODE_FILE_PAR *GcodeFilePar);
static int8_t ReadNoMissingData(uint8_t*buffer,uint8_t len);
static int8_t SaveNoMissingData(uint8_t*buffer,uint8_t len);

void ResetGfileAddr(void);
void SaveGfile(uint8_t*buffer,uint16_t len);
static GCODE_FILE_PAR GcodeFilePar;
static uint8_t CanReadGcodeFileFlag = 0;
static uint8_t GcodeParm[30] = {0};
static uint8_t GcodeParmLen = 0;
static uint32_t GcodeFileFlagIndex = 0;
// 1:擦除成功
// -1:擦除失败
int8_t CleanGfile(void);
void UsartDeal(void);
void FlashCheck(void);

void DataInit(void){
	uint8_t read_buf[30] = {0};
	W25Q_Init(&W25q64Dev,
		FlashCsL,
		FlashCsH,
		// spi时序要求，时钟平时是低，上升沿捕获，高位在前，8字节数据长度,
		W25QCS_SPI_RW,
		HAL_Delay,
		8,
		W25Q);
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
		spindle_stop,
		LaserControl,
		0);
	GetGcodeParm(&GcodeFilePar);

	sprintf(read_buf,"gcode文件大小 %d\r\n",GcodeFilePar.GFileLen);
	printPgmString(read_buf);

	// FlashCheck();
	#ifdef DEBUG_PWM
		LaserControl(50);
		AirPushControl(80);
	#endif
}
void MainServer(void){
	UsartDeal();
	SpProcess(&GrblLaser);
}
// 处理来着PC串口的数据
void UsartDeal(void){
	static uint32_t gfile_len = 0;
	static uint8_t ErrSaveFlag = 0;
	uint8_t read_buf[50] = {0};
	uint8_t serial_len = 0;
	// 0：还没有接受Gcode代码
	// 1：准备接受Gcode代码
	// 2：启动测量
	// 3：grbl参数设置，串口发送的命令直接由grbl模块处理
	static uint8_t data_flag = 0;
	uint32_t i = 0;
	if(PcMsgBufFinishFlag == 1){
		serial_len = PcMsgBufIndex;
		if(data_flag == 0){
			if(strstr((uint8_t*)PcMsgBuf,"ready to rec gcode") != 0){
				printPgmString("开始擦除Flash\r\n");
				if(CleanGfile() == 1){
					
				}

				data_flag = 1;
				ResetGfileAddr();
				printPgmString("准备接受G代码\r\n");	
			}else if(strstr((uint8_t*)PcMsgBuf,"start grbl") != 0){
				printPgmString("启动GRBL运行\r\n");
				GrblStart(&GrblLaser);
				data_flag = 2;
				CanReadGcodeFileFlag = 1;
			}else if(strstr((uint8_t*)PcMsgBuf,"set grbl parm") != 0){
				printPgmString("开始设置GRBL参数\r\n");
				data_flag = 3;
				CanReadGcodeFileFlag = 2;
			}
		}else if(data_flag == 1){
			// 在g代码接受过程中
			if(strstr((uint8_t*)PcMsgBuf,"gcode trans over") != 0){
				uint8_t temp_buf[40] = {0};
				printPgmString("G代码接受结束\r\n");
				SetGcodeParm(gfile_len);
				gfile_len = 0;
				ResetGfileAddr();
				
				if(1 == ErrSaveFlag){
					printPgmString("G代码接受失败\r\n");
				}else{
					printPgmString("G代码接受成功，没有错误\r\n");
				}
				ErrSaveFlag = 0;
				data_flag = 0;

				memset(temp_buf,0,sizeof(temp_buf));
				sprintf(temp_buf,"G代码大小%d\r\n",GcodeFilePar.GFileLen);
				printPgmString(temp_buf);
			}else{
				// 接受G代码
				SaveGfile(PcMsgBuf,serial_len);
				// W25QCSXX_Read(&W25q64Dev,w_index + GCODE_FILE_INDEX,read_buf,serial_len);
				// for(i = 0;i < serial_len;i ++){
				// 	if(read_buf[i] != PcMsgBuf[i]){
				// 		PrintLog(read_buf,serial_len);

				// 		memset(read_buf,0,sizeof(read_buf));
				// 		sprintf(read_buf,"G代码无法存储-地址%x\r\n",w_index+GCODE_FILE_INDEX);
				// 		printPgmString(read_buf);
				// 		ErrSaveFlag = 1;
				// 		break;
				// 	}
				// }
				// PrintLog(uint8_t*buf,uint8_t len){
				gfile_len += serial_len;
				
			}
		}else if(data_flag == 2){
			// 在启动运行中,将g代码从flash中读取然后发送到grbl模块
			if(strstr((uint8_t*)PcMsgBuf,"pause grbl") != 0){
				printPgmString("GRBL暂停运行\r\n");
				GrblPause(&GrblLaser);
				CanReadGcodeFileFlag = 0;
			}else if(strstr((uint8_t*)PcMsgBuf,"resume grbl") != 0){
				printPgmString("GRBL恢复运行\r\n");
				GrblResume(&GrblLaser);
				CanReadGcodeFileFlag = 1;
			}else if(strstr((uint8_t*)PcMsgBuf,"stop grbl") != 0){
				printPgmString("收到GRBL停止运行指令\r\n");
				GrblStop(&GrblLaser);
				CanReadGcodeFileFlag = 0;
				GcodeFileFlagIndex = 0;
				data_flag = 0;
			}
		}else if(data_flag == 3){//处于接受g代码参数的状态
			if(strstr((uint8_t*)PcMsgBuf,"exit grbl parm") != 0){
				printPgmString("退出设置GRBL参数\r\n");
				data_flag = 0;
			}
			memset(GcodeParm,0,sizeof(GcodeParm));
			memcpy(GcodeParm,PcMsgBuf,serial_len);
			GcodeParmLen = serial_len;
		}
		if(data_flag != 1){//为了不干扰紧凑的g代码传输过程
			if(strstr((uint8_t*)PcMsgBuf,"check gcode") != 0){
				uint32_t read_file_index = 0;
				uint16_t cunrrent_data_len = 0;
				while(GcodeFilePar.GFileLen > read_file_index){
					if((GcodeFilePar.GFileLen - read_file_index) > sizeof(read_buf)){
						cunrrent_data_len = sizeof(read_buf);
					}else{
						cunrrent_data_len = GcodeFilePar.GFileLen - read_file_index;
					}
					W25QCSXX_Read(&W25q64Dev,GCODE_FILE_INDEX+read_file_index,read_buf,cunrrent_data_len);
					PrintLog(read_buf,cunrrent_data_len);
					read_file_index += cunrrent_data_len;
				};
				
			}else if(strstr((uint8_t*)PcMsgBuf,"check grbl par") != 0){
				GrblPrintSettings(&GrblLaser);
			}
		}
		PcMsgBufFinishFlag = 0;
		memset(PcMsgBuf,0,PcMsgBufIndex);
		// PcMsgBuf[PcMsgBufIndex] = 0;
		PcMsgBufIndex = 0;
	}
}

// 从串口得到数据包,一般来说Gcode和配置code都是一行字符，所以一个数据包是以\r\n结束
// 数据指针指向接受缓存，当得到数据之后，复制数据到这个缓存区域
// maxlen代表的是缓冲区长度
// 返回-1:没有数据
// 返回>0:复制之后的数据长度（可能实际数据长度比maxlen大，但是返回maxlen）
static int8_t ReadCmd(uint8_t*buf,uint8_t maxlen){
	
	uint8_t buf_line[50] = {0};
	//uint8_t nr_index = 0;
	uint8_t i = 0;
	uint8_t g_len = 0;
	if(CanReadGcodeFileFlag == 1){
		if(GcodeFileFlagIndex >= GcodeFilePar.GFileLen){
			return -1;
		}
		W25QCSXX_Read(&W25q64Dev,GcodeFileFlagIndex + GCODE_FILE_INDEX,buf_line,sizeof(buf_line));
		for(i = 0;i < (sizeof(buf_line) - 1);i ++){
			// if((buf_line[i] == '\r') && (buf_line[i+1] == '\n')){
			if((buf_line[i] == '\n')){
				// 正常数据接受
				g_len = i;
				if(g_len > maxlen){
					GcodeFileFlagIndex = maxlen + GcodeFileFlagIndex;
					g_len = maxlen;
				}else{
					GcodeFileFlagIndex = i + GcodeFileFlagIndex + 2;//+2是为了跳过"\r\n"
				}		
				break;
			}
		}
		if(i == (sizeof(buf_line) - 1)){
			uint8_t tr_buf[30] = {0};
			sprintf(tr_buf,"出现读gcode行超过缓存\r\n");//g代码没有正确结尾,或者g代码中一行数据太多
			printPgmString(tr_buf);
			GcodeFileFlagIndex += i;
			return -1;
		}
		memcpy(buf,buf_line,g_len);
		return g_len;
	}else if(CanReadGcodeFileFlag == 0){
		return -1;
	}else if(2 == CanReadGcodeFileFlag){
		if(GcodeParmLen == 0){
			return -1;
		}
		g_len = GcodeParmLen > maxlen?maxlen:GcodeParmLen;
		memcpy(buf,GcodeParm,g_len);
		GcodeParmLen = 0;
		return g_len;
	}
	return -1;
}
static void SetGcodeParm(uint32_t GFileLen){
	GCODE_FILE_PAR GcodeFilePar_;
	GcodeFilePar_.GFileLen = GFileLen;
	return;
	if(W25QCSXX_Erase_Sector(&W25q64Dev,GCODE_FILE_CONFIG) == -1){
		printPgmString("存储Gfile参数失败-无法擦除\r\n");
		return;
	}
	W25QCS_Write(&W25q64Dev,GCODE_FILE_CONFIG,(uint8_t*)&GcodeFilePar_,sizeof(GcodeFilePar_));
	GcodeFilePar_.GFileLen = 0;
	W25QCSXX_Read(&W25q64Dev,GCODE_FILE_CONFIG,(uint8_t*)&GcodeFilePar_,sizeof(GcodeFilePar_));
	if(GFileLen != GcodeFilePar_.GFileLen){
		printPgmString("存储Gfile参数失败-校验错误\r\n");
		return;
	}
	GcodeFilePar.GFileLen = GFileLen;
}
static void GetGcodeParm(GCODE_FILE_PAR *GcodeFilePar_){
	W25QCSXX_Read(&W25q64Dev,GCODE_FILE_CONFIG,(uint8_t*)GcodeFilePar_,sizeof(GcodeFilePar_));
}

// 用于获取永久存储数据的函数,GRBL用来存储一些设置参数,必须存放len个字节
// -1：获取失败
// 0:获取成功
static int8_t ReadNoMissingData(uint8_t*buffer,uint8_t len){
	uint8_t checkbuf[1] = {0};
	W25QCSXX_Read(&W25q64Dev,GRBL_PAR_ADDR,buffer,len);
	W25QCSXX_Read(&W25q64Dev,len,checkbuf,1);
	if(checkbuf[0] == 0x2d){
		return 0;
	}
	return -1;
}
// 存放永久存储数据，
// -1：存放失败
// 0:存放成功
static int8_t SaveNoMissingData(uint8_t*buffer,uint8_t len){
	uint8_t checkbuf[1] = {0x2d};
	if(W25QCSXX_Erase_Sector(&W25q64Dev,GRBL_PAR_ADDR) == -1){
		return -1;
	}

	W25QCS_Write(&W25q64Dev,GRBL_PAR_ADDR,buffer,len);
	W25QCS_Write(&W25q64Dev,len,checkbuf,1);

	checkbuf[0] = 0;
	W25QCSXX_Read(&W25q64Dev,len,checkbuf,1);

	if(checkbuf[0] == 0x2d){
		return 0;
	}
	return -1;
}
void FlashCheck(void){
	uint16_t i = 0,k = 0,j = 0;
	if(W25QCSXX_Erase_Sector(&W25q64Dev,GCODE_FILE_INDEX) == -1){
		printPgmString("无法擦除\r\n");
	}
	for(j = 0;j < 32;j++){
		uint8_t sumff[128] = {0};
		W25QCSXX_Read(&W25q64Dev,GCODE_FILE_INDEX + i*sizeof(sumff),sumff,sizeof(sumff));
		for(k  = 0;k  < sizeof(sumff);k  ++){
			if(sumff[k] != 0xff){
				printPgmString("无法擦除\r\n");
				return;
			}
		}
	}
	for(j = 0;j < 32;j++){
		uint8_t sumff[128] = {0};
		memset(sumff,0x12,sizeof(sumff));
		W25QCS_Write(&W25q64Dev,GCODE_FILE_INDEX + i*sizeof(sumff),sumff,sizeof(sumff));
		memset(sumff,0,sizeof(sumff));
		W25QCSXX_Read(&W25q64Dev,GCODE_FILE_INDEX + i*sizeof(sumff),sumff,sizeof(sumff));
		for(k  = 0;k  < sizeof(sumff);k  ++){
			if(sumff[k] != 0x12){
				printPgmString("擦除以后不能写入\r\n");
				return;
			}
		}
	}
	printPgmString("FLASH可使用\r\n");

}
// 1:擦除成功
// -1:擦除失败
int8_t CleanGfile(void){
	uint8_t read_buf[50] = {0};
	uint16_t i = 0,j = 0;
	uint32_t sectortotal = 0;
	sectortotal = W25Q_GetSectorNum(&W25q64Dev);
	// 希望能提供1MB的空间用于存放一个G代码,所以有1024kB/4kB
	sectortotal = sectortotal > (1024/4)?(1024/4):sectortotal;
	for(i = GCODE_FILE_INDEX / 4096;i < sectortotal;i ++){
		if(W25QCSXXEraseSectorByIndex(&W25q64Dev,i) == -1){
			sprintf(read_buf,"擦除第 %d setor失败\r\n",i);
			printPgmString(read_buf);
			return -1;
		}else if(i%50 == 0){
			sprintf(read_buf,"正在擦除第 %d setor的数据，一共可存放%dKB数据\r\n",i,i*4);
			printPgmString(read_buf);
		}
	}
	printPgmString("Flash擦除成功\r\n");
	return 1;
}
static uint32_t w_index = 0;
void ResetGfileAddr(void){
	w_index = 0;
}
void SaveGfile(uint8_t*buffer,uint16_t len){
	uint8_t i = 0;
	uint8_t sum[1] = {0};
	W25QCSXX_Write_NoCheck(&W25q64Dev,GCODE_FILE_INDEX + w_index,buffer,len);
	// W25QCS_Write(&W25q64Dev,GCODE_FILE_INDEX + w_index,buffer,len);
	W25QCSXX_Read(&W25q64Dev,GCODE_FILE_INDEX + w_index + len - 1,sum,1);
	if(sum[0] != buffer[len-1]){
		printPgmString("数据存放失败\r\n");
	}
	w_index += len;
}