//
//

// <code>


#include <iostream> // cin, cout
#include <iomanip>
#include <thread>
#include <chrono>
//#include <signal.h>
#include "i2c.hpp"
#include "cw2015.hpp"
#include "bq25703a.hpp"


int main(int argc, char **argv)
{

    I2cDriver i2c("/dev/i2c-8");

    if (!i2c.Ready())
    {
        std::cout << "Failed to init I2C communication.\n";
        return -1;
    }
    std::cout << "I2C communication successfully setup.\n";

    GaugeCW2015 gauge(i2c);
    ChargerBQ25703A charger(i2c);

    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        float value;
        int err;

        std::cout << std::endl;
        std::cout << "--- Gauge ------------------" << std::endl;

        if ((err = gauge.GetVBat(value)) != 0)
            std::cout << "VBat Error: " << err << std::endl;
        else
            std::cout << "VBat: " << value << std::endl;

        if ((err = gauge.GetSoC(value)) != 0)
            std::cout << "SOC Error: " << err << std::endl;
        else
            std::cout << "SOC: " << value << std::endl;

        std::cout << "--- Charger ----------------" << std::endl;
        charger.Update(true);
        uint16_t stat;
        if ((err = charger.GetStatus(stat)) != 0)
            std::cout << "Status Error: " << err << std::endl;
        std::cout << "Status: (" << std::hex << stat << std::dec << ")" << std::endl;
        std::cout << "   Src Chr F P faults" << std::endl;
        std::cout << "   ------------------" << std::endl;
        std::cout << "   " <<
                         (int)charger.status.source << "   " <<
                         (int)charger.status.charging << "   " <<
                         (int)charger.status.fastCharge << " " <<
                         (int)charger.status.preCharge << " " <<
                         std::hex << (int)charger.status.faults << std::dec << std::endl;

        if ((err = charger.GetVBUS(value)) != 0)
            std::cout << "Error: " << err << std::endl;
        else
            std::cout << "VBUS: " << value << " V" << std::endl;

        if ((err = charger.GetPSYS(value)) != 0)
            std::cout << "Error: " << err << std::endl;
        else
            std::cout << "PSYS: " << value << " W" << std::endl;

        if ((err = charger.GetIIN(value)) != 0)
            std::cout << "Error: " << err << std::endl;
        else
            std::cout << "IIN:  " << value << " A" << std::endl;

        if ((err = charger.GetIDCHG(value)) != 0)
            std::cout << "Error: " << err << std::endl;
        else
            std::cout << "IDCH: " << value << " A" << std::endl;

        if ((err = charger.GetICHG(value)) != 0)
            std::cout << "Error: " << err << std::endl;
        else
            std::cout << "ICHG: " << value << " A" << std::endl;

        if ((err = charger.GetVSYS(value)) != 0)
            std::cout << "Error: " << err << std::endl;
        else
            std::cout << "VSYS: " << value << " V" << std::endl;

        if ((err = charger.GetVBAT(value)) != 0)
            std::cout << "Error: " << err << std::endl;
        else
            std::cout << "VBAT: " << value << " V" << std::endl;
    }

    return 0;
}
// </code>
