#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wiringPiI2C.h>

#define DeviceAddress  0x28
#define StatusRegister 0x01
#define OpModeRegister 0x04
#define NewValRegister 0x07
#define LSBRegister 0x05
#define MSBRegister 0x06

int CO2SensorInit(int *fd){
	int readResult;
	// Initialize dvice
	*fd =  wiringPiI2CSetup(DeviceAddress);
	if(*fd == -1)
		return -1;
	readResult = wiringPiI2CReadReg8(*fd, StatusRegister);
	// Check status
	if (readResult == 0x88){
		printf("Sensor initialized correctly!\n");
		return 0;
	}
	else{
		printf("Something is wrong with the sensor. See the Status register and documentation for more info!\n");
		return -1;
		}
	}
	
int CO2SensorSingleShotMeasurement(int *fd){
	// Set the sensor to a single shot measurement
	int readResult;
	wiringPiI2CWriteReg8 (*fd, OpModeRegister, 0x02);
	// Wait until new data is available
	while(wiringPiI2CReadReg8(*fd, NewValRegister) != 0x10){}
	// Read MSB and LSB registers and return the results
	uint16_t msb, lsb;
	lsb = wiringPiI2CReadReg8(*fd, LSBRegister);
	msb = wiringPiI2CReadReg8(*fd, MSBRegister);
	return  (msb << 8)+lsb;
	}
int CO2SensorSetContiniousMeasurement(int* fd){
	wiringPiI2CWriteReg8 (*fd, OpModeRegister, 0x02);
	return 0;
	}
int CO2SensorReadContinous(int* fd){
	if(wiringPiI2CReadReg8(*fd, NewValRegister) != 0x10)
		return -1;
	// Read MSB and LSB registers and return the results
	uint16_t msb, lsb;
	lsb = wiringPiI2CReadReg8(*fd, LSBRegister);
	msb = wiringPiI2CReadReg8(*fd, MSBRegister);
	return  (msb << 8)+lsb;
	}
