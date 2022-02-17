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
// 1:�����ɹ�
// -1:����ʧ��
int8_t CleanGfile(void);
void UsartDeal(void);
void FlashCheck(void);

void DataInit(void){
	uint8_t read_buf[30] = {0};
	W25Q_Init(&W25q64Dev,
		FlashCsL,
		FlashCsH,
		// spiʱ��Ҫ��ʱ��ƽʱ�ǵͣ������ز��񣬸�λ��ǰ��8�ֽ����ݳ���,
		W25QCS_SPI_RW,
		HAL_Delay,
		8,
		W25Q);
	GrblInit(&GrblLaser,
		// �Ӵ��ڵõ����ݰ�,һ����˵Gcode������code����һ���ַ�������һ�����ݰ�����\r\n����
		// ����ָ��ָ����ܻ��棬���õ�����֮�󣬸������ݵ������������
		// maxlen������ǻ���������
		// ����-1:û������
		// ����>0:����֮������ݳ��ȣ�����ʵ�����ݳ��ȱ�maxlen�󣬵��Ƿ���maxlen��
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
		// ����io����Ҫ����ߵ�ƽ֮�������ͣ����ʱ��Ӧ�ý����Ƚϲ����ж�������������жϷ���֮����x����������֮��������ж�
		SetTimeCompInterUs,
		// ��ӡ��־ʹ��
		printPgmString,
		ReadNoMissingData,
		SaveNoMissingData,
		// ��λ���غ���
		// 1���������г̿���
		// 0��û�д������г̿���
		IsTouchX,
		IsTouchY,
		IsTouchZ,
		// ������ʹ��
		spindle_run,
		spindle_stop,
		LaserControl,
		0);
	GetGcodeParm(&GcodeFilePar);

	sprintf(read_buf,"gcode�ļ���С %d\r\n",GcodeFilePar.GFileLen);
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
// ��������PC���ڵ�����
void UsartDeal(void){
	static uint32_t gfile_len = 0;
	static uint8_t ErrSaveFlag = 0;
	uint8_t read_buf[50] = {0};
	uint8_t serial_len = 0;
	// 0����û�н���Gcode����
	// 1��׼������Gcode����
	// 2����������
	// 3��grbl�������ã����ڷ��͵�����ֱ����grblģ�鴦��
	static uint8_t data_flag = 0;
	uint32_t i = 0;
	if(PcMsgBufFinishFlag == 1){
		serial_len = PcMsgBufIndex;
		if(data_flag == 0){
			if(strstr((uint8_t*)PcMsgBuf,"ready to rec gcode") != 0){
				printPgmString("��ʼ����Flash\r\n");
				if(CleanGfile() == 1){
					
				}

				data_flag = 1;
				ResetGfileAddr();
				printPgmString("׼������G����\r\n");	
			}else if(strstr((uint8_t*)PcMsgBuf,"start grbl") != 0){
				printPgmString("����GRBL����\r\n");
				GrblStart(&GrblLaser);
				data_flag = 2;
				CanReadGcodeFileFlag = 1;
			}else if(strstr((uint8_t*)PcMsgBuf,"set grbl parm") != 0){
				printPgmString("��ʼ����GRBL����\r\n");
				data_flag = 3;
				CanReadGcodeFileFlag = 2;
			}
		}else if(data_flag == 1){
			// ��g������ܹ�����
			if(strstr((uint8_t*)PcMsgBuf,"gcode trans over") != 0){
				uint8_t temp_buf[40] = {0};
				printPgmString("G������ܽ���\r\n");
				SetGcodeParm(gfile_len);
				gfile_len = 0;
				ResetGfileAddr();
				
				if(1 == ErrSaveFlag){
					printPgmString("G�������ʧ��\r\n");
				}else{
					printPgmString("G������ܳɹ���û�д���\r\n");
				}
				ErrSaveFlag = 0;
				data_flag = 0;

				memset(temp_buf,0,sizeof(temp_buf));
				sprintf(temp_buf,"G�����С%d\r\n",GcodeFilePar.GFileLen);
				printPgmString(temp_buf);
			}else{
				// ����G����
				SaveGfile(PcMsgBuf,serial_len);
				// W25QCSXX_Read(&W25q64Dev,w_index + GCODE_FILE_INDEX,read_buf,serial_len);
				// for(i = 0;i < serial_len;i ++){
				// 	if(read_buf[i] != PcMsgBuf[i]){
				// 		PrintLog(read_buf,serial_len);

				// 		memset(read_buf,0,sizeof(read_buf));
				// 		sprintf(read_buf,"G�����޷��洢-��ַ%x\r\n",w_index+GCODE_FILE_INDEX);
				// 		printPgmString(read_buf);
				// 		ErrSaveFlag = 1;
				// 		break;
				// 	}
				// }
				// PrintLog(uint8_t*buf,uint8_t len){
				gfile_len += serial_len;
				
			}
		}else if(data_flag == 2){
			// ������������,��g�����flash�ж�ȡȻ���͵�grblģ��
			if(strstr((uint8_t*)PcMsgBuf,"pause grbl") != 0){
				printPgmString("GRBL��ͣ����\r\n");
				GrblPause(&GrblLaser);
				CanReadGcodeFileFlag = 0;
			}else if(strstr((uint8_t*)PcMsgBuf,"resume grbl") != 0){
				printPgmString("GRBL�ָ�����\r\n");
				GrblResume(&GrblLaser);
				CanReadGcodeFileFlag = 1;
			}else if(strstr((uint8_t*)PcMsgBuf,"stop grbl") != 0){
				printPgmString("�յ�GRBLֹͣ����ָ��\r\n");
				GrblStop(&GrblLaser);
				CanReadGcodeFileFlag = 0;
				GcodeFileFlagIndex = 0;
				data_flag = 0;
			}
		}else if(data_flag == 3){//���ڽ���g���������״̬
			if(strstr((uint8_t*)PcMsgBuf,"exit grbl parm") != 0){
				printPgmString("�˳�����GRBL����\r\n");
				data_flag = 0;
			}
			memset(GcodeParm,0,sizeof(GcodeParm));
			memcpy(GcodeParm,PcMsgBuf,serial_len);
			GcodeParmLen = serial_len;
		}
		if(data_flag != 1){//Ϊ�˲����Ž��յ�g���봫�����
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

// �Ӵ��ڵõ����ݰ�,һ����˵Gcode������code����һ���ַ�������һ�����ݰ�����\r\n����
// ����ָ��ָ����ܻ��棬���õ�����֮�󣬸������ݵ������������
// maxlen������ǻ���������
// ����-1:û������
// ����>0:����֮������ݳ��ȣ�����ʵ�����ݳ��ȱ�maxlen�󣬵��Ƿ���maxlen��
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
				// �������ݽ���
				g_len = i;
				if(g_len > maxlen){
					GcodeFileFlagIndex = maxlen + GcodeFileFlagIndex;
					g_len = maxlen;
				}else{
					GcodeFileFlagIndex = i + GcodeFileFlagIndex + 2;//+2��Ϊ������"\r\n"
				}		
				break;
			}
		}
		if(i == (sizeof(buf_line) - 1)){
			uint8_t tr_buf[30] = {0};
			sprintf(tr_buf,"���ֶ�gcode�г�������\r\n");//g����û����ȷ��β,����g������һ������̫��
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
		printPgmString("�洢Gfile����ʧ��-�޷�����\r\n");
		return;
	}
	W25QCS_Write(&W25q64Dev,GCODE_FILE_CONFIG,(uint8_t*)&GcodeFilePar_,sizeof(GcodeFilePar_));
	GcodeFilePar_.GFileLen = 0;
	W25QCSXX_Read(&W25q64Dev,GCODE_FILE_CONFIG,(uint8_t*)&GcodeFilePar_,sizeof(GcodeFilePar_));
	if(GFileLen != GcodeFilePar_.GFileLen){
		printPgmString("�洢Gfile����ʧ��-У�����\r\n");
		return;
	}
	GcodeFilePar.GFileLen = GFileLen;
}
static void GetGcodeParm(GCODE_FILE_PAR *GcodeFilePar_){
	W25QCSXX_Read(&W25q64Dev,GCODE_FILE_CONFIG,(uint8_t*)GcodeFilePar_,sizeof(GcodeFilePar_));
}

// ���ڻ�ȡ���ô洢���ݵĺ���,GRBL�����洢һЩ���ò���,������len���ֽ�
// -1����ȡʧ��
// 0:��ȡ�ɹ�
static int8_t ReadNoMissingData(uint8_t*buffer,uint8_t len){
	uint8_t checkbuf[1] = {0};
	W25QCSXX_Read(&W25q64Dev,GRBL_PAR_ADDR,buffer,len);
	W25QCSXX_Read(&W25q64Dev,len,checkbuf,1);
	if(checkbuf[0] == 0x2d){
		return 0;
	}
	return -1;
}
// ������ô洢���ݣ�
// -1�����ʧ��
// 0:��ųɹ�
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
		printPgmString("�޷�����\r\n");
	}
	for(j = 0;j < 32;j++){
		uint8_t sumff[128] = {0};
		W25QCSXX_Read(&W25q64Dev,GCODE_FILE_INDEX + i*sizeof(sumff),sumff,sizeof(sumff));
		for(k  = 0;k  < sizeof(sumff);k  ++){
			if(sumff[k] != 0xff){
				printPgmString("�޷�����\r\n");
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
				printPgmString("�����Ժ���д��\r\n");
				return;
			}
		}
	}
	printPgmString("FLASH��ʹ��\r\n");

}
// 1:�����ɹ�
// -1:����ʧ��
int8_t CleanGfile(void){
	uint8_t read_buf[50] = {0};
	uint16_t i = 0,j = 0;
	uint32_t sectortotal = 0;
	sectortotal = W25Q_GetSectorNum(&W25q64Dev);
	// ϣ�����ṩ1MB�Ŀռ����ڴ��һ��G����,������1024kB/4kB
	sectortotal = sectortotal > (1024/4)?(1024/4):sectortotal;
	for(i = GCODE_FILE_INDEX / 4096;i < sectortotal;i ++){
		if(W25QCSXXEraseSectorByIndex(&W25q64Dev,i) == -1){
			sprintf(read_buf,"������ %d setorʧ��\r\n",i);
			printPgmString(read_buf);
			return -1;
		}else if(i%50 == 0){
			sprintf(read_buf,"���ڲ����� %d setor�����ݣ�һ���ɴ��%dKB����\r\n",i,i*4);
			printPgmString(read_buf);
		}
	}
	printPgmString("Flash�����ɹ�\r\n");
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
		printPgmString("���ݴ��ʧ��\r\n");
	}
	w_index += len;
}