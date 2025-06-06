#ifndef MQTT4HA_H
#define MQTT4HA_H

#include <WiFiClient.h>
#include <PubSubClient.h>
#include <vector>

/**
 * Universal Home Assistant MQTT Modul für ESP32
 * Features:
 * - Unterstützt: sensor, binary_sensor, switch, light, number, button, select, cover
 * - Automatische Discovery-Updates bei jedem MQTT-Connect (auch nach Reconnect!)
 * - Zentrale Debug-Steuerung per setDebug()
 * - Minimaler Sketch, alle announceX einfach aufrufen
 * Autor: Thomas Utpatel und ChatGPT, Stand: Juni 2025
 */

class MQTT4HA {
public:
    MQTT4HA(const char* clientName, const char* broker, int debug = 0);
    void setDebug(bool debugOn);

    void begin(const char* mqttUser = nullptr, const char* mqttPass = nullptr, int port = 1883);
    void loop();

    // Discovery-API (speichert alle Aufrufe für spätere Wiederholung!)
    void announceSensor(const char* name, const char* unit, const char* device_class = "");
    void sendSensor(const char* name, float value);

    void announceBinarySensor(const char* name, const char* device_class = "");
    void sendBinarySensor(const char* name, bool state);

    void announceSwitch(const char* name, const char* icon = "");
    void sendSwitchState(const char* name, bool on);
    void onSwitchCommand(const char* name, void (*callback)(bool));

    void announceLight(const char* name, bool dimmable = false, bool rgb = false);
    void sendLightState(const char* name, bool on, int brightness = 255, int r = 255, int g = 255, int b = 255);
    void onLightCommand(const char* name, void (*callback)(bool, int, int, int, int));

    void announceNumber(const char* name, float min, float max, float step, const char* unit = "");
    void sendNumberState(const char* name, float value);
    void onNumberCommand(const char* name, void (*callback)(float));

    void announceButton(const char* name, const char* icon = "");
    void onButtonPress(const char* name, void (*callback)());

    void announceSelect(const char* name, std::vector<const char*> options);
    void sendSelectState(const char* name, const char* value);
    void onSelectCommand(const char* name, void (*callback)(const char*));

    void announceCover(const char* name);
    void sendCoverState(const char* name, const char* position, const char* state = nullptr);
    void onCoverCommand(const char* name, void (*callback)(const char*));

    PubSubClient client; // Zugriff falls benötigt (z.B. .connected())

private:
    WiFiClient espClient;
    const char* _clientName;
    const char* _broker;
    const char* _mqttUser;
    const char* _mqttPass;
    int _port;
    bool _debug;

    // Discovery-Config: Speicherung aller Topics und Payloads
    struct DiscoveryMsg {
        String topic;
        String payload;
    };
    std::vector<DiscoveryMsg> discoveryConfigs;

    // Callbacks
    struct SwitchCB { String topic; void (*callback)(bool); };
    std::vector<SwitchCB> switches;

    struct LightCB  { String topic; void (*callback)(bool, int, int, int, int); };
    std::vector<LightCB> lights;

    struct NumberCB { String topic; void (*callback)(float); };
    std::vector<NumberCB> numbers;

    struct ButtonCB { String topic; void (*callback)(); };
    std::vector<ButtonCB> buttons;

    struct SelectCB { String topic; void (*callback)(const char*); };
    std::vector<SelectCB> selects;

    struct CoverCB  { String topic; void (*callback)(const char*); };
    std::vector<CoverCB> covers;

    // Zustandsverwaltung
    bool wasConnected = false;

    // Speichert Discovery-Konfiguration und sendet sie ggf. sofort
    void storeAndSendDiscovery(const String& topic, const String& payload);

    void _sendAllDiscovery(); // schickt alle gespeicherten Discovery-Configs

    void _reconnect();
    static void _callback(char* topic, byte* payload, unsigned int length);
    void _handleCallback(char* topic, byte* payload, unsigned int length);

    static MQTT4HA* instance;
};

#endif
