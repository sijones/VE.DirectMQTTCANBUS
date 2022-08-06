/*
PYLON Protocol, messages sent every 1 second.

0x351 – 14 02 74 0E 74 0E CC 01 – Battery voltage + current limits
0x355 – 1A 00 64 00 – State of Health (SOH) / State of Charge (SOC)
0x356 – 4e 13 02 03 04 05 – Voltage / Current / Temp
0x359 – 00 00 00 00 0A 50 4E – Protection & Alarm flags
0x35C – C0 00 – Battery charge request flags
0x35E – 50 59 4C 4F 4E 20 20 20 – Manufacturer name (“PYLON   “)

*/

#ifdef USE_CANBUS
//#pragma once
//#include <Arduino.h>
//#define CAN_INT 22              //CAN Init Pin for M5Stack
//#define CAN_CS_PIN 2           //CAN CS PIN

#include <mcp_can.h>                        // Library for CAN Interface      https://github.com/coryjfowler/MCP_CAN_lib
#include <SPI.h>

  // CAN BUS Library
MCP_CAN CAN(CAN_CS_PIN);
uint8_t CAN_MSG[7];
uint8_t MSG_PYLON[] = {0x50,0x59,0x4C,0x4F,0x4E,0x20,0x20,0x20};

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
uint8_t _battSOC;
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


uint32_t LoopTimer; // store current time
// The interval for sending the inverter updated information
// Normally around 5 seconds is ok, but for Pylontech protocol it's around every second.
uint16_t _CanBusSendInterval = 1000; 

enum Command
{
  ChargeDischargeLimits = 0x351,
  BattVoltCurrent = 0x356,
  StateOfCharge = 0x355
};

  //void CANBUSBMS();
bool CAN_Begin();
bool CAN_SendBattUpdate(uint8_t SOC, uint16_t Voltage, int32_t CurrentmA, int16_t BattTemp, uint8_t SOH);
bool CAN_SendAllUpdates();
bool CAN_SendBattUpdate();
bool CAN_SendParamUpdate();
void CAN_ForceCharge(bool);
bool CAN_Initialised(void){return _initialised;}
// Next function makes sure we only start sending data to inverter when we have configuration and valid battery data
bool CAN_AllReady(){
                    if (_initialDone) return true;
                    else if (_initialBattSOC && _initialBattVoltage && _initialBattCurrent &&
                    _initialChargeVoltage && _initialChargeCurrent && _initialDischargeVoltage && _initialDischargeCurrent)
                    _initialDone = true;
                    else return false;}

void CAN_BattSOC(uint8_t soc){_initialBattSOC = true; _battSOC = soc;}
void CAN_BattVoltage(uint16_t voltage){_initialBattVoltage = true; _battVoltage = voltage;}
void CAN_BattSOH(uint8_t soh){_battSOH = soh;}
void CAN_BattCurrentmA(int32_t currentmA){_initialBattCurrent = true; _battCurrentmA = currentmA;}
void CAN_BattTemp(int16_t batttemp){_battTemp = batttemp;}
void CAN_SetBattCapacity(uint32_t BattCapacity){_battCapacity = BattCapacity;} 
void CAN_EnablePylonTech(bool State) {_enablePYLONTECH = State;} 

uint8_t CAN_BattSOC(){return _battSOC;}
uint16_t CAN_BattVoltage(){return _battVoltage;}
uint8_t CAN_BattSOH(){return _battSOH;}
int32_t CAN_BattCurrentmA(){return _battCurrentmA;}
int16_t CAN_BattTemp(){return _battTemp;} 
bool CAN_ForceCharge(){return _forceCharge;}
bool CAN_ChargeEnable(){return _chargeEnabled;}
bool CAN_DischargeEnable(){return _dischargeEnabled;}
bool CAN_EnablePylonTech(){return _enablePYLONTECH;}

void CAN_SetChargeVoltage(uint32_t Voltage){
  _initialChargeVoltage = true; 
  if(_chargeVoltage != Voltage) {
    _dataChanged = true;
    _chargeVoltage = Voltage;
    }
  }

void CAN_SetChargeCurrent(uint32_t CurrentmA){
  if(!_initialChargeCurrent) _maxChargeCurrentmA = CurrentmA;
  _initialChargeCurrent = true;
  if (_chargeCurrentmA != CurrentmA) { 
    _dataChanged = true;
    _chargeCurrentmA = CurrentmA;
  }
}

void CAN_SetDischargeVoltage(uint32_t Voltage){_initialDischargeVoltage = true; _dischargeVoltage = Voltage;}

void CAN_SetDischargeCurrent(uint32_t CurrentmA){
  if(!_initialDischargeCurrent) _maxDischargeCurrentmA = CurrentmA;;
  _initialDischargeCurrent = true; 
  if (_dischargeCurrentmA != CurrentmA) {
    _dischargeCurrentmA = CurrentmA;
    _dataChanged = true;
    }
  }

void CAN_ForceCharge(bool State) {
  if (State != _forceCharge) _dataChanged = true;
  _forceCharge = State;
  }

void CAN_ChargeEnable(bool State) {
  if (State != _chargeEnabled) _dataChanged = true;
  _chargeEnabled = State;
  }

void CAN_DischargeEnable(bool State) {
  if (State != _dischargeEnabled) _dataChanged = true;
  _dischargeEnabled = State;
  }

bool CAN_DataChanged(){
  if (_dataChanged) {
    _dataChanged = false;
    return true;
  } else return false;
}


bool CAN_Begin() {

    /*if (CAN != NULL)
    {
        delete(*CAN);
    }
   
    CAN = new MCP_CAN(CAN_CS_PIN);
*/
  log_i("CAN Bus Initialising");
  // Initialize MCP2515 running at 8MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) 
  {
    // Change to normal mode to allow messages to be transmitted
    CAN.setMode(MCP_NORMAL);  
    log_i("CAN Bus Initialised");
    _initialised = true;
    LoopTimer = millis();
    }
  else
  {
    log_i("CAN Bus Failed to Initialise");
    _initialised = false;
    return false;
  }
  return true;
}

bool CAN_SendAllUpdate()
{
  // Turn off force charge, this is defined in PylonTech Protocol
  if (_battSOC > 96 && _forceCharge){
    CAN_ForceCharge(false);
  }

  if (_battCapacity > 0){
    if(_battSOC > 95)
      _chargeCurrentmA = (_battCapacity / 18);
    else if(_battSOC > 90)
      _chargeCurrentmA = (_battCapacity / 10);
    else
      _chargeCurrentmA = _maxChargeCurrentmA;
  }
  if (CAN_Initialised() && CAN_AllReady() && ((millis() - LoopTimer) > _CanBusSendInterval))
  {
    if (CAN_SendParamUpdate() && CAN_SendBattUpdate()) {
      LoopTimer = millis();
      return true;
      } else return false;
  } 
  else return false;
}

bool CAN_SendBattUpdate()
{
  return CAN_SendBattUpdate(_battSOC,_battVoltage,_battCurrentmA, _battTemp, _battSOH);
}

bool CAN_SendBattUpdate(uint8_t SOC, uint16_t Voltage, int32_t CurrentmA, int16_t BattTemp, uint8_t SOH = 100)
{
  byte sndStat;
  // Send SOC and SOH first
  if (!CAN_Initialised() && !CAN_AllReady()) return false;

  if(_enablePYLONTECH) {
    CAN_MSG[0] = lowByte(_battSOC);
    CAN_MSG[1] = highByte(_battSOC);
  } else if (_forceCharge) {
    CAN_MSG[0] = lowByte((int8_t) 1);
    CAN_MSG[1] = highByte((int8_t) 1);
  } else {
    CAN_MSG[0] = lowByte(_battSOC);
    CAN_MSG[1] = highByte(_battSOC);
  }

  CAN_MSG[2] = lowByte(_battSOH);
  CAN_MSG[3] = highByte(_battSOH);
  CAN_MSG[4] = 0;
  CAN_MSG[5] = 0;
  CAN_MSG[6] = 0;
  CAN_MSG[7] = 0;

  sndStat = CAN.sendMsgBuf(0x355, 0, 4, CAN_MSG);
  if(sndStat == CAN_OK){
    log_i("Inverter SOC Battery update via CAN Bus sent.");
  } else {
    log_i("Inverter SOC Battery update via CAN Bus failed.");
  }
  delay(_canSendDelay);

  // Current measured values of the BMS battery voltage, battery current, battery temperature

  CAN_MSG[0] = lowByte(uint16_t(_battVoltage));
  CAN_MSG[1] = highByte(uint16_t(_battVoltage));
  CAN_MSG[2] = lowByte(uint16_t(_battCurrentmA));
  CAN_MSG[3] = highByte(uint16_t(_battCurrentmA));
  CAN_MSG[4] = lowByte(uint16_t(_battTemp * 10));
  CAN_MSG[5] = highByte(uint16_t(_battTemp * 10));
  CAN_MSG[6] = 0x00;
  CAN_MSG[7] = 0x00;

  sndStat = CAN.sendMsgBuf(0x356, 0, 8, CAN_MSG);

  if(sndStat == CAN_OK){
    log_i("Inverter Battery Voltage, Current update via CAN Bus sent.");
  } else {
    log_i("Inverter Battery Voltage, Current update via CAN Bus failed.");
  }

  delay(_canSendDelay);

  if (_enablePYLONTECH){
    //0x359 – 00 00 00 00 0A 50 4E – Protection & Alarm flags
    CAN_MSG[0] = 0x00;
    CAN_MSG[1] = 0x00;
    CAN_MSG[2] = 0x00;
    CAN_MSG[3] = 0x00;
    CAN_MSG[4] = 0x0A;
    CAN_MSG[5] = 0x50;
    CAN_MSG[6] = 0x4E;
    CAN_MSG[7] = 0x00;

    sndStat = CAN.sendMsgBuf(0x359, 0, 7, CAN_MSG);
    delay(_canSendDelay); 

    //0x35C – C0 00 – Battery charge request flags
    CAN_MSG[0] = 0xC0;
    CAN_MSG[1] = 0x00;
    if (_forceCharge) CAN_MSG[1] += bmsForceCharge;
    if (_chargeEnabled) CAN_MSG[1] += bmsChargeEnable;
    if (_dischargeEnabled) CAN_MSG[1] += bmsDischargeEnable;
    CAN_MSG[2] = 0x00;
    CAN_MSG[3] = 0x00;
    CAN_MSG[4] = 0x00;
    CAN_MSG[5] = 0x00;
    CAN_MSG[6] = 0x00;
    CAN_MSG[7] = 0x00;

    sndStat = CAN.sendMsgBuf(0x35C, 0, 2, CAN_MSG);
    delay(_canSendDelay); 
  }
  return true;
}

bool CAN_SendParamUpdate(){

  byte sndStat;

  if (!CAN_Initialised() && !CAN_AllReady()) return false;
  
  // Send PYLON String if enabled
  if(_enablePYLONTECH) {
    sndStat = CAN.sendMsgBuf(0x35E, 0, 8, MSG_PYLON);
    if (sndStat == CAN_OK){
      log_i("Sent PYLONTECH String.");
    }  else
      log_i("Failed to send PYLONTECH String.");
    delay(_canSendDelay);
  }

  // Battery charge and discharge parameters
  CAN_MSG[0] = lowByte(_chargeVoltage / 100);              // Maximum battery voltage
  CAN_MSG[1] = highByte(_chargeVoltage / 100);
  if(_chargeEnabled || _enablePYLONTECH){
    CAN_MSG[2] = lowByte(_chargeCurrentmA / 100);              // Maximum charging current 
    CAN_MSG[3] = highByte(_chargeCurrentmA / 100);
  } else {
    CAN_MSG[2] = lowByte(0);              // Maximum charging current 
    CAN_MSG[3] = highByte(0);
  }
  if(_dischargeEnabled || _enablePYLONTECH){
    CAN_MSG[4] = lowByte(_dischargeCurrentmA / 100);                 // Maximum discharge current 
    CAN_MSG[5] = highByte(_dischargeCurrentmA / 100);
  } else {
    CAN_MSG[4] = lowByte(0);                 // Maximum discharge current 
    CAN_MSG[5] = highByte(0);
  }
  CAN_MSG[6] = lowByte(_dischargeVoltage / 100);                 // Currently not used by SOLIS
  CAN_MSG[7] = highByte(_dischargeVoltage / 100);                // Currently not used by SOLIS

  sndStat = CAN.sendMsgBuf(0x351, 0, 8, CAN_MSG);

  if(sndStat == CAN_OK){
    log_i("Inverter Parameters update via CAN Bus sent.");
  } else
  {
    log_i("Inverter Parameters update via CAN Bus failed.");
  }

  return true;

}


boolean UpdateCanBusData(VEDirectBlock_t * block) {
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
      if (dataValid) CAN_BattVoltage((uint16_t) parsedValue.toInt() * 0.1);
    }
    
    if (key.compareTo(String('I')) == 0)
    {
      log_i("Battery Current Update: %smA",parsedValue.c_str());
      if (dataValid) CAN_BattCurrentmA((int32_t) (parsedValue.toInt() *0.01 ));
    }

    if (key.compareTo(String("SOC")) == 0)
    {
      log_i("Battery SOC Update: %s%%",parsedValue.c_str());
      if (dataValid) CAN_BattSOC((uint8_t) (parsedValue.toInt()*0.1));
    }
    
   /* if (key.compareTo(String('SOC')) == 0)
    {
      log_i("Battery Temp Update: %sC",parsedValue.c_str());
      CAN_BattTemp((uint16_t) (parsedValue.toInt()*0.1));
    } */
    

  }
}


#endif
