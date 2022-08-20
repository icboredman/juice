//
//

// <code>


#include <iostream> // cin, cout
#include <iomanip>
#include <thread>
#include <chrono>
#include "i2c.hpp"
#include "cw2015.hpp"
#include "bq25703a.hpp"
#include <mqtt/async_client.h>
#include "settings.hpp"

using namespace std;

const string SERVER_ADDRESS { "tcp://localhost:1883" };
//const string TOPIC { "juice" };
const int QOS = 0; // at most once

const auto TIMEOUT = std::chrono::seconds(10);

typedef struct
{
    struct sGauge {
        float VBat;
        float SoC;
    } gauge;
    struct sCharger {
        float VBus;
        float VSys;
        float VBat;
        float IIn;
        float IChg;
        float IDchg;
    } charger;
    struct sStat {
        bool source;
        bool charging;
        bool fastCharge;
        bool preCharge;
        uint8_t faults;
    } status;
} tPowerData;

tPowerData juice;

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

    Settings::RetrieveCluster();
    string devstr = to_string(Settings::Cluster()->unit_id);
    uint zeros = 5 - min((size_t)5, devstr.size());
    string topic = "aria/fish" + string(zeros, '0').append(devstr) + "/fish/juice";

    cout << "Initializing for server '" << SERVER_ADDRESS << "'...";
    mqtt::async_client client(SERVER_ADDRESS, "");
    cout << "  OK" << endl;

    try
    {
        cout << "\nConnecting...";
        client.connect()->wait();
        cout << "  OK" << endl;

        cout << "\nPublishing messages..." << endl;

        while (1)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            // collect data
            float value;
            int err;

            if ((err = gauge.GetVBat(value)) != 0)
                cout << "Gauge VBat Error: " << err << endl;
            else
                juice.gauge.VBat = value;

            if ((err = gauge.GetSoC(value)) != 0)
                cout << "Gauge SOC Error: " << err << endl;
            else
                juice.gauge.SoC = value;

            charger.Update(true);

            memcpy(&juice.status, &charger.status, sizeof(juice.status));

            if ((err = charger.GetVBUS(value)) != 0)
                cout << "Charger VBus Error: " << err << endl;
            else
                juice.charger.VBus = value;

            if ((err = charger.GetVBAT(value)) != 0)
                cout << "Charger VBat Error: " << err << endl;
            else
                juice.charger.VBat = value;

            if ((err = charger.GetVSYS(value)) != 0)
                cout << "Charger VSys Error: " << err << endl;
            else
                juice.charger.VSys = value;

            if ((err = charger.GetIIN(value)) != 0)
                cout << "Charger IIn Error: " << err << endl;
            else
                juice.charger.IIn = value;

            if ((err = charger.GetIDCHG(value)) != 0)
                cout << "Charger IDchg Error: " << err << endl;
            else
                juice.charger.IDchg = value;

            if ((err = charger.GetICHG(value)) != 0)
                cout << "Charger IChg Error: " << err << endl;
            else
                juice.charger.IChg = value;

            // publish data over MQTT
            mqtt::message_ptr pubmsg = mqtt::make_message(topic, &juice, sizeof(juice));
            pubmsg->set_qos(QOS);
            client.publish(pubmsg)->wait_for(TIMEOUT);
            //cout << "  ...OK" << endl;
        }
    }
    catch (const mqtt::exception& exc)
    {
        cerr << exc.what() << endl;
        return 1;
    }

    return 0;
}
// </code>
