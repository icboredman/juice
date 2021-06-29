//
//
#ifndef CW2015_H
#define CW2015_H

#include <stdint.h>
#include "i2c.hpp"


class GaugeCW2015
{
    public:
        GaugeCW2015(I2cDriver &drv);
        int GetMode(uint8_t &val);
        int SetMode(uint8_t val);
        int GetVBat(float &val);
        int GetSoC(float &val);

    private:
        I2cDriver *i2c;
        void Initialize();
};

#endif
