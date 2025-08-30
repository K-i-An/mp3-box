#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// ====== WLAN Zugangsdaten (anpassen!) ======
const char* ssids[] = { "Wireless_EG_2.4GHz", "Wireless_EG1", "Wireless_DG", "DG@2.5G" };
const char* password = "7rxJsxLSYN2yY";

// ====== Pins ======
constexpr uint8_t RST_PIN = 5;
constexpr uint8_t SS_PIN = 4;
const int buttonPause = 3;                // Pause/Start Button
SoftwareSerial mySoftwareSerial(13, 15);  // RX, TX

// ====== Variablen ======
int folderCnt = 46;
bool bpPause = false;
bool dancePause = false;
bool bpFolder = false;
bool btnPauseHigh = true;
bool initBox = false;
bool stopDance = false;
int stopDanceTimer = 0;
int volumeMP3Player = 0;
unsigned long playTime = 0;
unsigned long pauseTime = 5000;  // 5s


int selFolder = 0;
int selFileNr = 0;
int maxFileNr = 0;
int time1, time2, timeDiv;

unsigned long sleepTimer = 90UL * 60UL * 1000UL;  // 1,5h
unsigned long lastActionTime = 0;

// ====== RFID-Listen-Struktur ======
struct storedUID {
  String uid;
  int folderID;
};
storedUID listUID[46];

// ====== Objekte ======
DFRobotDFPlayerMini myDFPlayer;
MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
ESP8266WebServer server(80);

// ====== Funktionsprototypen ======
void setupMP3();
void setupRFID();
void saveRFIDTable();
void detectedRFIDCard();
void selectFolder();
void printDetail(uint8_t type, int value);
int getUIDFolder(String uid);
void stopDanceRoutine();
void setupWiFi();
void setupWebpage();
void handleSelect();
void handleRoot();
void handleStatus();

// ====== Setup ======
void setup() {
  mySoftwareSerial.begin(9600);
  Serial.begin(9600);
  pinMode(buttonPause, INPUT_PULLUP);

  setupMP3();
  setupRFID();
  saveRFIDTable();
  setupWiFi();
  setupWebpage();

  lastActionTime = millis();
}

void setupWebpage() {
  // Webserver-Routen
  server.on("/", []() {
    handleRoot();
  });

  server.on("/play", []() {
    myDFPlayer.start();
    bpPause = false;
    lastActionTime = millis();
    server.send(200, "text/plain", "Play gestartet");
  });

  server.on("/pause", []() {
    myDFPlayer.pause();
    bpPause = true;
    lastActionTime = millis();
    server.send(200, "text/plain", "Pause aktiviert");
  });

  server.on("/select", []() {
    handleSelect();
  });

  server.on("/louder", []() {
    if (volumeMP3Player < 30) {
      volumeMP3Player += 1;
      myDFPlayer.volume(volumeMP3Player);
    }
    server.send(200, "text/plain", "Lauter");
  });

  server.on("/quieter", []() {
    if (volumeMP3Player > 0) {
      volumeMP3Player -= 1;
      myDFPlayer.volume(volumeMP3Player);
    }
    server.send(200, "text/plain", "Leiser");
  });

  server.on("/status", []() {
    handleStatus();
  });

  server.begin();
  Serial.println("Webserver gestartet.");
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Musikbox Steuerung</title>
</head>
<body>
  <h2>Musikbox Steuerung</h2>

  <a href="/play"><button>Play</button></a>
  <a href="/pause"><button>Pause</button></a>
  <a href="/louder"><button>Lauter</button></a>
  <a href="/quieter"><button>Leiser</button></a>
  <a href="/status"><button>Status</button></a>

  <h2>Ordner w&auml;hlen</h2>
  <p>Klicke die Nummer, um den Ordner zu starten:</p>
  <ul>
    <li><a href='/select?folder=1'>1. Mulan</a></li>
    <li><a href='/select?folder=2'>2. TKKG</a></li>
    <li><a href='/select?folder=3'>3. Gloeckner</a></li>
    <li><a href='/select?folder=4'>4. Peter Pan</a></li>
    <li><a href='/select?folder=5'>5. Sally / School</a></li>
    <li><a href='/select?folder=6'>6. Pocahontas</a></li>
    <li><a href='/select?folder=7'>7. Barbie</a></li>
    <li><a href='/select?folder=8'>8. Dornr&ouml;&szlig;chen</a></li>
    <li><a href='/select?folder=9'>9. Raya</a></li>
    <li><a href='/select?folder=10'>10. Rapunzel</a></li>
    <li><a href='/select?folder=11'>11. Die Sch&ouml;ne und das Biest</a></li>
    <li><a href='/select?folder=12'>12. Aladin</a></li>
    <li><a href='/select?folder=13'>13. Tarzan</a></li>
    <li><a href='/select?folder=14'>14. Der Prinz von &Auml;gypten</a></li>
    <li><a href='/select?folder=15'>15. Vaiana</a></li>
    <li><a href='/select?folder=16'>16. K&ouml;nig der L&ouml;wen</a></li>
    <li><a href='/select?folder=17'>17. Luca</a></li>
    <li><a href='/select?folder=18'>18. Casper</a></li>
    <li><a href='/select?folder=19'>19. Harry Potter</a></li>
    <li><a href='/select?folder=20'>20. Black Pink</a></li>
    <li><a href='/select?folder=21'>21. Lea</a></li>
    <li><a href='/select?folder=22'>22. Lina</a></li>
    <li><a href='/select?folder=23'>23. Bibi & Tina</a></li>
    <li><a href='/select?folder=24'>24. Herkules</a></li>
    <li><a href='/select?folder=25'>25. Encanto</a></li>
    <li><a href='/select?folder=26'>26. Emmi</a></li>
    <li><a href='/select?folder=27'>27. Bobo</a></li>
    <li><a href='/select?folder=28'>28. Gute Nacht (mein Schatz)</a></li>
    <li><a href='/select?folder=29'>29. Party</a></li>
    <li><a href='/select?folder=30'>30. Super Simple Songs</a></li>
    <li><a href='/select?folder=31'>31. Worship</a></li>
    <li><a href='/select?folder=32'>32. Workout</a></li>
    <li><a href='/select?folder=33'>33. 5 Geschwister</a></li>
    <li><a href='/select?folder=34'>34. G&auml;nsehaut</a></li>
    <li><a href='/select?folder=35'>35. 5 Freunde</a></li>
    <li><a href='/select?folder=36'>36. Die 3 ???</a></li>
    <li><a href='/select?folder=37'>37</a></li>
    <li><a href='/select?folder=38'>38</a></li>
    <li><a href='/select?folder=39'>39</a></li>
    <li><a href='/select?folder=40'>40</a></li>
    <li><a href='/select?folder=41'>41</a></li>
    <li><a href='/select?folder=42'>42</a></li>
    <li><a href='/select?folder=43'>43</a></li>
    <li><a href='/select?folder=44'>44</a></li>
    <li><a href='/select?folder=45'>45</a></li>
    <li><a href='/select?folder=46'>46</a></li>
  </ul>
  <hr>

  <h2>Manuelle Eingabe</h2>
  <form action='/select' method='get'>
    Ordnernummer: <input type='number' name='folder' min='1' max='46'>
    <input type='submit' value='Start'>
  </form>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

void handleSelect() {
  if (server.hasArg("folder")) {
    int folder = server.arg("folder").toInt();
    if (folder >= 1 && folder <= 46) {
      // Ordner starten (erstes File im Ordner)
      selFileNr = 1;
      myDFPlayer.playFolder(folder, selFileNr);
      selFolder = folder;
      maxFileNr = myDFPlayer.readFileCountsInFolder(folder);

      String html = "<!DOCTYPE html><html><head><meta charset='utf-8'><title>Auswahl</title></head><body>";
      html += "<h1>Ordner gestartet</h1>";
      html += "<p>Ordner <b>" + String(folder) + "</b> wird abgespielt.</p>";
      html += "<p><a href='/'>Zur&uuml;ck zur &Uuml;bersicht</a></p>";
      html += "</body></html>";
      server.send(200, "text/html", html);
    } else {
      server.send(400, "text/plain", "Ung&uuml;ltige Ordnernummer!");
    }
  } else {
    server.send(400, "text/plain", "Kein Ordner angegeben!");
  }
}

void handleStatus() {
  int idleTime;
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Status</title></head><body>";
  html += "<h2>📡 MP3-Box Status</h2>";
  html += "<p><b>WLAN:</b> " + WiFi.SSID() + "</p>";

  if (selFolder > 0) {
    html += "<p><b>Ordner:</b> " + String(selFolder) + "</p>";
  } else {
    html += "<p><b>Ordner:</b> keiner ausgew&auml;hlt</p>";
  }
  idleTime = millis() - lastActionTime;
  html += "<p><b>Idle Time:</b> " + String(idleTime) + "</p>";
  html += "<p><b>Abgeschaltet in:</b> " + String(sleepTimer - idleTime) + "</p>";
  html += "<p><b>Datei Nr:</b> " + String(selFileNr) + "</p>";
  html += "<p><b>Max. Datei Nr:</b> " + String(maxFileNr) + "</p>";
  html += "<p><b>Lautst&auml;rke:</b> " + String(volumeMP3Player) + "</p>";
  html += "<p><b>Stoptanz:</b> " + String(stopDance) + "</p>";
  html += "<br><a href='/'>Zur&uuml;ck</a>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void setupWiFi() {
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  Serial.println("🔎 Suche nach verfügbaren WLANs...");

  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("❌ Keine Netzwerke gefunden!");
    return;
  }

  // Versuche, eine bekannte SSID zu finden
  unsigned long startAttemptTime;
  for (int i = 0; i < n; ++i) {
    String foundSSID = WiFi.SSID(i);
    for (int j = 0; j < (sizeof(ssids) / sizeof(ssids[0])); j++) {
      if (foundSSID == ssids[j]) {
        startAttemptTime = millis();
        Serial.print("➡ Verbinde mit: ");
        Serial.println(foundSSID);
        WiFi.begin(ssids[j], password);
        while (WiFi.status() != WL_CONNECTED && (millis() - startAttemptTime < 10000)) {
          delay(500);
          Serial.print(".");
        }
        if (WiFi.status() == WL_CONNECTED) {
          if (MDNS.begin("EstherMP3Box")) {
            Serial.println("MDNS responder started");
          } else {
            Serial.println("Error setting up MDNS responder!");
          }
          Serial.println("");
          Serial.println("✅ Verbunden mit WLAN!");
          Serial.print("IP-Adresse: ");
          Serial.println(WiFi.localIP());
          return;
        }
      }
    }
  }
  Serial.println("❌ Keine bekannte SSID gefunden!");
}

// ====== Loop ======
void loop() {
  server.handleClient();
  MDNS.update();

  // RFID Check
  if (rfid.PICC_IsNewCardPresent()) {
    detectedRFIDCard();
    lastActionTime = millis();
  }

  // Button handling
  int buttonStatePause = digitalRead(buttonPause);
  if (buttonStatePause == LOW && btnPauseHigh) {
    btnPauseHigh = false;
    lastActionTime = millis();
    time1 = millis();
  }
  if (buttonStatePause == HIGH && !btnPauseHigh) {
    btnPauseHigh = true;
    time2 = millis();
    timeDiv = abs(time2 - time1);

    if (timeDiv < 500) {  // kurzer Klick = Pause/Play
      if (!bpPause) {
        myDFPlayer.pause();
        bpPause = true;
      } else {
        myDFPlayer.start();
        bpPause = false;
      }
    } else {  // langer Klick = Stop-Tanz
      if (stopDance == true) {
        Serial.println("Stop Tanz deaktiviert!");
        stopDance = false;
      } else {
        stopDance = true;
        Serial.println("Stop Tanz aktiv");
      }
    }
    lastActionTime = millis();
  }

  if (stopDance == true)
    stopDanceRoutine();

  // RFID Folder spielen
  if (bpFolder) selectFolder();

  // Init einmalig
  if (myDFPlayer.available() && !initBox) {
    initBox = true;
  }

  if (myDFPlayer.available()) {
    uint8_t type = myDFPlayer.readType();
    int value = myDFPlayer.read();

    switch (type) {
      case DFPlayerPlayFinished:
        Serial.print("Datei beendet: ");
        Serial.println(selFileNr);

        selFileNr++;  // nächste Datei
        myDFPlayer.playFolder((uint8_t)selFolder, (uint8_t)selFileNr);
        break;

      case DFPlayerError:
        Serial.print("DFPlayerError: ");
        Serial.println(value);

        // Wenn Datei fehlt oder Ordner zu Ende, zurück auf erste Datei
        if (value == 12 || value == 2) {  // 12=Invalid file, 2=File not found
          Serial.println("Ordner zu Ende oder Datei fehlt, zurücksetzen");
          selFileNr = 1;
        }
        break;
    }
  }

  if (myDFPlayer.available() && Serial.available())
    printDetail(myDFPlayer.readType(), myDFPlayer.read());

  // Inaktivität prüfen
  if (millis() - lastActionTime > sleepTimer) {
    Serial.println("Schlafe...");
    myDFPlayer.pause();
  }
}

// ====== MP3 Setup ======
void setupMP3() {
  Serial.println(F("Initializing DFPlayer ..."));
  if (!myDFPlayer.begin(mySoftwareSerial)) {
    Serial.println(F("Unable to begin: check connection / SD card!"));
    while (true)
      ;
  }
  myDFPlayer.setTimeOut(500);
  myDFPlayer.volume(15);
  volumeMP3Player = 15;
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
}

// ====== RFID ======
void setupRFID() {
  SPI.begin();
  rfid.PCD_Init();
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  Serial.println("RFID reading enabled");
}

void detectedRFIDCard() {
  if (!rfid.PICC_ReadCardSerial()) return;
  char uidString[50] = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    sprintf(uidString + strlen(uidString), "%02x", rfid.uid.uidByte[i]);
  }
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  selFolder = getUIDFolder(String(uidString));
  bpFolder = true;
}

void selectFolder() {
  if (selFolder > 0) {
    selFileNr = 1;
    myDFPlayer.playFolder(selFolder, selFileNr);
    Serial.print("Starte Ordner: ");
    Serial.println(selFolder);
  }
  lastActionTime = millis();
  bpFolder = false;
}

void stopDanceRoutine() {
  if (!dancePause && (millis() - stopDanceTimer) > playTime) {
    myDFPlayer.pause();
    dancePause = true;
    stopDanceTimer = millis();
  } else if (dancePause && (millis() - stopDanceTimer) > pauseTime) {
    playTime = random(10000, 30000);  // bis 30s
    myDFPlayer.start();
    stopDanceTimer = millis();
    dancePause = false;
  }
}

// ====== RFID-Tabelle ======
void saveRFIDTable() {
  listUID[0].uid = "1dfdf2cb930000";
  listUID[0].folderID = 1;  // Mulan
  listUID[1].uid = "1dbac8cb930000";
  listUID[1].folderID = 2;  // TKKG
  listUID[2].uid = "04c5f521110189";
  listUID[2].folderID = 3;  // Gloeckner
  listUID[3].uid = "04956422110189";
  listUID[3].folderID = 4;  // Peter Pan
  listUID[4].uid = "04bbb921110189";
  listUID[4].folderID = 5;  // Sally
  listUID[5].uid = "04f98a21110189";
  listUID[5].folderID = 6;  // Pocahontas
  listUID[6].uid = "048bf621110189";
  listUID[6].folderID = 7;  // Barbie
  listUID[7].uid = "04b8c821110189";
  listUID[7].folderID = 8;  // Dornroesschen
  listUID[8].uid = "04ce1d22110189";
  listUID[8].folderID = 9;  // Raya
  listUID[9].uid = "041c7321110189";
  listUID[9].folderID = 10;  // Rapunzel
  listUID[10].uid = "9673df5f";
  listUID[10].folderID = 11;  // Schoene & Biest
  listUID[11].uid = "9e496b1d";
  listUID[11].folderID = 12;  // Aladin
  listUID[12].uid = "11aeb789";
  listUID[12].folderID = 13;  // Tarzan
  listUID[13].uid = "808be112";
  listUID[13].folderID = 14;  // Prinz Agypten
  listUID[14].uid = "1dc0b394680000";
  listUID[14].folderID = 15;  // Vaiana
  listUID[15].uid = "1dd8b394680000";
  listUID[15].folderID = 16;  // Koenig der Loewen
  listUID[16].uid = "1dd5b394680000";
  listUID[16].folderID = 17;  // Luca
  listUID[17].uid = "1dfbb394680000";
  listUID[17].folderID = 18;  // Casper
  listUID[18].uid = "1dfab394680000";
  listUID[18].folderID = 19;  // Harry Potter
  listUID[19].uid = "1de4b394680000";
  listUID[19].folderID = 20;  // Black Pink
  listUID[20].uid = "1de0b394680000";
  listUID[20].folderID = 21;  // Lea
  listUID[21].uid = "1dfdb394680000";
  listUID[21].folderID = 22;  // Lina
  listUID[22].uid = "1d82b394680000";
  listUID[22].folderID = 23;  // Bibi & Tina
  listUID[23].uid = "1d7fb394680000";
  listUID[23].folderID = 24;  // Herkules
  listUID[24].uid = "1d2bdbcb930000";
  listUID[24].folderID = 25;  // Encanto
  listUID[25].uid = "04504f21110189";
  listUID[25].folderID = 26;  // Emmi
  listUID[26].uid = "04881422110189";
  listUID[26].folderID = 27;  // Bobo
  listUID[27].uid = "1d49d4cb930000";
  listUID[27].folderID = 28;  // Gute Nacht
  listUID[28].uid = "1d4fe5cb930000";
  listUID[28].folderID = 29;  // Party
  listUID[29].uid = "1227ae89";
  listUID[29].folderID = 30;  // Super Simple Songs
  listUID[30].uid = "eeac901d";
  listUID[30].folderID = 31;  // Worship
  listUID[31].uid = "1d00cbcb930000";
  listUID[31].folderID = 32;  // Workout
  listUID[32].uid = "1d73edcb930000";
  listUID[32].folderID = 33;  // 5 Geschwister
  listUID[33].uid = "1da4d4cb930000";
  listUID[33].folderID = 34;  // Gaensehaut
  listUID[34].uid = "1dd4e3cb930000";
  listUID[34].folderID = 35;  // 5 Freunde
  listUID[35].uid = "1dc1dacb930000";
  listUID[35].folderID = 36;  // Die 3 ???
  listUID[36].uid = "1d8f3930880000";
  listUID[36].folderID = 37;  //
  listUID[37].uid = "1d712c30880000";
  listUID[37].folderID = 38;  //
  listUID[38].uid = "1de94330880000";
  listUID[38].folderID = 39;  //
  listUID[39].uid = "1d5f1b30880000";
  listUID[39].folderID = 40;  //
  listUID[40].uid = "1d2d4030880000";
  listUID[40].folderID = 41;  //
  listUID[41].uid = "1d852530880000";
  listUID[41].folderID = 42;  //
  listUID[42].uid = "1dea2f30880000";
  listUID[42].folderID = 43;  //
  listUID[43].uid = "1d182230880000";
  listUID[43].folderID = 44;  //
  listUID[44].uid = "1d854030880000";
  listUID[44].folderID = 45;  //
  listUID[45].uid = "1d882c30880000";
  listUID[45].folderID = 46;  //
}

// ====== UID-Ordner Zuordnung ======
int getUIDFolder(String uid) {
  for (int i = 0; i < folderCnt; i++) {
    if (listUID[i].uid.equalsIgnoreCase(uid)) return listUID[i].folderID;
  }
  Serial.print("Unbekannte UID: ");
  Serial.println(uid);
  return -1;
}

void printDetail(uint8_t type, int value) {
  if (type == DFPlayerError) {
    Serial.print(F("DFPlayerError: "));
    Serial.println(value);
  }
}
