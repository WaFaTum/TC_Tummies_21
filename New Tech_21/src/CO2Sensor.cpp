#include "CO2Sensor.h"

CO2Sensor::CO2Sensor(int devAddr): deviceAddr{devAddr} {}
bool CO2Sensor::init(){    
}

bool CO2Sensor::init(){
	// Initialize device
	this->fd =  wiringPiI2CSetup(this->deviceAddr);
	if(this->fd == -1)
		return false;
	
	// Check status
	if (wiringPiI2CReadReg8(this->fd, this->statusReg) == 0x88){
		std::cout << "Sensor initialized correctly!" << std::endl;
		return true;
	}
	else{
		std::cout << "Something is wrong with the sensor. See the Status register and documentation for more info!" << std::endl;
		return false;
	}
}

int CO2Sensor::singleShotMeasurement(){
    // Set the sensor to a single shot measurement
	wiringPiI2CWriteReg8 (this->fd, this->OpModeReg, 0x02);
	// Wait until new data is available
	while(wiringPiI2CReadReg8(this->fd, this->NewValReg) != 0x10){}
	// Read MSB and LSB registers and return the results
	uint16_t msb, lsb;
	lsb = wiringPiI2CReadReg8(this->fd, this->LSBReg);
	msb = wiringPiI2CReadReg8(this->fd, this->MSBReg);
	return  (msb << 8)+lsb;
}

void CO2Sensor::SetContiniousMeasurement(){
	wiringPiI2CWriteReg8 (this->fd, this->OpModeReg, 0x02);
}
int CO2Sensor::continousMeasuremnt(){
    // If there's no new value don't read 
	if(wiringPiI2CReadReg8(this->fd, this->NewValReg) != 0x10)
		return -1;
	// Read MSB and LSB registers and return the results
	uint16_t msb, lsb;
	lsb = wiringPiI2CReadReg8(*fd, LSBRegister);
	msb = wiringPiI2CReadReg8(*fd, MSBRegister);
	return  (msb << 8)+lsb;
}