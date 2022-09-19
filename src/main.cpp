
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
#include "FS.h"
#include "SPIFFS.h"
#include "config.h"

#include "EspMQTTClient.h"
#include "EEPROM.h"
mEEPROM pref;

EspMQTTClient client(
    ssid,
    pw,
    mqtt_server,
    mqtt_username,    // Omit this parameter to disable client authentification
    mqtt_password,    // Omit this parameter to disable client authentification
    mqtt_clientID,
    mqtt_port);

#include "TimeLib.h"
#include "VEDirect.h"
#include "CANBUS.h" 
uint32_t SendCanBusMQTTUpdates;
CANBUS Inverter;

#include "MQTT.h"

#ifdef USE_ONEWIRE
#include "ONEWIRE.h"
#endif

time_t last_boot;
VEDirect ve;
//time_t last_vedirect;
uint32_t last_vedirect_millis;

void UpdateCanBusData(VEDirectBlock_t * block) {
  for (int i = 0; i < block->kvCount; i++) {
    bool dataValid = false;
    String key = block->b[i].key;
    String value = block->b[i].value;
    String parsedValue = "";
    if (value.startsWith("-"))
      parsedValue = "-";

    for (auto x : value)
    {
      if (isDigit(x))
        parsedValue += x;
    }
    if (parsedValue.length() > 0)
      dataValid = true;

    //int intValue = parsedValue.toInt();
    //if ( espMQTT.publish(topic.c_str(), value.c_str())) {
    //  log_i("MQTT message sent succesfully: %s: \"%s\"", topic.c_str(), value.c_str());
    //} else {
    //  log_e("Sending MQTT message failed: %s: %s", topic.c_str(), value.c_str());
    //}

    if (key.compareTo(String('V')) == 0)
    {
      log_i("Battery Voltage Update: %sV", parsedValue.c_str());
      if (dataValid) Inverter.BattVoltage((uint16_t) round(parsedValue.toInt() * 0.1));
    }
    
    if (key.compareTo(String('I')) == 0)
    {
      log_i("Battery Current Update: %smA",parsedValue.c_str());
      if (dataValid) Inverter.BattCurrentmA((int32_t) (parsedValue.toInt() *0.01 ));
    }

    if (key.compareTo(String("SOC")) == 0)
    {
      log_i("Battery SOC Update: %s%%",parsedValue.c_str());
      if (dataValid) Inverter.BattSOC((uint8_t) round((parsedValue.toInt()*0.1)));
    }
    
   /* if (key.compareTo(String('SOC')) == 0)
    {
      log_i("Battery Temp Update: %sC",parsedValue.c_str());
      BattTemp((uint16_t) (parsedValue.toInt()*0.1));
    } */
    

  }
}



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
/*  client.loop();
  log_i("Entering loop to wait for WiFi connection");
  while (!client.isWifiConnected()){
    delay(25);
    client.loop();
    }
*/
  //if (client.isWifiConnected()) {
 //   setClock();
 //   last_boot = time(nullptr);
    if (Inverter.Begin(CAN_CS_PIN)) {
      Inverter.SetChargeVoltage(initBattChargeVoltage);
      Inverter.SetChargeCurrent(initBattChargeCurrent);
      Inverter.SetDischargeVoltage(initBattDischargeVoltage);
      Inverter.SetDischargeCurrent(initBattDischargeCurrent);
      Inverter.SetBattCapacity(initBattCapacity);
#ifdef USE_PYLONTECH
      Inverter.EnablePylonTech(true);
#endif
      SendCanBusMQTTUpdates = millis();
      last_vedirect_millis = millis();
      ve.begin();
      // looking good; moving to loop
      return;
    }
 // }
  // oh oh, we did not get CANBUS, that is bad; we can't continue
  // wait a while and reboot to try again
  delay(5000);
  ESP.restart();
}

void loop() {
  VEDirectBlock_t block;
  //time_t t = time(nullptr);
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

  //if ( abs( t - last_vedirect) >= VE_WAIT_TIME) {
  if ((millis() - last_vedirect_millis) >= VE_WAIT_TIME_MS) {
    if ( ve.getNewestBlock(&block)) {
      //last_vedirect = t;
      last_vedirect_millis = millis();
      log_i("New block arrived; Value count: %d, serial %d", block.kvCount, block.serial);
      UpdateCanBusData(&block);
      // The send CAN Bus data is handled in a task every second.
      //Inverter.SendAllUpdates();
      if ( ((millis() - SendCanBusMQTTUpdates) > 15000) || Inverter.DataChanged() )
      {
          log_i("Sending Switch Update Data");
          SendCanBusMQTTUpdates = millis();
          sendUpdateMQTTData();        
      }

      if (client.isMqttConnected()) {
        sendASCII2MQTT(&block);
      }
    }
    // If CAN Bus has failed to send to many packets we reboot
    if (Inverter.CanBusFailed()){
      log_e("Can Bus has too many failed sending events, rebooting.");
      delay(50);
      ESP.restart();
    }
    
  }
}
