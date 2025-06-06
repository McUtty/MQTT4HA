#include "MQTT4HA.h"

MQTT4HA* MQTT4HA::instance = nullptr;

MQTT4HA::MQTT4HA(const char* clientName, const char* broker, int debug)
    : client(espClient)
{
    _clientName = clientName;
    _broker = broker;
    _debug = debug;
    instance = this;
}

void MQTT4HA::setDebug(bool debugOn) { _debug = debugOn; }

void MQTT4HA::begin(const char* mqttUser, const char* mqttPass, int port) {
    _mqttUser = mqttUser;
    _mqttPass = mqttPass;
    _port = port;
    client.setServer(_broker, _port);
    client.setCallback(_callback);
    if (_debug) Serial.println("[MQTT4HA] MQTT konfiguriert, warte auf Verbindung...");
}

void MQTT4HA::loop() {
    // 1. Verbindung sicherstellen
    if (!client.connected()) {
        _reconnect();
    }
    client.loop();

    // 2. Nach jedem neuen Connect alle Discovery-Nachrichten senden!
    if (client.connected() && !wasConnected) {
        if (_debug) Serial.println("[MQTT4HA] MQTT (re-)connected! Sende Discovery für alle Entitäten...");
        _sendAllDiscovery();
        wasConnected = true;
    }
    if (!client.connected()) wasConnected = false;
}

// Speichert die Discovery-Konfiguration und sendet sie ggf. sofort
void MQTT4HA::storeAndSendDiscovery(const String& topic, const String& payload) {
    DiscoveryMsg msg = { topic, payload };
    discoveryConfigs.push_back(msg);
    if (client.connected()) client.publish(topic.c_str(), payload.c_str(), true);
    if (_debug) {
        Serial.print("[MQTT4HA] Discovery gespeichert & (ggf. sofort) gesendet: ");
        Serial.print(topic); Serial.print(" -> "); Serial.println(payload);
    }
}

void MQTT4HA::_sendAllDiscovery() {
    for (auto& msg : discoveryConfigs) {
        if (_debug) {
            Serial.print("[MQTT4HA] (Re-)Discovery: "); Serial.print(msg.topic); Serial.print(" -> "); Serial.println(msg.payload);
        }
        client.publish(msg.topic.c_str(), msg.payload.c_str(), true);
    }
}

// ----- Discovery-Funktionen -----
void MQTT4HA::announceSensor(const char* name, const char* unit, const char* device_class) {
    String id = _clientName, topic = "homeassistant/sensor/" + id + "/" + name + "/config";
    String state_topic = id + "/" + name, unique_id = id + "_" + name;
    String payload = "{";
    payload += "\"dev\":{\"name\":\"" + id + "\",\"ids\":\"" + id + "\",\"mf\":\"DIY\",\"mdl\":\"ESP32\"},";
    payload += "\"name\":\"" + String(name) + "\",";
    payload += "\"uniq_id\":\"" + unique_id + "\",";
    payload += "\"stat_t\":\"" + state_topic + "\",";
    payload += "\"unit_of_meas\":\"" + String(unit) + "\"";
    if (String(device_class).length()) payload += ",\"dev_cla\":\"" + String(device_class) + "\"";
    payload += "}";
    storeAndSendDiscovery(topic, payload);
}
void MQTT4HA::sendSensor(const char* name, float value) {
    String topic = String(_clientName) + "/" + name, payload = String(value, 2);
    if (_debug) { Serial.print("[MQTT4HA] Sende Sensorwert: "); Serial.print(topic); Serial.print(" -> "); Serial.println(payload); }
    client.publish(topic.c_str(), payload.c_str());
}

void MQTT4HA::announceBinarySensor(const char* name, const char* device_class) {
    String id = _clientName, topic = "homeassistant/binary_sensor/" + id + "/" + name + "/config";
    String state_topic = id + "/" + name, unique_id = id + "_" + name;
    String payload = "{";
    payload += "\"dev\":{\"name\":\"" + id + "\",\"ids\":\"" + id + "\",\"mf\":\"DIY\",\"mdl\":\"ESP32\"},";
    payload += "\"name\":\"" + String(name) + "\",";
    payload += "\"uniq_id\":\"" + unique_id + "\",";
    payload += "\"stat_t\":\"" + state_topic + "\"";
    if (String(device_class).length()) payload += ",\"dev_cla\":\"" + String(device_class) + "\"";
    payload += "}";
    storeAndSendDiscovery(topic, payload);
}
void MQTT4HA::sendBinarySensor(const char* name, bool state) {
    String topic = String(_clientName) + "/" + name, payload = state ? "ON" : "OFF";
    if (_debug) { Serial.print("[MQTT4HA] Sende BinarySensor: "); Serial.print(topic); Serial.print(" -> "); Serial.println(payload); }
    client.publish(topic.c_str(), payload.c_str(), true);
}

void MQTT4HA::announceSwitch(const char* name, const char* icon) {
    String id = _clientName, topic = "homeassistant/switch/" + id + "/" + name + "/config";
    String commandTopic = id + "/" + name + "/set", stateTopic = id + "/" + name + "/state", unique_id = id + "_" + name;
    String payload = "{";
    payload += "\"dev\":{\"name\":\"" + id + "\",\"ids\":\"" + id + "\",\"mf\":\"DIY\",\"mdl\":\"ESP32\"},";
    payload += "\"name\":\"" + String(name) + "\",";
    payload += "\"uniq_id\":\"" + unique_id + "\",";
    payload += "\"stat_t\":\"" + stateTopic + "\",";
    payload += "\"cmd_t\":\"" + commandTopic + "\"";
    if (String(icon).length()) payload += ",\"ic\":\"" + String(icon) + "\"";
    payload += "}";
    storeAndSendDiscovery(topic, payload);
}
void MQTT4HA::sendSwitchState(const char* name, bool on) {
    String topic = String(_clientName) + "/" + name + "/state";
    String payload = on ? "ON" : "OFF";
    if (_debug) { Serial.print("[MQTT4HA] Sende Switch: "); Serial.print(topic); Serial.print(" -> "); Serial.println(payload); }
    client.publish(topic.c_str(), payload.c_str(), true);
}
void MQTT4HA::onSwitchCommand(const char* name, void (*callback)(bool)) {
    String topic = String(_clientName) + "/" + name + "/set";
    switches.push_back({topic, callback});
    client.subscribe(topic.c_str());
    if (_debug) { Serial.print("[MQTT4HA] Switch-Command abonniert: "); Serial.println(topic); }
}

void MQTT4HA::announceLight(const char* name, bool dimmable, bool rgb) {
    String id = _clientName, topic = "homeassistant/light/" + id + "/" + name + "/config";
    String commandTopic = id + "/" + name + "/set", stateTopic = id + "/" + name + "/state", unique_id = id + "_" + name;
    String payload = "{";
    payload += "\"dev\":{\"name\":\"" + id + "\",\"ids\":\"" + id + "\",\"mf\":\"DIY\",\"mdl\":\"ESP32\"},";
    payload += "\"name\":\"" + String(name) + "\",";
    payload += "\"uniq_id\":\"" + unique_id + "\",";
    payload += "\"stat_t\":\"" + stateTopic + "\",";
    payload += "\"cmd_t\":\"" + commandTopic + "\"";
    if (dimmable) payload += ",\"brightness\":true";
    if (rgb) payload += ",\"rgb\":true";
    payload += "}";
    storeAndSendDiscovery(topic, payload);
}
void MQTT4HA::sendLightState(const char* name, bool on, int brightness, int r, int g, int b) {
    String topic = String(_clientName) + "/" + name + "/state";
    String payload = "{";
    payload += "\"state\":\""; payload += on ? "ON" : "OFF"; payload += "\"";
    payload += ",\"brightness\":" + String(brightness);
    payload += ",\"color\":{\"r\":" + String(r) + ",\"g\":" + String(g) + ",\"b\":" + String(b) + "}";
    payload += "}";
    if (_debug) { Serial.print("[MQTT4HA] Sende Light: "); Serial.print(topic); Serial.print(" -> "); Serial.println(payload); }
    client.publish(topic.c_str(), payload.c_str(), true);
}
void MQTT4HA::onLightCommand(const char* name, void (*callback)(bool, int, int, int, int)) {
    String topic = String(_clientName) + "/" + name + "/set";
    lights.push_back({topic, callback});
    client.subscribe(topic.c_str());
    if (_debug) { Serial.print("[MQTT4HA] Light-Command abonniert: "); Serial.println(topic); }
}

void MQTT4HA::announceNumber(const char* name, float min, float max, float step, const char* unit) {
    String id = _clientName, topic = "homeassistant/number/" + id + "/" + name + "/config";
    String cmdTopic = id + "/" + name + "/set", stateTopic = id + "/" + name, unique_id = id + "_" + name;
    String payload = "{";
    payload += "\"dev\":{\"name\":\"" + id + "\",\"ids\":\"" + id + "\",\"mf\":\"DIY\",\"mdl\":\"ESP32\"},";
    payload += "\"name\":\"" + String(name) + "\",";
    payload += "\"uniq_id\":\"" + unique_id + "\",";
    payload += "\"stat_t\":\"" + stateTopic + "\",";
    payload += "\"cmd_t\":\"" + cmdTopic + "\",";
    payload += "\"min\":" + String(min) + ",";
    payload += "\"max\":" + String(max) + ",";
    payload += "\"step\":" + String(step);
    if (String(unit).length()) payload += ",\"unit_of_meas\":\"" + String(unit) + "\"";
    payload += "}";
    storeAndSendDiscovery(topic, payload);
}
void MQTT4HA::sendNumberState(const char* name, float value) {
    String topic = String(_clientName) + "/" + name, payload = String(value);
    if (_debug) { Serial.print("[MQTT4HA] Sende Number: "); Serial.print(topic); Serial.print(" -> "); Serial.println(payload); }
    client.publish(topic.c_str(), payload.c_str(), true);
}
void MQTT4HA::onNumberCommand(const char* name, void (*callback)(float)) {
    String topic = String(_clientName) + "/" + name + "/set";
    numbers.push_back({topic, callback});
    client.subscribe(topic.c_str());
    if (_debug) { Serial.print("[MQTT4HA] Number-Command abonniert: "); Serial.println(topic); }
}

void MQTT4HA::announceButton(const char* name, const char* icon) {
    String id = _clientName, topic = "homeassistant/button/" + id + "/" + name + "/config";
    String cmdTopic = id + "/" + name + "/cmd", unique_id = id + "_" + name;
    String payload = "{";
    payload += "\"dev\":{\"name\":\"" + id + "\",\"ids\":\"" + id + "\",\"mf\":\"DIY\",\"mdl\":\"ESP32\"},";
    payload += "\"name\":\"" + String(name) + "\",";
    payload += "\"uniq_id\":\"" + unique_id + "\",";
    payload += "\"cmd_t\":\"" + cmdTopic + "\",";
    payload += "\"payload_press\":\"1\"";
    if (String(icon).length()) payload += ",\"ic\":\"" + String(icon) + "\"";
    payload += "}";
    storeAndSendDiscovery(topic, payload);
}
void MQTT4HA::onButtonPress(const char* name, void (*callback)()) {
    String topic = String(_clientName) + "/" + name + "/cmd";
    buttons.push_back({topic, callback});
    client.subscribe(topic.c_str());
    if (_debug) { Serial.print("[MQTT4HA] Button-Command abonniert: "); Serial.println(topic); }
}

void MQTT4HA::announceSelect(const char* name, std::vector<const char*> options) {
    String id = _clientName, topic = "homeassistant/select/" + id + "/" + name + "/config";
    String cmdTopic = id + "/" + name + "/set", stateTopic = id + "/" + name, unique_id = id + "_" + name;
    String payload = "{";
    payload += "\"dev\":{\"name\":\"" + id + "\",\"ids\":\"" + id + "\",\"mf\":\"DIY\",\"mdl\":\"ESP32\"},";
    payload += "\"name\":\"" + String(name) + "\",";
    payload += "\"uniq_id\":\"" + unique_id + "\",";
    payload += "\"stat_t\":\"" + stateTopic + "\",";
    payload += "\"cmd_t\":\"" + cmdTopic + "\",";
    payload += "\"options\":[";
    for (size_t i = 0; i < options.size(); i++) {
        if (i > 0) payload += ",";
        payload += "\"" + String(options[i]) + "\"";
    }
    payload += "]";
    payload += "}";
    storeAndSendDiscovery(topic, payload);
}
void MQTT4HA::sendSelectState(const char* name, const char* value) {
    String topic = String(_clientName) + "/" + name, payload = String(value);
    if (_debug) { Serial.print("[MQTT4HA] Sende Select: "); Serial.print(topic); Serial.print(" -> "); Serial.println(payload); }
    client.publish(topic.c_str(), payload.c_str(), true);
}
void MQTT4HA::onSelectCommand(const char* name, void (*callback)(const char*)) {
    String topic = String(_clientName) + "/" + name + "/set";
    selects.push_back({topic, callback});
    client.subscribe(topic.c_str());
    if (_debug) { Serial.print("[MQTT4HA] Select-Command abonniert: "); Serial.println(topic); }
}

void MQTT4HA::announceCover(const char* name) {
    String id = _clientName, topic = "homeassistant/cover/" + id + "/" + name + "/config";
    String cmdTopic = id + "/" + name + "/set", stateTopic = id + "/" + name + "/state", unique_id = id + "_" + name;
    String payload = "{";
    payload += "\"dev\":{\"name\":\"" + id + "\",\"ids\":\"" + id + "\",\"mf\":\"DIY\",\"mdl\":\"ESP32\"},";
    payload += "\"name\":\"" + String(name) + "\",";
    payload += "\"uniq_id\":\"" + unique_id + "\",";
    payload += "\"stat_t\":\"" + stateTopic + "\",";
    payload += "\"cmd_t\":\"" + cmdTopic + "\"";
    payload += "}";
    storeAndSendDiscovery(topic, payload);
}
void MQTT4HA::sendCoverState(const char* name, const char* position, const char* state) {
    String topic = String(_clientName) + "/" + name + "/state";
    String payload = String(position);
    if (state != nullptr) payload += String(",") + String(state);
    if (_debug) { Serial.print("[MQTT4HA] Sende Cover: "); Serial.print(topic); Serial.print(" -> "); Serial.println(payload); }
    client.publish(topic.c_str(), payload.c_str(), true);
}
void MQTT4HA::onCoverCommand(const char* name, void (*callback)(const char*)) {
    String topic = String(_clientName) + "/" + name + "/set";
    covers.push_back({topic, callback});
    client.subscribe(topic.c_str());
    if (_debug) { Serial.print("[MQTT4HA] Cover-Command abonniert: "); Serial.println(topic); }
}

// ----- Callback Handling -----
void MQTT4HA::_callback(char* topic, byte* payload, unsigned int length) {
    if (instance) instance->_handleCallback(topic, payload, length);
}
void MQTT4HA::_handleCallback(char* topic, byte* payload, unsigned int length) {
    String t = String(topic), message;
    for (unsigned int i = 0; i < length; i++) message += (char)payload[i];
    if (_debug) { Serial.print("[MQTT4HA] Empfange MQTT: "); Serial.print(t); Serial.print(" -> "); Serial.println(message); }
    // Switch
    for (auto& sw : switches) if (t == sw.topic) { bool on = (message == "ON" || message == "on" || message == "1"); if (sw.callback) sw.callback(on); return; }
    // Light
    for (auto& l : lights) if (t == l.topic) {
        bool on = message.indexOf("\"state\":\"ON\"") >= 0;
        int brightness = 255, r = 255, g = 255, b = 255;
        int pos = message.indexOf("\"brightness\":"); if (pos >= 0) brightness = message.substring(pos + 13).toInt();
        pos = message.indexOf("\"color\":");
        if (pos >= 0) {
            int rpos = message.indexOf("\"r\":", pos); if (rpos > 0) r = message.substring(rpos + 4).toInt();
            int gpos = message.indexOf("\"g\":", pos); if (gpos > 0) g = message.substring(gpos + 4).toInt();
            int bpos = message.indexOf("\"b\":", pos); if (bpos > 0) b = message.substring(bpos + 4).toInt();
        }
        if (l.callback) l.callback(on, brightness, r, g, b); return;
    }
    // Number
    for (auto& n : numbers) if (t == n.topic) { float value = message.toFloat(); if (n.callback) n.callback(value); return; }
    // Button
    for (auto& b : buttons) if (t == b.topic) { if (b.callback) b.callback(); return; }
    // Select
    for (auto& s : selects) if (t == s.topic) { if (s.callback) s.callback(message.c_str()); return; }
    // Cover
    for (auto& c : covers) if (t == c.topic) { if (c.callback) c.callback(message.c_str()); return; }
}

void MQTT4HA::_reconnect() {
    while (!client.connected()) {
        if (_debug) { Serial.print("[MQTT4HA] Verbinde zu MQTT Broker "); Serial.print(_broker); Serial.print(" ... "); }
        bool connected = false;
        if (_mqttUser && _mqttPass) connected = client.connect(_clientName, _mqttUser, _mqttPass);
        else connected = client.connect(_clientName);
        if (connected) { if (_debug) Serial.println("erfolgreich!"); }
        else { if (_debug) { Serial.print("Fehlgeschlagen, rc="); Serial.print(client.state()); Serial.println(" - neuer Versuch in 2s"); } delay(2000); }
    }
}
