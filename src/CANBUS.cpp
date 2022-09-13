#include "CANBUS.h"

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
    log_i("CAN Bus Failed to Initialise");
    _initialised = false;
    return false;
  }
  return true;
}

bool CANBUS::SendAllUpdates()
{
  // Turn off force charge, this is defined in PylonTech Protocol
  if (_battSOC > 96 && _forceCharge){
    ForceCharge(false);
  }

  if (_battCapacity > 0){
    if(_battSOC > 95)
      _chargeCurrentmA = (_battCapacity / 20);
    else if(_battSOC > 90)
      _chargeCurrentmA = (_battCapacity / 10);
    else
      _chargeCurrentmA = _maxChargeCurrentmA;
  }
  if (Initialised() && AllReady() && ((millis() - LoopTimer) > _CanBusSendInterval))
  {
    if (SendParamUpdate() && SendBattUpdate()) {
      LoopTimer = millis();
      return true;
      } else return false;
  } 
  else return false;
}

bool CANBUS::SendBattUpdate()
{
  return SendBattUpdate(_battSOC,_battVoltage,_battCurrentmA, _battTemp, _battSOH);
}

bool CANBUS::SendBattUpdate(uint8_t SOC, uint16_t Voltage, int32_t CurrentmA, int16_t BattTemp, uint8_t SOH = 100)
{
  byte sndStat;
  // Send SOC and SOH first
  if (!Initialised() && !AllReady()) return false;

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

  sndStat = CAN->sendMsgBuf(0x355, 0, 4, CAN_MSG);
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

  sndStat = CAN->sendMsgBuf(0x356, 0, 8, CAN_MSG);

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

    sndStat = CAN->sendMsgBuf(0x359, 0, 8, CAN_MSG);
    delay(_canSendDelay); 

    //0x35C – C0 00 – Battery charge request flags
    CAN_MSG[0] = 0xC0;
    CAN_MSG[1] = 0x00;
    if (_forceCharge) CAN_MSG[1] || bmsForceCharge;
    if (_chargeEnabled) CAN_MSG[1] || bmsChargeEnable;
    if (_dischargeEnabled) CAN_MSG[1] || bmsDischargeEnable;
    CAN_MSG[2] = 0x00;
    CAN_MSG[3] = 0x00;
    CAN_MSG[4] = 0x00;
    CAN_MSG[5] = 0x00;
    CAN_MSG[6] = 0x00;
    CAN_MSG[7] = 0x00;

    sndStat = CAN->sendMsgBuf(0x35C, 0, 2, CAN_MSG);
    delay(_canSendDelay); 
  }
  return true;
}


void CANBUS::SetChargeVoltage(uint32_t Voltage){
  _initialChargeVoltage = true; 
  if(_chargeVoltage != Voltage) {
    _dataChanged = true;
    _chargeVoltage = Voltage;
    }
  }

void CANBUS::SetChargeCurrent(uint32_t CurrentmA){
  if(!_initialChargeCurrent) {
    _maxChargeCurrentmA = CurrentmA; 
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

bool CANBUS::DataChanged(){
  if (_dataChanged) {
    _dataChanged = false;
    return true;
  } else return false;
}



bool CANBUS::SendParamUpdate(){

  byte sndStat;

  if (!Initialised() && !AllReady()) return false;
  
  // Send PYLON String if enabled
  if(_enablePYLONTECH) {
    sndStat = CAN->sendMsgBuf(0x35E, 0, 8, MSG_PYLON);
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

  sndStat = CAN->sendMsgBuf(0x351, 0, 8, CAN_MSG);

  if(sndStat == CAN_OK){
    log_i("Inverter Parameters update via CAN Bus sent.");
  } else
  {
    log_i("Inverter Parameters update via CAN Bus failed.");
  }

  return true;

}
