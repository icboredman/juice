//
//
#include "cw2015.hpp"

#define DEV_ADDR    0x62

#define REG_VERSION 0x00
#define REG_VCELL   0x02
#define REG_SOC     0x04
#define REG_RRT     0x06
#define REG_CONFIG  0x08
#define REG_MODE    0x0A


    GaugeCW2015::GaugeCW2015(I2cDriver &drv)
    {
        i2c = &drv;
        Initialize();
    }

    void GaugeCW2015::Initialize()
    {
        uint8_t mode;
        if (GetMode(mode) == 0)
            if (mode & 0xC0)   // if sleeping, need to wake up
                SetMode(0x00); // wake up by writing zero
    }

    int GaugeCW2015::GetMode(uint8_t &val)
    {
        return i2c->Read8(val, DEV_ADDR, REG_MODE);
    }

    int GaugeCW2015::SetMode(uint8_t val)
    {
        return i2c->Write8(val, DEV_ADDR, REG_MODE);
    }

    int GaugeCW2015::GetVBat(float &val)
    {
        uint16_t data;
        int err;
        if ((err = i2c->Read2(data, DEV_ADDR, REG_VCELL)) == 0)
            val = data * 305e-6 * 2.0;
        return err;
    }

    int GaugeCW2015::GetSoC(float &val)
    {
        uint16_t data;
        int err;
        if ((err = i2c->Read2(data, DEV_ADDR, REG_SOC)) == 0)
            val = data / 256.0;
        return err;
    }
