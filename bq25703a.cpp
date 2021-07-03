//
//
#include "bq25703a.hpp"

#define DEV_ADDR    0x6B

#define adcVBUS                 0x27
#define adcPSYS                 0x26
#define adcVSYS                 0x2D
#define adcVBAT                 0x2C
#define adcICHG                 0x29
#define adcIDCHG                0x28
#define adcIIN                  0x2B
#define adcCMPIN                0x2A
#define chargerStatus1          0x21
#define chargerStatus2          0x20
#define manufacturerID          0x2E
#define deviceID                0x2F
#define ADCOptions              0x3B
#define ADCEns                  0x3A
#define chargerOption0reg1      0x01
#define chargerOption0reg2      0x00
#define chargerOption1reg1      0x31
#define chargerOption1reg2      0x30
#define chargeCurrentReg1       0x03
#define chargeCurrentReg2       0x02
#define maxChargeVoltageReg1    0x05
#define maxChargeVoltageReg2    0x04
#define inputVoltageReg1        0x0B
#define inputVoltageReg2        0x0A
#define minSystemVoltageReg1    0x0D
#define minSystemVoltageReg2    0x0C


    ChargerBQ25703A::ChargerBQ25703A(I2cDriver &drv)
    {
        i2c = &drv;
        Initialize();
    }

    void ChargerBQ25703A::Initialize()
    {
        // Disable Low Power Mode, set Watchdog to 5 sec
        i2c->Write16(0x0E22, DEV_ADDR, chargerOption0reg2);
        // Configure Max Charge Voltage to 8.2 V
        uint16_t val = (8200 / 16) << 4;
        i2c->Write8(val & 0xFF, DEV_ADDR, maxChargeVoltageReg2); // LSB
        i2c->Write8(val >> 8, DEV_ADDR, maxChargeVoltageReg1);   // MSB
        // Configure Charge Current to 1 A
        val = (1000 / 64) << 6;
        i2c->Write8(val & 0xFF, DEV_ADDR, chargeCurrentReg2); // LSB
        i2c->Write8(val >> 8, DEV_ADDR, chargeCurrentReg1);   // MSB
        // Set the ADC on IBAT and PSYS to record values
        i2c->Write16(0x1192, DEV_ADDR, chargerOption1reg2);
        // enable continuous conversion on ADCs: VBUS, PSYS, IIN, IDCHG, ICHG, VSYS, VBAT
        i2c->Write16(0x7FA0, DEV_ADDR, ADCEns);
    }

    void ChargerBQ25703A::Kick(void)
    {
        // Configure Charge Current to 1 A
        uint16_t val = (1000 / 64) << 6;
        i2c->Write8(val & 0xFF, DEV_ADDR, chargeCurrentReg2); // LSB
        i2c->Write8(val >> 8, DEV_ADDR, chargeCurrentReg1);   // MSB
    }

    int ChargerBQ25703A::GetID(uint8_t &val)
    {
        return i2c->Read8(val, DEV_ADDR, manufacturerID);
    }

    int ChargerBQ25703A::GetStatus(uint16_t &val)
    {
        return i2c->Read16(val, DEV_ADDR, chargerStatus2);
    }

    int ChargerBQ25703A::GetVBUS(float &val)
    {
        uint8_t regval;
        int err;
        if ((err = i2c->Read8(regval, DEV_ADDR, adcVBUS)) == 0)
            val = regval * 0.064 + 3.2;
        return err;
    }

    int ChargerBQ25703A::GetPSYS(float &val)
    {
        uint8_t regval;
        int err;
        if ((err = i2c->Read8(regval, DEV_ADDR, adcPSYS)) == 0)
            val = regval * 0.012;
        return err;
    }

    int ChargerBQ25703A::GetIIN(float &val)
    {
        uint8_t regval;
        int err;
        if ((err = i2c->Read8(regval, DEV_ADDR, adcIIN)) == 0)
            val = regval * 0.050;
        return err;
    }

    int ChargerBQ25703A::GetIDCHG(float &val)
    {
        uint8_t regval;
        int err;
        if ((err = i2c->Read8(regval, DEV_ADDR, adcIDCHG)) == 0)
            val = regval * 0.256;
        return err;
    }

    int ChargerBQ25703A::GetICHG(float &val)
    {
        uint8_t regval;
        int err;
        if ((err = i2c->Read8(regval, DEV_ADDR, adcICHG)) == 0)
            val = regval * 0.064;
        return err;
    }

    int ChargerBQ25703A::GetVSYS(float &val)
    {
        uint8_t regval;
        int err;
        if ((err = i2c->Read8(regval, DEV_ADDR, adcVSYS)) == 0)
            val = regval * 0.064 + 2.88;
        return err;
    }

    int ChargerBQ25703A::GetVBAT(float &val)
    {
        uint8_t regval;
        int err;
        if ((err = i2c->Read8(regval, DEV_ADDR, adcVBAT)) == 0)
            val = regval * 0.064 + 2.88;
        return err;
    }


    int ChargerBQ25703A::GetOption(uint16_t &val)
    {
//        return i2c->Read16(val, DEV_ADDR, maxChargeVoltageReg2);
//        return i2c->Read16(val, DEV_ADDR, chargeCurrentReg2);
        return i2c->Read16(val, DEV_ADDR, minSystemVoltageReg2);
    }

    int ChargerBQ25703A::SetOption(uint16_t val)
    {
        return i2c->Write16(val, DEV_ADDR, chargerOption1reg2);
    }
