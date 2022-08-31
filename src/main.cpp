
/*
   VE.Direct to CAN BUS & MQTT Gateway using a ESP32 Board
   Collect Data from VE.Direct device like Victron MPPT 75/15 / Smart Shunt
   and send it to a an inverter and MQTT gateway. From there you can
   integrate the data into any Home Automation Software like 
   ioBroker annd make graphs.

   The ESP32 will read data from the VE.Direct interface and transmit the
   data via WiFi to a MQTT broker and via CAN Bus to an inverter, it supports
   the basic profile and Pylontech protocol.

   GITHUB Link

   MIT License

   Copyright (c) 2020 Ralf Lehmann

   
   Copyright (c) 2022 Simon Jones 

   Implemented new Wifi & MQTT Library supporting OTA on device no web server is required
   use: http://IPAddress/OTA and browse to the bin file and update
   Implemented CAN Bus support using MCP2515 chip and library from:
    
   

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/


/*
   All configuration comes from config.h
   So please see there for WiFi, MQTT and OTA configuration
*/
#include <Arduino.h>
#include "config.h"

#include "EspMQTTClient.h"

EspMQTTClient client(
    ssid,
    pw,
    mqtt_server,
    mqtt_username,    // Omit this parameter to disable client authentification
    mqtt_password,    // Omit this parameter to disable client authentification
    mqtt_clientID,
    mqtt_port);

#include "TimeLib.h"
#include "vedirectEEPROM.h"
mEEPROM pref;
#include "VEDirect.h"

time_t last_boot;

#ifdef USE_CANBUS
#include <SPI.h>
#include <mcp_can.h>              // Library for CAN Interface      https://github.com/coryjfowler/MCP_CAN_lib
#include "CANBUSBMS.h" 
uint32_t SendCanBusMQTTUpdates;
#endif

#include "vedirectMQTT.h"

#ifdef USE_ONEWIRE
#include "vedirectONEWIRE.h"
#endif

VEDirect ve;
time_t last_vedirect;


void setup() {
  Serial.begin(115200);

#ifdef USE_OTA
  client.enableHTTPWebUpdater("/ota");
  client.enableOTA(mqtt_password,8266);
#endif

  // fetch previously stored parameters from EEPROM
  //VE_WAIT_TIME = pref.getInt("VE_WAIT_TIME", VE_WAIT_TIME);
//#ifdef USE_OTA
  //OTA_WAIT_TIME = pref.getInt("OTA_WAIT_TIME", OTA_WAIT_TIME);
//#endif
//#ifdef USE_ONEWIRE
  //OW_WAIT_TIME = pref.getInt("OW_WAIT_TIME", OW_WAIT_TIME);
//#endif

//Wait here while wifi is connecting
  client.loop();
  log_i("Entering loop to wait for WiFi connection");
  while (!client.isWifiConnected()){
    delay(25);
    client.loop();
    }

  if (client.isWifiConnected()) {
 //   setClock();
 //   last_boot = time(nullptr);
    if (client.isWifiConnected()
#ifdef USE_CANBUS
  && CAN_Begin()
#endif
    ) {

#ifdef USE_CANBUS
      CAN_SetChargeVoltage(initBattChargeVoltage);
      CAN_SetChargeCurrent(initBattChargeCurrent);
      CAN_SetDischargeVoltage(initBattDischargeVoltage);
      CAN_SetDischargeCurrent(initBattDischargeCurrent);
      CAN_SetBattCapacity(initBattCapacity);
#ifdef USE_PYLONTECH
      CAN_EnablePylonTech(true);
#endif
      SendCanBusMQTTUpdates = millis();
#endif
      ve.begin();
      // looking good; moving to loop
      return;
    }
  }
  // oh oh, we did not get WiFi or MQTT (CANBUS if enabled), that is bad; we can't continue
  // wait a while and reboot to try again
  delay(5000);
  ESP.restart();
}

void loop() {
  VEDirectBlock_t block;
  time_t t = time(nullptr);
  // MQTT Processing loop
  client.loop();

#ifdef USE_ONEWIRE
  if ( abs(t - last_ow) >= OW_WAIT_TIME) {
    if ( checkWiFi()) {
      sendOneWireMQTT();
      last_ow = t;
      //sendOPInfo();
    }
  }
#endif

  if ( abs( t - last_vedirect) >= VE_WAIT_TIME) {
    if ( ve.getNewestBlock(&block)) {
      last_vedirect = t;
      log_i("New block arrived; Value count: %d, serial %d", block.kvCount, block.serial);
#ifdef USE_CANBUS
      UpdateCanBusData(&block);
      CAN_SendAllUpdate();
      if ( ((millis() - SendCanBusMQTTUpdates) > 15000) || CAN_DataChanged() )
      {
          log_i("Sending Switch Update Data");
          SendCanBusMQTTUpdates = millis();
          sendUpdateMQTTData();        
      }
#endif
      if (client.isMqttConnected()) {
        sendASCII2MQTT(&block);
      }
    }
  }
}
