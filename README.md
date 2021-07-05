# juice
Linux charger and fuel gauge userland driver for 2S LiPo battery.

Designed to work with ["Juice Board"](https://www.khadas.com/product-page/juice-board) module attached to [Khadas Edge-V](https://www.khadas.com/product-page/edge-v) SBC.

Publishes battery info as a MQTT topic.

MQTT client code uses Eclipse C++ library [paho.mqtt.cpp](https://github.com/eclipse/paho.mqtt.cpp)


## Notes
* Fuel Gauge IC: CW2015C
* Charger IC: BQ25703A
* Uses Linux I2C driver (ioctl, I2C_RDWR-style)
* Requires MQTT broker, such as Mosquitto
* Requires Paho C++ MQTT library installed in default location

### Compiling
* `cd build`
* `cmake ..`
* `make`

### Running
* easiest is to add link to `<path>/bin/juice` to **/etc/rc.local** file in order to launch at boot

### Example
* provided example code could be used as a basis to integrate into final application
