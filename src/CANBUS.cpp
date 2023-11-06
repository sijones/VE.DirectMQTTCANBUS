#include "CANBUS.h"

void canSendTask(void * pointer){
  CANBUS *Inverter = (CANBUS *) pointer;
  log_i("Starting CAN Bus send task");
  for (;;) {
    if(Inverter->SendAllUpdates())
        log_d("Success from SendAllUpdates");
      else
        log_e("Failure returned from SendAllUpdates");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

}

bool CANBUS::Begin(uint8_t _CS_PIN) {

  if (CAN != NULL)
  {
    delete(CAN);
  }
   
  CAN = new MCP_CAN(_CS_PIN);

  log_i("CAN Bus Initialising");
  // Initialize MCP2515 running at 8MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if (CAN->begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) 
  {
    // Change to normal mode to allow messages to be transmitted
    CAN->setMode(MCP_NORMAL);  
    log_i("CAN Bus Initialised");
    _initialised = true;
    LoopTimer = millis();
    }
  else
  {
    log_e("CAN Bus Failed to Initialise");
    _initialised = false;
    return false;
  }

  // Create a task to send the CAN Bus data so Wifi/MQTT doesn't
  // interfere with it.
  xTaskCreate(
    &canSendTask,    
    "canSendTask",  
    10000,            
    this,             
    2,                
    &tHandle);       

// Get the Pylontech protocol setting from EEPROM if set.
  if(pref.isKey(ccPylonTech))
    _enablePYLONTECH = pref.getBool(ccPylonTech, _enablePYLONTECH);
  return true;
}

bool CANBUS::SendAllUpdates()
{
  log_i("Sending all CAN Bus Data");
  // Turn off force charge, this is defined in PylonTech Protocol
  if (_battSOC > 96 && _forceCharge){
    ForceCharge(false);
  }

  if (_battCapacity > 0 && _initialBattData){
    if(_battSOC > 95)
      _chargeCurrentmA = (_battCapacity / 20);
    else if(_battSOC > 90)
      _chargeCurrentmA = (_battCapacity / 10);
    else
      _chargeCurrentmA = _maxChargeCurrentmA;
  }

  if(!_initialBattCurrent)
    log_i("Waiting on VE Initial Battery Current.");
  if(!_initialBattVoltage)
    log_i("Waiting on VE Initial Battery Voltage.");
  if(!_initialBattSOC)
    log_i("Waiting on VE Initial Battery SOC.");
  if(!_initialChargeCurrent)
    log_e("Initial Charge Current needs to be set.");
  if(!_initialChargeVoltage)
    log_e("Initial Charge Voltage needs to be set.");
  if(!_initialDischargeCurrent)
    log_e("Initial Discharge Current needs to be set.");
  if(!_initialDischargeVoltage)
    log_e("Initial Discharge Voltage needs to be set.");
  
  //if (Initialised() && AllReady() && ((millis() - LoopTimer) > _CanBusSendInterval))
  if (Initialised() && Configured())
  {
    if (SendParamUpdate() && SendBattUpdate()) {
      //LoopTimer = millis();
      return true;
      } else return false;
  } 
  else 
  {
    log_e("CAN Bus Data not Initialised or Configured");
    return false;
  }
    
}

bool CANBUS::SendBattUpdate()
{
  return SendBattUpdate(_battSOC,_battVoltage,_battCurrentmA, _battTemp, _battSOH);
}

bool CANBUS::SendBattUpdate(uint8_t SOC, uint16_t Voltage, int32_t CurrentmA, int16_t BattTemp, uint8_t SOH = 100)
{
  byte sndStat;
  // Send SOC and SOH first
  if (!Initialised() && !Configured()) return false;

  if(_enablePYLONTECH) {
    CAN_MSG[0] = lowByte(SOC);
    CAN_MSG[1] = highByte(SOC);
  } else if (_forceCharge) {
    CAN_MSG[0] = lowByte((int8_t) 1);
    CAN_MSG[1] = highByte((int8_t) 1);
  } else {
    CAN_MSG[0] = lowByte(SOC);
    CAN_MSG[1] = highByte(SOC);
  }

  CAN_MSG[2] = lowByte(SOH);
  CAN_MSG[3] = highByte(SOH);
  CAN_MSG[4] = 0;
  CAN_MSG[5] = 0;
  CAN_MSG[6] = 0;
  CAN_MSG[7] = 0;

  sndStat = CAN->sendMsgBuf(0x355, 0, 4, CAN_MSG);
  if(sndStat == CAN_OK){
    _failedCanSendCount = 0;
    log_i("Inverter SOC Battery update via CAN Bus sent.");
  } else {
    _failedCanSendCount++;
    log_e("Inverter SOC Battery update via CAN Bus failed.");
  }
  delay(_canSendDelay);

  // Current measured values of the BMS battery voltage, battery current, battery temperature

  CAN_MSG[0] = lowByte(uint16_t(Voltage));
  CAN_MSG[1] = highByte(uint16_t(Voltage));
  CAN_MSG[2] = lowByte(uint16_t(CurrentmA));
  CAN_MSG[3] = highByte(uint16_t(CurrentmA));
  CAN_MSG[4] = lowByte(uint16_t(BattTemp * 10));
  CAN_MSG[5] = highByte(uint16_t(BattTemp * 10));
  CAN_MSG[6] = 0x00;
  CAN_MSG[7] = 0x00;

  sndStat = CAN->sendMsgBuf(0x356, 0, 8, CAN_MSG);

  if(sndStat == CAN_OK){
    _failedCanSendCount = 0;
    log_i("Inverter Battery Voltage, Current update via CAN Bus sent.");
  } else {
    _failedCanSendCount++;
    log_e("Inverter Battery Voltage, Current update via CAN Bus failed.");
  }

  delay(_canSendDelay);

  //if (_enablePYLONTECH){
    //0x359 – 00 00 00 00 0A 50 4E – Protection & Alarm flags
    CAN_MSG[0] = 0x00;
    CAN_MSG[1] = 0x00;
    CAN_MSG[2] = 0x00;
    CAN_MSG[3] = 0x00;
    CAN_MSG[4] = 0x0A;
    CAN_MSG[5] = 0x50;
    CAN_MSG[6] = 0x4E;
    CAN_MSG[7] = 0x00;

    sndStat = CAN->sendMsgBuf(0x359, 0, 8, CAN_MSG);
    if(sndStat == CAN_OK){
      _failedCanSendCount = 0;
      log_i("Inverter Protection / Alarm Flags via CAN Bus sent.");
    } else {
      _failedCanSendCount++;
      log_e("Inverter Protection / Alarm Flags via CAN Bus failed.");
    }
    delay(_canSendDelay); 

    //0x35C – C0 00 – Battery charge request flags
    CAN_MSG[0] = 0xC0;
    CAN_MSG[1] = 0x00;
    if (_forceCharge) CAN_MSG[1] | bmsForceCharge;
    if (_chargeEnabled) CAN_MSG[1] | bmsChargeEnable;
    if (_dischargeEnabled) CAN_MSG[1] | bmsDischargeEnable;
    CAN_MSG[2] = 0x00;
    CAN_MSG[3] = 0x00;
    CAN_MSG[4] = 0x00;
    CAN_MSG[5] = 0x00;
    CAN_MSG[6] = 0x00;
    CAN_MSG[7] = 0x00;

    sndStat = CAN->sendMsgBuf(0x35C, 0, 2, CAN_MSG);
    if(sndStat == CAN_OK){
      _failedCanSendCount = 0;
      log_i("Battery Charge Flags via CAN Bus sent.");
    } else {
      _failedCanSendCount++;
      log_e("Battery Charge Flags via CAN Bus failed.");
    }
    delay(_canSendDelay); 
  //}
  return true;
}


void CANBUS::SetChargeVoltage(uint32_t Voltage){
 
  if(!_initialChargeVoltage)
  {
    _initialChargeVoltage = true;
    if(!pref.isKey(ccChargeVolt)) 
      pref.putUInt(ccChargeVolt,Voltage);
    else
      Voltage = pref.getUInt(ccChargeVolt,Voltage);
  }

  if(_chargeVoltage != Voltage) {
    _dataChanged = true;
    _chargeVoltage = Voltage;
    pref.putUInt(ccChargeVolt,Voltage);
  }

}

void CANBUS::SetChargeCurrent(uint32_t CurrentmA){
  if(!_initialChargeCurrent) {
    if(!pref.isKey(ccChargeCurrent)) {
      _maxChargeCurrentmA = CurrentmA; 
      pref.putUInt(ccChargeCurrent,CurrentmA);
    } else
    _maxChargeCurrentmA = pref.getUInt(ccChargeCurrent,CurrentmA);
    _initialChargeCurrent = true; 
  }
  else if (_chargeCurrentmA != CurrentmA && _initialDone) { 
    _dataChanged = true;
    _chargeCurrentmA = CurrentmA;
  } else
    return;
}

void CANBUS::SetDischargeVoltage(uint32_t Voltage){
  _initialDischargeVoltage = true; 
  _dischargeVoltage = Voltage;
  }

void CANBUS::SetDischargeCurrent(uint32_t CurrentmA){
  if(!_initialDischargeCurrent) {
    _maxDischargeCurrentmA = CurrentmA;
    _initialDischargeCurrent = true; 
    }
  if (_dischargeCurrentmA != CurrentmA && _initialDone) {
    _dischargeCurrentmA = CurrentmA;
    _dataChanged = true;
    }
  }

void CANBUS::ForceCharge(bool State) {
  if (State != _forceCharge) _dataChanged = true;
  _forceCharge = State;
  }

void CANBUS::ChargeEnable(bool State) {
  if (State != _chargeEnabled) _dataChanged = true;
  _chargeEnabled = State;
  }

void CANBUS::DischargeEnable(bool State) {
  if (State != _dischargeEnabled) _dataChanged = true;
  _dischargeEnabled = State;
  }
void CANBUS::EnablePylonTech(bool enable){
  pref.putBool(ccPylonTech,enable);
  _enablePYLONTECH = enable;
}

bool CANBUS::DataChanged(){
  if (_dataChanged) {
    _dataChanged = false;
    return true;
  } else return false;
}


bool CANBUS::SendParamUpdate(){

  byte sndStat;

  if (!Initialised() && !Configured()) return false;
  
  // Send PYLON String if enabled
  //if(_enablePYLONTECH) {
    sndStat = CAN->sendMsgBuf(0x35E, 0, 8, MSG_PYLON);
    if (sndStat == CAN_OK){
      log_i("Sent PYLONTECH String.");
    }  else
      log_i("Failed to send PYLONTECH String.");
    delay(_canSendDelay);
  //}

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

  sndStat = CAN->sendMsgBuf(0x351, 0, 8, CAN_MSG);

  if(sndStat == CAN_OK){
    log_i("Inverter Parameters update via CAN Bus sent.");
  } else
  {
    log_i("Inverter Parameters update via CAN Bus failed.");
  }

  return true;

}

bool CANBUS::AllReady()
{
  if (_initialDone) return true;
  else if (_initialBattSOC && _initialBattVoltage && _initialBattCurrent &&
          _initialChargeVoltage && _initialChargeCurrent && _initialDischargeVoltage && _initialDischargeCurrent)
    {
      _dischargeCurrentmA = _maxDischargeCurrentmA;
      _chargeCurrentmA = _maxChargeCurrentmA;
      _initialDone = true;
      _initialConfig = true;
      _initialBattData = true;
      return true;
    } 
  else if (_initialChargeVoltage && _initialChargeCurrent 
            && _initialDischargeVoltage && _initialDischargeCurrent &&(!_initialConfig))
    { 
      _initialConfig = true;
      return false;
    }
  else 
    return false;
}

bool CANBUS::Configured()
{
  AllReady(); // Check if we need to set the flags

  if (_initialConfig) return true;
  else if (_initialChargeVoltage && _initialChargeCurrent 
      && _initialDischargeVoltage && _initialDischargeCurrent)
      {
        _initialConfig = true;
        return true;
      }
  else return false;
}