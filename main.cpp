//
//

// <code>


#include <iostream> // cin, cout
#include <fstream>  // ifstream, ofstream
#include <iomanip>
#include <thread>
#include <chrono>
#include <systemd/sd-daemon.h>  // sd_notify
#include "i2c.hpp"
#include "cw2015.hpp"
#include "bq25703a.hpp"
#include <mqtt/async_client.h>
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

#define HEART_RATE_MS  1000

int main(int argc, char **argv)
{
    // from "https://blog.hackeriet.no/systemd-service-type-notify-and-watchdog-c/"
    // Detect if expected to notify watchdog
    uint64_t watchdogNotifyIntervalUsec = 0;
    int watchdogEnabled = sd_watchdog_enabled(0, &watchdogNotifyIntervalUsec);
    // man systemd.service recommends notifying every half time of max
    watchdogNotifyIntervalUsec = watchdogNotifyIntervalUsec / 2;
    cout << "Systemd watchdog enabled: " << watchdogEnabled << endl;
    cout << "Systemd watchdog notify interval (divided by 2): " << watchdogNotifyIntervalUsec << endl;

    I2cDriver i2c("/dev/i2c-8");

    if (!i2c.Ready())
    {
        std::cout << "Failed to init I2C communication.\n";
        return -1;
    }
    std::cout << "I2C communication successfully setup.\n";

    GaugeCW2015 gauge(i2c);
    ChargerBQ25703A charger(i2c);

    // get my unit id
    uint32_t myId = 0;
    std::ifstream idfile("/home/fish/my-unit-id.txt", std::ios::in);
    if (idfile)
        idfile >> myId;
    idfile.close();
    string devstr = to_string(myId);
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

        cout << "Initialization complete." << endl;
        // Notify systemd service that we're ready
        sd_notify(0, "READY=1");
        uint64_t elapsedUsec = 0;

        cout << "\nPublishing messages on topic: " << topic << endl;

        while (1)
        {
            auto next_run_time = chrono::system_clock::now() + chrono::milliseconds(HEART_RATE_MS);

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

            if (watchdogEnabled)
            {
                elapsedUsec += HEART_RATE_MS * 1000;
                if (elapsedUsec >= watchdogNotifyIntervalUsec)
                {// Notify systemd this service is still alive and good
//                    sd_notify(0, "WATCHDOG=1");
                    elapsedUsec = 0;
                }
            }

            this_thread::sleep_until(next_run_time);
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
