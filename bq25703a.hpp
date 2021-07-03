//
//
#ifndef BQ25703A_H
#define BQ25703A_H

#include <stdint.h>
#include "i2c.hpp"


class ChargerBQ25703A
{
    public:
        ChargerBQ25703A(I2cDriver &drv);
        void Kick(void);
        int GetID(uint8_t &val);
        int GetStatus(uint16_t &val);
        int GetVBUS(float &val);
        int GetPSYS(float &val);
        int GetIIN(float &val);
        int GetIDCHG(float &val);
        int GetICHG(float &val);
        int GetVSYS(float &val);
        int GetVBAT(float &val);

        int GetOption(uint16_t &val);
        int SetOption(uint16_t val);

    private:
        I2cDriver *i2c;
        void Initialize();
};

#endif
