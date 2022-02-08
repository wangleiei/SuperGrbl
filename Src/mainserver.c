#include "mainserver.h"
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
		// 限位开关函数
		// 1：触碰到行程开关
		// 0：没有触碰到行程开关
		IsTouchX,
		IsTouchY,
		IsTouchZ,
		// 主轴雕刻使用
		spindle_run,
		spindle_stop);
}
void MainServer(void){
	SpProcess(&GrblLaser);
}