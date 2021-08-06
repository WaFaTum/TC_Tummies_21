#include "I2CSensor.h"

class CO2Sensor public:I2CSensor{
    
    private:
    int fd;
    const int statusReg = 0x01;
    const int opModeReg = 0x04;
    const int LSBReg = 0x05;
    const int MSBReg = 0x06;
    const int newValReg = 0x07;
    
    public:
    CO2Sensor(int devAddr);
    bool init();
    int singleShotMeasurement();
    int continuousMeasurement();
}

