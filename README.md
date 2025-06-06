# MQTT4HA (ESP32 MQTT for Home Assistant)
Lib zum einbinden von Geräten via MQTT an Home Assistant mit nur 3 Zeilen Code

## Was macht die Bibliothek?

- Vereinfacht die Anbindung von ESP32/ESP8266-Geräten an Home Assistant per MQTT.
- Alle Home Assistant-Typen (Sensoren, Schalter, Licht, Zahleneingaben, Buttons, Auswahlfelder, uvm.) lassen sich mit wenigen Funktionsaufrufen „automatisch“ einbinden.
- Discovery und MQTT-Handling laufen vollautomatisch im Hintergrund, auch bei Verbindungsabbrüchen oder Reconnect.

## Wie nutzt du die Bibliothek?

- #### Im Sketch einbinden:
Bibliothek einbinden und ein MQTT4HA-Objekt mit Gerätenamen und Broker-Adresse anlegen.
- #### WLAN verbinden:
WLAN wird wie gewohnt im Sketch hergestellt (die Bibliothek übernimmt das nicht).
- #### MQTT starten:
Mit den Zugangsdaten den MQTT-Client starten (über die begin-Funktion).
- #### Geräte/Entitäten anmelden:
Für jede Entität (Sensor, Switch, etc.) wird einmal eine „announce...“-Funktion aufgerufen.
→ Die Bibliothek merkt sich das und kümmert sich selbstständig um das Senden der nötigen Home Assistant-Konfiguration.
- #### Werte senden / empfangen:
Sensoren und Zustände sendest du mit den passenden „send...“-Funktionen.
Schalt- oder Steuerbefehle aus Home Assistant werden über Callbacks (onSwitchCommand, onButtonPress usw.) abgefangen.
- #### Im Loop laufen lassen:
Mit einem einzigen Funktionsaufruf im Loop bleibt die MQTT-Verbindung aktiv und alle Nachrichten werden verarbeitet.
- #### Debug-Modus:
Über einen Schalter kannst du alle Debug-Ausgaben jederzeit an- oder ausschalten.

## Welche Hauptfunktionen gibt es?

- #### announce...:
Meldet eine neue Entität (Sensor, Switch, Light, Number, Button, Select, Cover, etc.) bei Home Assistant an.
- #### send...:
Sendet aktuelle Werte oder Zustände einer Entität (Sensorwert, Switch-Zustand, Lichtfarbe, usw.) an Home Assistant.
- #### on...Command/Press:
Verknüpft einen Callback (Funktion), die ausgeführt wird, wenn Home Assistant einen Befehl (z.B. Schalten, Auswahl, Button-Press) sendet.
- #### setDebug:
Schaltet alle Debug-Ausgaben an oder aus.
- #### begin:
Startet die MQTT-Verbindung (nachdem WLAN steht).
- #### loop:
Hält die MQTT-Verbindung und verarbeitet eingehende Befehle.

## Was passiert im Hintergrund?

- #### Alle „announce...“-Aufrufe werden gespeichert:
- Die Bibliothek merkt sich jede angemeldete Entität.
- Discovery wird immer dann gesendet, wenn die MQTT-Verbindung besteht – auch nach Reconnect.
- Du musst nie auf das richtige Timing achten:
- Egal, wann du eine announce...-Funktion rufst, die Lib sorgt dafür, dass Home Assistant alles korrekt sieht.
- Bei Verbindungsabbrüchen werden alle Entities automatisch wieder im Broker/Home Assistant eingetragen.
- Wie ist der typische Ablauf für eigene Projekte?

- #### MQTT4HA-Objekt anlegen (Gerätename, Broker-Adresse, Debug-Flag)
- WLAN verbinden
- ha.begin(...) aufrufen
- ha.announce... für alle gewünschten Entities aufrufen
- Callbacks mit ha.on...Command(...) registrieren (optional, z.B. für Switch)
- ha.loop() im Loop aufrufen
- ha.send... für aktuelle Werte/Zustände verwenden



| **Funktion**                | **Beschreibung**                                         |
| --------------------------- | -------------------------------------------------------- |
| `begin()`                   | Startet die MQTT-Verbindung (nach WLAN-Verbindung)       |
| `setDebug(bool)`            | Debug-Ausgaben an/aus schalten                           |
| `loop()`                    | Hält die MQTT-Verbindung aktiv und verarbeitet Befehle   |
|                             |                                                          |
| `announceSensor(...)`       | Meldet einen Sensor an Home Assistant an                 |
| `sendSensor(...)`           | Sendet einen Sensorwert                                  |
|                             |                                                          |
| `announceBinarySensor(...)` | Meldet einen Binary Sensor (z.B. Tür auf/zu) an          |
| `sendBinarySensor(...)`     | Sendet den Status des Binary Sensors (ON/OFF)            |
|                             |                                                          |
| `announceSwitch(...)`       | Meldet einen Switch (Schalter) an                        |
| `sendSwitchState(...)`      | Sendet den aktuellen Zustand des Schalters (ON/OFF)      |
| `onSwitchCommand(...)`      | Callback für empfangene Schaltbefehle aus Home Assistant |
|                             |                                                          |
| `announceLight(...)`        | Meldet ein Licht (auch RGB/Dimmbar) an                   |
| `sendLightState(...)`       | Sendet Licht-Zustand, Helligkeit und Farbe               |
| `onLightCommand(...)`       | Callback für Steuerbefehle aus Home Assistant            |
|                             |                                                          |
| `announceNumber(...)`       | Meldet eine Zahl-Eingabe an (z.B. Sollwert)              |
| `sendNumberState(...)`      | Sendet aktuellen Zahlenwert                              |
| `onNumberCommand(...)`      | Callback für Werteingaben aus Home Assistant             |
|                             |                                                          |
| `announceButton(...)`       | Meldet einen Button (Taster) an                          |
| `onButtonPress(...)`        | Callback für Button-Events aus Home Assistant            |
|                             |                                                          |
| `announceSelect(...)`       | Meldet ein Auswahlfeld (Dropdown) an                     |
| `sendSelectState(...)`      | Sendet den aktuellen Auswahlwert                         |
| `onSelectCommand(...)`      | Callback für Auswahlbefehle aus Home Assistant           |
|                             |                                                          |
| `announceCover(...)`        | Meldet ein Cover (Rollladen/Garagentor) an               |
| `sendCoverState(...)`       | Sendet die aktuelle Position/zustand des Covers          |
| `onCoverCommand(...)`       | Callback für Steuerbefehle aus Home Assistant            |



| **Befehl**                                                | **Beschreibung**                                              | **Beispiel**                                      |
| --------------------------------------------------------- | ------------------------------------------------------------- | ------------------------------------------------- |
| MQTT4HA(const char\* name, const char\* broker, \[debug]) | Konstruktor: Objekt anlegen mit Gerätename, Broker, Debug     | MQTT4HA ha("esp32\_demo", "192.168.1.10", true);  |
| begin(\[user], \[pass], \[port])                          | Startet MQTT-Client (nachdem WLAN verbunden wurde!)           | ha.begin("mqttuser", "mqttpass", 1883);           |
| setDebug(bool)                                            | Debug-Ausgaben an/aus schalten                                | ha.setDebug(true);                                |
| loop()                                                    | Muss regelmäßig in loop() laufen, hält MQTT-Verbindung        | ha.loop();                                        |
| announceSensor(name, einheit, \[device\_class])           | Sensor für Home Assistant Discovery anmelden                  | ha.announceSensor("temp", "°C", "temperature");   |
| sendSensor(name, value)                                   | Sensorwert veröffentlichen                                    | ha.sendSensor("temp", 22.5);                      |
| announceBinarySensor(name, \[device\_class])              | Binary Sensor (z.B. Türkontakt) anmelden                      | ha.announceBinarySensor("door", "door");          |
| sendBinarySensor(name, state)                             | Binary Sensor-Zustand (ON/OFF) senden                         | ha.sendBinarySensor("door", true);                |
| announceSwitch(name, \[icon])                             | Schalter (Switch) per Discovery anmelden                      | ha.announceSwitch("led", "mdi\:lightbulb");       |
| sendSwitchState(name, on)                                 | Schalter-Zustand an HA melden (ON/OFF)                        | ha.sendSwitchState("led", true);                  |
| onSwitchCommand(name, callback)                           | Callback für Schalter-Befehle aus Home Assistant registrieren | ha.onSwitchCommand("led", ledCallback);           |
| announceLight(name, \[dimmbar], \[rgb])                   | Licht (auch RGB/Dimmbar) anmelden                             | ha.announceLight("lampe", true, true);            |
| sendLightState(name, on, brightness, r, g, b)             | Licht steuern (Status, Helligkeit, Farbe)                     | ha.sendLightState("lampe", true, 128, 255, 0, 0); |
| onLightCommand(name, callback)                            | Callback für Licht-Befehle aus Home Assistant                 | ha.onLightCommand("lampe", lightCallback);        |
| announceNumber(name, min, max, step, \[einheit])          | Zahleneingabe (Number) anmelden                               | ha.announceNumber("solltemp", 10, 40, 0.5, "°C"); |
| sendNumberState(name, value)                              | Zahlenwert an HA senden                                       | ha.sendNumberState("solltemp", 22.0);             |
| onNumberCommand(name, callback)                           | Callback für Zahleneingaben                                   | ha.onNumberCommand("solltemp", numberCallback);   |
| announceButton(name, \[icon])                             | Button (Taster) anmelden                                      | ha.announceButton("restart", "mdi\:restart");     |
| onButtonPress(name, callback)                             | Callback für Button-Events                                    | ha.onButtonPress("restart", restartCallback);     |
| announceSelect(name, options)                             | Auswahlfeld (Dropdown) anmelden                               | ha.announceSelect("mode", {"Auto", "Eco"});       |
| sendSelectState(name, value)                              | Auswahlwert an HA senden                                      | ha.sendSelectState("mode", "Auto");               |
| onSelectCommand(name, callback)                           | Callback für Auswahlbefehle                                   | ha.onSelectCommand("mode", modeCallback);         |
| announceCover(name)                                       | Rollladen/Tor (Cover) anmelden                                | ha.announceCover("garage");                       |
| sendCoverState(name, position, \[state])                  | Cover-Status an HA senden                                     | ha.sendCoverState("garage", "open");              |
| onCoverCommand(name, callback)                            | Callback für Cover-Befehle                                    | ha.onCoverCommand("garage", coverCallback);       |

## Mit und Ohne Geräteanmeldung bei Home Assistant
| **Anwendung**                         | **Schritt**      | **Code-Snippet**                                                             | **Kommentar**                 |
| ------------------------------------- | ---------------- | ---------------------------------------------------------------------------- | ----------------------------- |
| **Gerät bei Home Assistant anmelden** | Objekt anlegen   | `MQTT4HA ha("esp32_demo", "192.168.1.10");`                                  | Name + Broker-IP              |
|                                       | WLAN verbinden   | `WiFi.begin("SSID", "PASS"); while(WiFi.status()!=WL_CONNECTED) delay(100);` | Wie gewohnt im Sketch         |
|                                       | MQTT starten     | `ha.begin("user", "pass");`                                                  | Port optional                 |
|                                       | Entität anmelden | `ha.announceSensor("temp", "°C", "temperature");`                            | Automatisch in HA sichtbar    |
|                                       | Wert senden      | `ha.sendSensor("temp", 22.5);`                                               | Wert an HA schicken           |
|                                       | loop() aufrufen  | `ha.loop();`                                                                 | Verbindung & Kommandos halten |

| **Nur MQTT-Nachricht (ohne Home Assistant)** | **Schritt**                                      | **Code-Snippet**                           | **Kommentar** |
| -------------------------------------------- | ------------------------------------------------ | ------------------------------------------ | ------------- |
| Objekt anlegen                               | `MQTT4HA ha("demo", "192.168.1.10");`            | Name und Broker                            |               |
| WLAN verbinden                               | `WiFi.begin("SSID", "PASS"); ...`                |                                            |               |
| MQTT starten                                 | `ha.begin("user", "pass");`                      |                                            |               |
| Beliebige Nachricht senden                   | `ha.client.publish("mein/topic", "Dein Text!");` | Direkt mit client-Objekt, beliebiges Topic |               |
| loop() aufrufen                              | `ha.loop();`                                     | Verbindung halten                          |               |

## Beispiel: Home Assistant Discovery
```cpp
MQTT4HA ha("esp32_demo", "192.168.1.10");
WiFi.begin("SSID", "PASS");
while (WiFi.status() != WL_CONNECTED) delay(500);
ha.begin("user", "pass");
ha.announceSensor("temp", "°C", "temperature");
ha.sendSensor("temp", 22.5);
ha.loop();
```

## Beispiel: Nur MQTT-Nachricht (ohne HA)
```cpp
MQTT4HA ha("demo", "192.168.1.10");
WiFi.begin("SSID", "PASS");
while (WiFi.status() != WL_CONNECTED) delay(500);
ha.begin("user", "pass");
ha.client.publish("mein/thema", "Nur MQTT – kein Home Assistant");
ha.loop();
```
