# This code will no longer be maintained, please use https://github.com/sijones/DiyBatteryBMS.git 

VE.Direct2MQTTCANBUS takes data from a Victron Smart Shunt and sends it to a inverter over CAN allowing for "DIY LifePO4" Batteries to be integrated.

The data is also sent over MQTT and allows commands to be sent back to control Charge/Discharge/Force Charge.

This software uses a ESP32 developers board with a MCP2515 Can Bus adapter currently developed using Visual Studio code.

The software sends the data in Pylontech Protocol, most inverters should support this.

- See also: https://www.victronenergy.com/live/vedirect_protocol:faq

With the help of the MQTT server you can integrate the monitoring data to virtually any Home Automation System. I use Home Assistant to automate off peak battery charging (using Force Charge) and can also enable and disable the charging and discharging.

## Features
- Listen to VE.Direct messages and publish a block (consisting of several key-value pairs) to a MQTT broker<br>Every key from the device will be appended to the MQTT_PREFIX and build a topic. e.g. MQTT_PREFIX="/SMARTBMS"; Topic /SMARTBMS/V will contain the Battery Voltage<br> so please see the VE.Direct protocol for the meaning of topics
- Supports MQTT Commands to enable and disable charge/discharging of an inverter, force charge the batteries to be able to charge over night at off peak rates. See the home assistant file for the commands and config.
- SSL is currently disabled
- Supports single MQTT server
- OneWire temperature sensors will be supported in a future version
- OTA (Over The Air Update)<br> use your browser and go to http://IPADDRESS/ota and upload the lastest binary.
- One config file to enable/disable features and configure serial port or MQTT Topics
- Works with both Victron Smart Shunt and BMV hardware

## Limitations
- VE.Direct2MQTT is only listening to messages of the VE.Direct device<br>It understands only the "ASCII" part of the protocol that is only good to receive a set of values. You can't request any special data or change any parameters of the VE.Direct device.<br>

## Hardware & Software Installation
See the Wiki page

## Disclaimer
I WILL NOT BE HELD LIABLE FOR ANY DAMAGE THAT YOU DO TO YOU OR ONE OF YOUR DEVICES.
