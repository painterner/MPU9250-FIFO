#include "wiringPi.h"
#include "wiringPiI2C.h"
#include "stdio.h"
#define WHO_AM_I 0x75
#define PWR_MGMT_1 0X6B
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0X3C
#define GYRO_XOUT_H 0x43
#define GYRO_XOUT_L 0x44
#define GYRO_CONFIG 0X1B
#define ACCEL_CONFIG 0X1C
#define CONFIG 0X1A
#define FIFO_CNTH 114
#define FIFO_CNTL 115
#define FIFO_ENABLE 35	//Not include I2C_SLV_3
#define INT_FLAG 58
#define FIFO_R_W 116

#define MPU_TYPE 0X73
#define Pwr_Reset 0x80
int fd;
signed char readBytes;		// = int readBytes
signed char _AH,_AL,_GH,_GL,_H,_L;
int _AR,_GR,_R,fifoBuff[14];

void FifoRead(void);

int main()
{
	wiringPiSetup();
	fd = wiringPiI2CSetup(0x68);
	readBytes = wiringPiI2CReadReg8(fd, WHO_AM_I);
	if(readBytes != MPU_TYPE){
		printf("Check mpu type error,get %X\r\n",readBytes);
		return -1;
	} 
	printf("Check mpu succeed\r\n");	
	wiringPiI2CWriteReg8(fd,PWR_MGMT_1,Pwr_Reset);
	wiringPiI2CWriteReg8(fd,PWR_MGMT_1,0x00);
	wiringPiI2CWriteReg8(fd,GYRO_CONFIG,0x1D);	//unable self-test DLPF +2000dps
	wiringPiI2CWriteReg8(fd,ACCEL_CONFIG,0x18);	//unable self-test +-16g
	wiringPiI2CWriteReg8(fd,CONFIG,0x00);		
		//fifo new valueinstead old value;DLPF->GYRO:8khz(Fs) 250Khz(band) TEMP:4000hz(band)
	wiringPiI2CWriteReg8(fd,FIFO_ENABLE,0xf8);	//temp gyro accel

	unsigned char _FH,_FL;
	_FH=wiringPiI2CReadReg8(fd,FIFO_CNTH);	
	_FL=wiringPiI2CReadReg8(fd,FIFO_CNTL);
	_R=_FH<<8 | _FL;
	printf("FIFO_CNT: %d\r\n",_R);
	
	char arbitrary;
	scanf("%c",&arbitrary);
	while(1){
		_AH = wiringPiI2CReadReg8(fd,ACCEL_XOUT_H);
		//	printf("_H : %X\n",_H);
		_AL = wiringPiI2CReadReg8(fd,ACCEL_XOUT_L);
		_GH = wiringPiI2CReadReg8(fd,GYRO_XOUT_L);
		_GL = wiringPiI2CReadReg8(fd,GYRO_XOUT_L);
		
		_AR = _AH<<8 | _AL;
		_GR = _GH<<8 | _GL;

		FifoRead();
		printf("_AR : %d\n",_AR);
		printf("_BR : %d\n",_GR);		

		delay(0);
	}
	return 0;
}

void FifoRead(void)
{

	if(wiringPiI2CReadReg8(fd,INT_FLAG) & 0x10){
		printf("fifo overflow!!!\n");
		wiringPiI2CWriteReg8(fd,INT_FLAG,0x10);
	}
	for(int i=0;i<14;i++){			//temp(2) gyro(6) accel(6)
		fifoBuff[i] = wiringPiI2CReadReg8(fd,FIFO_R_W);
	}
	_R=fifoBuff[0]<<8 | fifoBuff[1];
	
	_R=fifoBuff[1]<<8 | fifoBuff[0];
	printf("temperature: %d\n",_R);
	_R=fifoBuff[2]<<8 | fifoBuff[3];
	_R=fifoBuff[3]<<8 | fifoBuff[2];
	printf("GyroX: %d\n",_R);
}
