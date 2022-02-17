import serial
import time
# import re
# import crcmod
rx_buf = bytes()
file_buf = bytes()
ser = serial.Serial("COM5", 115200, timeout=9000000000)

rx_buf = bytes()

a = "ready to rec gcode"
ser.write(a.encode(encoding = "gbk" ))
time.sleep(1)
# 等待flash擦除完成
while True:
	rx_buf = ser.readline()	
	str_rx_buf = rx_buf.decode(encoding = "gbk" )
	print(str_rx_buf)
	if str_rx_buf.find("准备接受G代码") != -1:
		break# 擦除完成,
# 开始发送g代码
print("开始发送g代码")
fo = open("testGCODE.nc")
file_buf = fo.read()
ind = 0;
while ind < len(file_buf):
	a = file_buf[ind:ind+100].encode(encoding = "utf-8" )
	# print(file_buf[ind:ind+100])
	# break;
	ser.write(a)
	ind = ind + 100;
	time.sleep(0.2)
	pass
print(fo.name,"文件发送完毕，一共",len(file_buf),"字节")


a = "gcode trans over"
ser.write(a.encode(encoding = "gbk" ))
print(ser.readline().decode(encoding = "gbk" ))

ser.close() # 关闭串口
