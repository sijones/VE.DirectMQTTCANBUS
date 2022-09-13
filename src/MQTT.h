void onMessageReceived(const String& topic, const String& message) 
{
  if (topic == MQTT_PREFIX + "/set/" + "DischargeCurrent") {
    Inverter.SetDischargeCurrent(message.toInt());
    client.publish(MQTT_PREFIX + "/Param/DischargeCurrent", message);
  }
  else if (topic == MQTT_PREFIX + "/set/" + "ChargeVoltage") {
    if (message.toInt() > 0) {
      Inverter.SetChargeVoltage(message.toInt());
      client.publish(MQTT_PREFIX + "/Param/ChargeVoltage", message);
    }
  }
  else if (topic == MQTT_PREFIX + "/set/" + "ChargeCurrent") {
   Inverter.SetChargeCurrent(message.toInt());
   client.publish(MQTT_PREFIX + "/Param/ChargeCurrent", message);
  }
  else if (topic == MQTT_PREFIX + "/set/" + "ForceCharge") {
    Inverter.ForceCharge((message == "ON") ? true : false);
    client.publish(MQTT_PREFIX + "/Param/ForceCharge", (Inverter.ForceCharge() == true) ? "ON" : "OFF" ); }
  else if (topic == MQTT_PREFIX + "/set/" + "DischargeEnable") {
    Inverter.DischargeEnable((message == "ON") ? true : false); 
    client.publish(MQTT_PREFIX + "/Param/DischargeEnable", (Inverter.DischargeEnable() == true) ? "ON" : "OFF" ); }
  else if (topic == MQTT_PREFIX + "/set/" + "ChargeEnable") {
    Inverter.ChargeEnable((message == "ON") ? true : false); 
    client.publish(MQTT_PREFIX + "/Param/ChargeEnable", (Inverter.ChargeEnable() == true) ? "ON" : "OFF" ); }
  else if (topic == MQTT_PREFIX + "/set/" + "EnablePYLONTECH") {
    Inverter.EnablePylonTech((message == "ON") ? true : false); 
    client.publish(MQTT_PREFIX + "/Param/EnablePYLONTECH", (Inverter.EnablePylonTech() == true) ? "ON" : "OFF" ); }
  else {
    client.publish(MQTT_PREFIX + "/LastMessage", "Command not recognised, Topic: " + topic + " - Payload: " + message);
  }
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
  return true;
}

bool sendUpdateMQTTData()
{
  if (client.isMqttConnected()){
    client.publish(MQTT_PREFIX + "/Param/EnablePYLONTECH", (Inverter.EnablePylonTech() == true) ? "ON" : "OFF" );
    client.publish(MQTT_PREFIX + "/Param/ForceCharge", (Inverter.ForceCharge() == true) ? "ON" : "OFF" );  
    client.publish(MQTT_PREFIX + "/Param/DischargeEnable", (Inverter.DischargeEnable() == true) ? "ON" : "OFF" ); 
    client.publish(MQTT_PREFIX + "/Param/ChargeEnable", (Inverter.ChargeEnable() == true) ? "ON" : "OFF" ); 
    return true;
  } else  
    return false;
}

void onConnectionEstablished()
{
  log_i("Wifi Connected");
  client.subscribe(MQTT_PREFIX + "/set/#", onMessageReceived);
}
