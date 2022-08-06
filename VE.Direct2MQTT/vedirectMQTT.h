void onMessageReceived(const String& topic, const String& message) 
{
#ifdef USE_CANBUS
  if (topic == MQTT_PREFIX + "/set/" + "DischargeLimit") {
    CAN_SetDischargeCurrent(message.toInt());
  }
  else if (topic == MQTT_PREFIX + "/set/" + "ChargeVoltage") {
    if (message.toInt() > 0)
      CAN_SetChargeVoltage(message.toInt());
  }
  else if (topic == MQTT_PREFIX + "/set/" + "ChargeLimit") {
   CAN_SetChargeCurrent(message.toInt());
  }
  else if (topic == MQTT_PREFIX + "/set/" + "ForceCharge") {
    CAN_ForceCharge((message == "ON") ? true : false);
    client.publish(MQTT_PREFIX + "/Param/ForceCharge", (CAN_ForceCharge() == true) ? "ON" : "OFF" ); }
  else if (topic == MQTT_PREFIX + "/set/" + "DischargeEnable") {
    CAN_DischargeEnable((message == "ON") ? true : false); 
    client.publish(MQTT_PREFIX + "/Param/DischargeEnable", (CAN_DischargeEnable() == true) ? "ON" : "OFF" ); }
  else if (topic == MQTT_PREFIX + "/set/" + "ChargeEnable") {
    CAN_ChargeEnable((message == "ON") ? true : false); 
    client.publish(MQTT_PREFIX + "/Param/ChargeEnable", (CAN_ChargeEnable() == true) ? "ON" : "OFF" ); }
  else if (topic == MQTT_PREFIX + "/set/" + "EnablePYLONTECH") {
    CAN_EnablePylonTech((message == "ON") ? true : false); 
    client.publish(MQTT_PREFIX + "/Param/EnablePYLONTECH", (CAN_EnablePylonTech() == true) ? "ON" : "OFF" ); }
  else {
    client.publish(MQTT_PREFIX + "/LastMessage", "Command not recognised Topic: " + topic + " - Payload: " + message);
  }
#endif
}

//
// Send ASCII data from passive mode to MQTT
//
bool sendASCII2MQTT(VEDirectBlock_t * block) {
  for (int i = 0; i < block->kvCount; i++) {
    String key = block->b[i].key;
    String value = block->b[i].value;
    String topic = MQTT_PREFIX + "/" + key;
    if (client.isMqttConnected()) {
      topic.replace("#", ""); // # in a topic is a no go for MQTT
      value.replace("\r\n", "");
      if ( client.publish(topic.c_str(), value.c_str())) {
        log_i("MQTT message sent succesfully: %s: \"%s\"", topic.c_str(), value.c_str());
        } else {
        log_e("Sending MQTT message failed: %s: %s", topic.c_str(), value.c_str());
        }
    }
  }
}

bool sendUpdateMQTTData()
{
  client.publish(MQTT_PREFIX + "/Param/EnablePYLONTECH", (CAN_EnablePylonTech() == true) ? "ON" : "OFF" );
  client.publish(MQTT_PREFIX + "/Param/ForceCharge", (CAN_ForceCharge() == true) ? "ON" : "OFF" );  
  client.publish(MQTT_PREFIX + "/Param/DischargeEnable", (CAN_DischargeEnable() == true) ? "ON" : "OFF" ); 
  client.publish(MQTT_PREFIX + "/Param/ChargeEnable", (CAN_ChargeEnable() == true) ? "ON" : "OFF" ); 
}

void onConnectionEstablished()
{
  log_i("Wifi Connected");
  client.subscribe(MQTT_PREFIX + "/set/#", onMessageReceived);
}
