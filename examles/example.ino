#include <WiFi.h>
#include <MQTT4HA.h>

const char* WIFI_SSID = "DEIN_SSID";
const char* WIFI_PASS = "DEIN_PASS";
const char* MQTT_BROKER = "192.168.1.10";
const char* MQTT_USER = "mqttuser"; // optional
const char* MQTT_PASS = "mqttpass"; // optional
const char* CLIENT_NAME = "esp32_universal";

MQTT4HA ha(CLIENT_NAME, MQTT_BROKER, true); // true = Debug aktiv

void ledCallback(bool on) {
    digitalWrite(2, on ? HIGH : LOW);
    ha.sendSwitchState("led", on);
}

void setup() {
    Serial.begin(115200);
    pinMode(2, OUTPUT);

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) delay(500);

    ha.begin(MQTT_USER, MQTT_PASS);
    ha.setDebug(true);

    ha.announceSensor("temperature", "°C", "temperature");
    ha.announceSwitch("led", "mdi:lightbulb");
    ha.onSwitchCommand("led", ledCallback);
    ha.announceBinarySensor("door", "door");
}

void loop() {
    ha.loop();

    static unsigned long last = 0;
    if (millis() - last > 10000) {
        ha.sendSensor("temperature", 22.5);
        ha.sendBinarySensor("door", true);
        last = millis();
    }
}
