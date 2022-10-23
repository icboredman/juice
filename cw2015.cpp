//
//
#include "cw2015.hpp"

#define SOC_USE_TABLE

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

#ifndef SOC_USE_TABLE
    int GaugeCW2015::GetSoC(float &val)
    {
        uint16_t data;
        int err;
        if ((err = i2c->Read2(data, DEV_ADDR, REG_SOC)) == 0)
            val = data / 256.0;
        return err;
    }
#else
//https://stackoverflow.com/a/7091791
    typedef struct { float x; float y; } tLut;

    tLut lut[12] =
    {
        {4.00, 100},
        {3.86, 90},
        {3.78, 80},
        {3.72, 70},
        {3.67, 60},
        {3.62, 50},
        {3.59, 40},
        {3.57, 30},
        {3.54, 20},
        {3.48, 10},
        {3.25, 5},
        {2.8, 0}
    };

    float Interpolate(tLut* c, float x, int n)
    {
        int i;

        for ( i = 0; i < n-1; i++ )
        {
            if ( c[i].x >= x && c[i+1].x < x )
            {
                float diffx = x - c[i+1].x;
                float diffn = c[i].x - c[i+1].x;

                return c[i+1].y + ( c[i].y - c[i+1].y ) * diffx / diffn;
            }
        }

        return 0; // Not in Range
    }

    int GaugeCW2015::GetSoC(float &val)
    {
        float vBat = val / 2;
        if (vBat >= lut[0].x)
            val = lut[0].y;
        else if (vBat <= lut[11].x)
            val = lut[11].y;
        else
            val = Interpolate(lut, vBat, 12);
        return 0;
    }
#endif
