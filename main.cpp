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

    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        float value;
        int err;

        if ((err = gauge.GetVBat(value)) != 0)
            std::cout << "VBat Error: " << err << std::endl;
        else
            std::cout << "VBat: " << value << std::endl;

        if ((err = gauge.GetSoC(value)) != 0)
            std::cout << "SOC Error: " << err << std::endl;
        else
            std::cout << "SOC: " << value << std::endl;
    }

    return 0;
}
// </code>
