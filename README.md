Victron is building wonderful devices for Solar systems.
Some of the devices have a VE.Direct interface and Victron disclosed the protocol.

- See also: https://www.victronenergy.com/live/vedirect_protocol:faq

VE.Direct2MQTT is using an ESP32 developers board and the Arduino IDE to send all ASCII data coming from a VE.Direct device to a MQTT server and over CAN BUS to most inverters that support the PylonTech protocol, Full PylonTech protcol can be disabled and only the basic commands sent.

With the help of the MQTT server you can integrate the monitoring data to virtually any Home Automation System. I use Home Assistant to automate off peak battery charging (using Force Charge) and can also enable and disable the charging and discharging.

## Features
- Listen to VE.Direct messages and publish a block (consisting of several key-value pairs) to a MQTT broker<br>Every key from the device will be appended to the MQTT_PREFIX and build a topic. e.g. MQTT_PREFIX="/SMARTBMS"; Topic /SMARTBMS/V will contain the Battery Voltage<br> so please see the VE.Direct protocol for the meaning of topics
- Supports MQTT Commands to enable and disable charge/discharging of an inverter, force charge the batteries to be able to charge over night at off peak rates. See the home assistant file for the commands and config.
- SSL is currently disabled
- Supports single MQTT server
- OneWire temperature sensors will be supported in a future version
- OTA (Over The Air Update)<br> use your browser and go to http://IPADDRESS/ota and upload the lastest binary.
- One config file to enable/disable features and configure serial port or MQTT Topics


## Limitations
- VE.Direct2MQTT is only listening to messages of the VE.Direct device<br>It understands only the "ASCII" part of the protocol that is only good to receive a set of values. You can't request any special data or change any parameters of the VE.Direct device.<br>

## Hardware & Software Installation
See the Wiki page

## Disclaimer
I WILL NOT BE HELD LIABLE FOR ANY DAMAGE THAT YOU DO TO YOU OR ONE OF YOUR DEVICES.
