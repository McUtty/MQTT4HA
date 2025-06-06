# MQTT4HA (ESP32 MQTT for Home Assistant)
Lib zum einbinden von Geräten via MQTT an Home Assistant mit nur 3 Zeilen Code

## Was macht die Bibliothek?

- Vereinfacht die Anbindung von ESP32/ESP8266-Geräten an Home Assistant per MQTT.
- Alle Home Assistant-Typen (Sensoren, Schalter, Licht, Zahleneingaben, Buttons, Auswahlfelder, uvm.) lassen sich mit wenigen Funktionsaufrufen „automatisch“ einbinden.
- Discovery und MQTT-Handling laufen vollautomatisch im Hintergrund, auch bei Verbindungsabbrüchen oder Reconnect.

## Wie nutzt du die Bibliothek?

### Im Sketch einbinden:
Bibliothek einbinden und ein MQTT4HA-Objekt mit Gerätenamen und Broker-Adresse anlegen.
### WLAN verbinden:
WLAN wird wie gewohnt im Sketch hergestellt (die Bibliothek übernimmt das nicht).
### MQTT starten:
Mit den Zugangsdaten den MQTT-Client starten (über die begin-Funktion).
### Geräte/Entitäten anmelden:
Für jede Entität (Sensor, Switch, etc.) wird einmal eine „announce...“-Funktion aufgerufen.
→ Die Bibliothek merkt sich das und kümmert sich selbstständig um das Senden der nötigen Home Assistant-Konfiguration.
### Werte senden / empfangen:
Sensoren und Zustände sendest du mit den passenden „send...“-Funktionen.
Schalt- oder Steuerbefehle aus Home Assistant werden über Callbacks (onSwitchCommand, onButtonPress usw.) abgefangen.
### Im Loop laufen lassen:
Mit einem einzigen Funktionsaufruf im Loop bleibt die MQTT-Verbindung aktiv und alle Nachrichten werden verarbeitet.
### Debug-Modus:
Über einen Schalter kannst du alle Debug-Ausgaben jederzeit an- oder ausschalten.

## Welche Hauptfunktionen gibt es?

### announce...:
Meldet eine neue Entität (Sensor, Switch, Light, Number, Button, Select, Cover, etc.) bei Home Assistant an.
### send...:
Sendet aktuelle Werte oder Zustände einer Entität (Sensorwert, Switch-Zustand, Lichtfarbe, usw.) an Home Assistant.
### on...Command/Press:
Verknüpft einen Callback (Funktion), die ausgeführt wird, wenn Home Assistant einen Befehl (z.B. Schalten, Auswahl, Button-Press) sendet.
### setDebug:
Schaltet alle Debug-Ausgaben an oder aus.
### begin:
Startet die MQTT-Verbindung (nachdem WLAN steht).
### loop:
Hält die MQTT-Verbindung und verarbeitet eingehende Befehle.

## Was passiert im Hintergrund?

### Alle „announce...“-Aufrufe werden gespeichert:
- Die Bibliothek merkt sich jede angemeldete Entität.
- Discovery wird immer dann gesendet, wenn die MQTT-Verbindung besteht – auch nach Reconnect.
- Du musst nie auf das richtige Timing achten:
- Egal, wann du eine announce...-Funktion rufst, die Lib sorgt dafür, dass Home Assistant alles korrekt sieht.
- Bei Verbindungsabbrüchen werden alle Entities automatisch wieder im Broker/Home Assistant eingetragen.
- Wie ist der typische Ablauf für eigene Projekte?

### MQTT4HA-Objekt anlegen (Gerätename, Broker-Adresse, Debug-Flag)
- WLAN verbinden
- ha.begin(...) aufrufen
- ha.announce... für alle gewünschten Entities aufrufen
- Callbacks mit ha.on...Command(...) registrieren (optional, z.B. für Switch)
- ha.loop() im Loop aufrufen
- ha.send... für aktuelle Werte/Zustände verwenden
