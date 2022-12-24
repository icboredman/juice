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
#include "version.h"

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


int Average(float &val, int num)
{
    static float avg = 0;
    if (num < 1)
        return -1;
    // exponential moving average:
    avg -= avg / num;
    avg += val / num;
    val = avg;
    // cumulative average:
    //avg = avg * (num - 1)/num + val * 1/num;
    return 0;
}


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

    auto connOpts = mqtt::connect_options_builder()
        .clean_session()
        .finalize();

    protopower::Gauge juice_gauge;

    cout << "Initialization complete." << endl;
    // Notify systemd service that we're ready
    sd_notify(0, "READY=1");

    while (1)
    {
        try
        {
            cout << "\nConnecting...";
            client.connect(connOpts)->wait();
            cout << "  OK" << endl;

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
                    Average(value, 5);
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

                // store app version
                juice_gauge.set_appversion(VERSION);

                // record battery current, charging or discharging
                if (juice_gauge.charging())
                    juice_gauge.set_ibat( -juice_legacy.charger.IChg );
                else
                    juice_gauge.set_ibat( juice_legacy.charger.IDchg );

                // record temperature
                std::ifstream tzone0("/sys/class/thermal/thermal_zone0/temp", std::ios::in);
                uint32_t t0=0;
                if (tzone0)
                    tzone0 >> t0;
                tzone0.close();
                std::ifstream tzone1("/sys/class/thermal/thermal_zone1/temp", std::ios::in);
                uint32_t t1=0;
                if (tzone1)
                    tzone1 >> t1;
                tzone1.close();
                juice_gauge.set_temp( t0 > t1 ? t0/1000.0 : t1/1000.0 );

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
                        sd_notify(0, "WATCHDOG=1");
                        elapsedUsec = 0;
                    }
                }

                // safety checks
                if ( !juice_gauge.charging() && juice_gauge.vbat() > 3.0 && juice_gauge.soc() < 0.1 )
                {
                    std::cout << "Battery empty! Shutting down..." << std::endl;
                    system("shutdown -P now 'Battery empty'");
                }
                if ( juice_gauge.temp() >= 55.0 )
                {
                    std::cout << "Overheating! Shutting down..." << std::endl;
                    system("shutdown -P now 'Overheating'");
                }

                this_thread::sleep_until(next_run_time);
            }
        }
        catch (const mqtt::exception& exc)
        {
            cerr << exc.what() << endl;
            //return 1;
        }

        if (!client.is_connected())
        {
            cout << "Will attempt to reconnect in 1 sec..." << endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        else
            return 1;
    }

    return 0;
}
// </code>
