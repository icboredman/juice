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

#include "powerdata.pb.h"

using namespace std;

const string SERVER_ADDRESS { "tcp://localhost:1883" };
//const string TOPIC { "juice" };
const int QOS = 0; // at most once

const auto TIMEOUT = std::chrono::seconds(10);

typedef struct alignas(4)
{
    struct alignas(4) sGauge {
        float VBat;
        float SoC;
    } gauge;
    struct alignas(4) sCharger {
        float VBus;
        float VSys;
        float VBat;
        float IIn;
        float IChg;
        float IDchg;
    } charger;
    struct alignas(1) sStat {
        bool source;
        bool charging;
        bool fastCharge;
        bool preCharge;
        uint16_t faults;
    } status;
} tPowerData;

tPowerData juice_legacy;

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

    protopower::Gauge juice_gauge;

    try
    {
        cout << "\nConnecting...";
        client.connect()->wait();
        cout << "  OK" << endl;

        cout << "\nPublishing messages on topic: " << topic << endl;

        while (1)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            // collect data
            float value;
            int err;

            if ((err = gauge.GetVBat(value)) != 0)
                cout << "Gauge VBat Error: " << err << endl;
            else
            {
                juice_legacy.gauge.VBat = value;
                juice_gauge.set_vbat(value);
            }

            if ((err = gauge.GetSoC(value)) != 0)
                cout << "Gauge SOC Error: " << err << endl;
            else
            {
                juice_legacy.gauge.SoC = value;
                juice_gauge.set_soc(value);
            }

            charger.Update(true);

            memcpy(&juice_legacy.status, &charger.status, sizeof(juice_legacy.status));
            juice_gauge.set_charging(juice_legacy.status.charging);

            if ((err = charger.GetVBUS(value)) != 0)
                cout << "Charger VBus Error: " << err << endl;
            else
                juice_legacy.charger.VBus = value;

            if ((err = charger.GetVBAT(value)) != 0)
                cout << "Charger VBat Error: " << err << endl;
            else
                juice_legacy.charger.VBat = value;

            if ((err = charger.GetVSYS(value)) != 0)
                cout << "Charger VSys Error: " << err << endl;
            else
                juice_legacy.charger.VSys = value;

            if ((err = charger.GetIIN(value)) != 0)
                cout << "Charger IIn Error: " << err << endl;
            else
                juice_legacy.charger.IIn = value;

            if ((err = charger.GetIDCHG(value)) != 0)
                cout << "Charger IDchg Error: " << err << endl;
            else
                juice_legacy.charger.IDchg = value;

            if ((err = charger.GetICHG(value)) != 0)
                cout << "Charger IChg Error: " << err << endl;
            else
                juice_legacy.charger.IChg = value;

            // publish data over MQTT
            string msg_data = juice_gauge.SerializeAsString();
            mqtt::message_ptr pubmsg = mqtt::make_message(topic, msg_data);
            pubmsg->set_qos(QOS);
            client.publish(pubmsg)->wait_for(TIMEOUT);

            // publish internal data using legacy format
            pubmsg = mqtt::make_message("internal/juice", &juice_legacy, sizeof(juice_legacy));
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
