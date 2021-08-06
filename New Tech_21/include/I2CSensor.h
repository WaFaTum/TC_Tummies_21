#pragma once
class I2CSensor{
    protected: int device Addr;
    public: virtual bool init ()=0;
    virtual int sigleShotMeasurement()=0;
    virtual int continuousMeasurement()=0;
}
