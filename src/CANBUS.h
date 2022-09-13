/*
PYLON Protocol, messages sent every 1 second.

0x351 – 14 02 74 0E 74 0E CC 01 – Battery voltage + current limits
0x355 – 1A 00 64 00 – State of Health (SOH) / State of Charge (SOC)
0x356 – 4e 13 02 03 04 05 – Voltage / Current / Temp
0x359 – 00 00 00 00 0A 50 4E – Protection & Alarm flags
0x35C – C0 00 – Battery charge request flags
0x35E – 50 59 4C 4F 4E 20 20 20 – Manufacturer name (“PYLON   “)

*/

#include <Arduino.h>
#include <SPI.h>
#include <mcp_can.h>              // Library for CAN Interface      https://github.com/coryjfowler/MCP_CAN_lib

class CANBUS {
  private:
//#pragma once
//#define CAN_INT 22              //CAN Init Pin for M5Stack
//#define CAN_CS_PIN 2           //CAN CS PIN

#include <mcp_can.h>                        // Library for CAN Interface      https://github.com/coryjfowler/MCP_CAN_lib
#include <SPI.h>

  // CAN BUS Library
MCP_CAN *CAN;
uint8_t CAN_MSG[7];
uint8_t MSG_PYLON[8] = {0x50,0x59,0x4C,0x4F,0x4E,0x20,0x20,0x20};

    /*
    uint8_t lowByte;
    uint8_t highByte;
    */
bool _initialised = false;
bool _enablePYLONTECH = false;
bool _forceCharge = false;
bool _chargeEnabled = true;
bool _dischargeEnabled = true;
bool _dataChanged = false;
uint8_t _canSendDelay = 5;

enum Charging {
  bmsForceCharge = 8,
  bmsDischargeEnable = 64,
  bmsChargeEnable = 128
};
// Flags set to check all data has come before starting CANBUS sending
bool _initialBattSOC = false;
bool _initialBattVoltage = false;
bool _initialBattCurrent = false;
bool _initialChargeVoltage = false;
bool _initialChargeCurrent = false;
bool _initialDischargeVoltage = false;
bool _initialDischargeCurrent = false;
bool _initialDone = false;

    // Used to tell the inverter battery data
uint8_t _battSOC = 50;
uint8_t _battSOH = 100; // State of health, not useful so defaulted to 100% 
uint16_t _battVoltage;
int32_t _battCurrentmA;
int16_t _battTemp;
uint32_t _battCapacity = 0; // Only used for limiting current at high SOC.

    // Used to tell the inverter battery limits
uint32_t _chargeVoltage = 0;
uint32_t _dischargeVoltage = 0; 
uint32_t _chargeCurrentmA = 0;
uint32_t _dischargeCurrentmA = 0;

  // These are set by the initial call to set Current Limits and used as the max.
uint32_t _maxChargeCurrentmA = 0;
uint32_t _maxDischargeCurrentmA = 0;

// Track how many failed CAN BUS sends and reboot ESP if more than limit
uint8_t _maxFailedAttempts = 10;
uint8_t _failedAttemptsCount = 0;

uint32_t LoopTimer; // store current time
// The interval for sending the inverter updated information
// Normally around 5 seconds is ok, but for Pylontech protocol it's around every second.
uint16_t _CanBusSendInterval = 1000; 

public:

  enum Command
{
  ChargeDischargeLimits = 0x351,
  BattVoltCurrent = 0x356,
  StateOfCharge = 0x355
};

  //void CANBUSBMS();
  bool Begin(uint8_t _CS_PIN);
  bool SendBattUpdate(uint8_t SOC, uint16_t Voltage, int32_t CurrentmA, int16_t BattTemp, uint8_t SOH);
  bool SendAllUpdates();
  bool SendBattUpdate();
  bool SendParamUpdate();
  bool DataChanged();
  void SetChargeVoltage(uint32_t Voltage);
  void SetChargeCurrent(uint32_t CurrentmA);
  void SetDischargeVoltage(uint32_t Voltage);
  void SetDischargeCurrent(uint32_t CurrentmA);
  void ChargeEnable(bool);
  void DischargeEnable(bool);

  void ForceCharge(bool);
  bool Initialised(void){return _initialised;}
// Next function makes sure we only start sending data to inverter when we have configuration and valid battery data
  bool AllReady(){
                    if (_initialDone) return true;
                    else if (_initialBattSOC && _initialBattVoltage && _initialBattCurrent &&
                    _initialChargeVoltage && _initialChargeCurrent && _initialDischargeVoltage && _initialDischargeCurrent)
                    {
                      _dischargeCurrentmA = _maxDischargeCurrentmA;
                      _chargeCurrentmA = _maxChargeCurrentmA;
                      _initialDone = true;
                      return true;
                    } else 
                      return false;}

void BattSOC(uint8_t soc){_initialBattSOC = true; _battSOC = soc;}
void BattVoltage(uint16_t voltage){_initialBattVoltage = true; _battVoltage = voltage;}
void BattSOH(uint8_t soh){_battSOH = soh;}
void BattCurrentmA(int32_t currentmA){_initialBattCurrent = true; _battCurrentmA = currentmA;}
void BattTemp(int16_t batttemp){_battTemp = batttemp;}
void SetBattCapacity(uint32_t BattCapacity){_battCapacity = BattCapacity;} 
void EnablePylonTech(bool State) {_enablePYLONTECH = State;} 

uint8_t BattSOC(){return _battSOC;}
uint16_t BattVoltage(){return _battVoltage;}
uint8_t BattSOH(){return _battSOH;}
int32_t BattCurrentmA(){return _battCurrentmA;}
int16_t BattTemp(){return _battTemp;} 
bool ForceCharge(){return _forceCharge;}
bool ChargeEnable(){return _chargeEnabled;}
bool DischargeEnable(){return _dischargeEnabled;}
bool EnablePylonTech(){return _enablePYLONTECH;}

}; // End of Class
