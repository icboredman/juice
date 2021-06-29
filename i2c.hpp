//
//
#ifndef I2CDRIVER_H
#define I2CDRIVER_H

#include <stdint.h>

class I2cDriver
{
    public:
        I2cDriver(const char* dev);
        ~I2cDriver();
        bool Ready();
        int Read8(uint8_t &val, uint8_t addr, uint8_t reg);
        int Write8(uint8_t val, uint8_t addr, uint8_t reg);
        int Read2(uint16_t &val, uint8_t addr, uint8_t reg);

    private:
        int fd;
};

#endif
