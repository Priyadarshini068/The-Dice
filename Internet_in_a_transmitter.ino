-----------------------FINAL TRANSMITTER CODE--------------------------

// ============================================================
// ESP32-S3 EMERGENCY BOX - LORA TRANSMITTER
// ============================================================

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <RadioLib.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPS++.h>

// ============================================================
// PIN CONFIGURATION
// ============================================================

// Push Button
#define BUTTON_PIN 4

// OLED
#define OLED_SDA 8
#define OLED_SCL 9

// SX1278 LoRa
#define LORA_CS   10
#define LORA_DIO0 2
#define LORA_RST  5

// GPS
#define GPS_RX 17
#define GPS_TX 18

// ============================================================
// HARDWARE OBJECTS
// ============================================================

SX1278 radio = new Module(
  LORA_CS,
  LORA_DIO0,
  LORA_RST
);

HardwareSerial gpsSerial(1);

TinyGPSPlus gps;

WebServer server(80);

Adafruit_SSD1306 display(
  128,
  64,
  &Wire,
  -1
);

// ============================================================
// GPS DEFAULT LOCATION
// ============================================================

float currentLat = 12.9716;
float currentLng = 77.5946;

// ============================================================
// OLED FUNCTION
// ============================================================

void updateOLED(
  String line1,
  String line2,
  String line3
) {

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println(line1);

  display.setCursor(0, 20);
  display.println(line2);

  display.setCursor(0, 40);
  display.println(line3);

  display.display();

  // Give OLED time to update
  delay(100);
}

// ============================================================
// SEND LORA SOS
// ============================================================

void sendLoRaSOS(String triggerType) {

  Serial.println();
  Serial.println("========================================");
  Serial.println("[SOS] SOS REQUEST RECEIVED");
  Serial.println("========================================");

  // ----------------------------------------------------------
  // OLED: SOS STARTED
  // ----------------------------------------------------------

  updateOLED(
    "SOS ACTIVATED!",
    "Sending LoRa...",
    "Please Wait..."
  );

  delay(1000);

  // ----------------------------------------------------------
  // GPS
  // ----------------------------------------------------------

  if (gps.location.isValid()) {

    currentLat = gps.location.lat();
    currentLng = gps.location.lng();

    Serial.println("[GPS] GPS FIX AVAILABLE");

    Serial.print("[GPS] Latitude: ");
    Serial.println(currentLat, 6);

    Serial.print("[GPS] Longitude: ");
    Serial.println(currentLng, 6);

  } else {

    Serial.println("[GPS] No GPS fix");
    Serial.println("[GPS] Using default coordinates");
  }

  // ----------------------------------------------------------
  // CREATE JSON PACKET
  // ----------------------------------------------------------

  String packet =
    "{\"type\":\"" +
    triggerType +
    "\",\"lat\":" +
    String(currentLat, 6) +
    ",\"lng\":" +
    String(currentLng, 6) +
    "}";

  Serial.println();
  Serial.println("[LORA] Packet:");
  Serial.println(packet);

  // ----------------------------------------------------------
  // OLED: TRANSMITTING
  // ----------------------------------------------------------

  updateOLED(
    "TRANSMITTING SOS",
    "LoRa Sending...",
    "Please Wait..."
  );

  delay(300);

  // ----------------------------------------------------------
  // TRANSMIT
  // ----------------------------------------------------------

  Serial.println("[LORA] Transmitting...");

  int state = radio.transmit(packet);

  // ----------------------------------------------------------
  // TRANSMISSION SUCCESS
  // ----------------------------------------------------------

  if (state == RADIOLIB_ERR_NONE) {

    Serial.println();
    Serial.println("[LORA] TRANSMISSION SUCCESS");

    updateOLED(
      "SOS SENT!",
      "LoRa SUCCESS",
      "Alert Delivered"
    );

  }

  // ----------------------------------------------------------
  // TRANSMISSION FAILED
  // ----------------------------------------------------------

  else {

    Serial.println();
    Serial.print("[LORA] TRANSMISSION FAILED");
    Serial.print(" Error: ");
    Serial.println(state);

    updateOLED(
      "SOS FAILED!",
      "LoRa ERROR",
      "Code: " + String(state)
    );
  }

  // Keep result on OLED
  delay(3000);

  // ----------------------------------------------------------
  // RETURN TO READY
  // ----------------------------------------------------------

  updateOLED(
    "EMERGENCY BOX",
    "SYSTEM READY",
    "WiFi: Emergency-Box"
  );

  Serial.println();
  Serial.println("[SYSTEM] Emergency Box READY");
  Serial.println();
}

// ============================================================
// WEB PAGE
// ============================================================

void handleRoot() {

  String html =
    "<!DOCTYPE html>"
    "<html>"
    "<head>"

    "<meta name='viewport' "
    "content='width=device-width, initial-scale=1'>"

    "<style>"

    "body{"
    "font-family:Arial;"
    "text-align:center;"
    "background:#121212;"
    "color:white;"
    "padding:20px;"
    "}"

    ".card{"
    "background:#1e1e1e;"
    "padding:20px;"
    "border-radius:15px;"
    "}"

    "h1{"
    "color:#ff4d4d;"
    "}"

    ".btn{"
    "color:white;"
    "padding:18px;"
    "width:100%;"
    "font-size:20px;"
    "border:none;"
    "border-radius:10px;"
    "margin:10px 0;"
    "font-weight:bold;"
    "}"

    ".sos{"
    "background:#ff4d4d;"
    "}"

    ".med{"
    "background:#ff9800;"
    "}"

    "</style>"

    "</head>"

    "<body>"

    "<div class='card'>"

    "<h1>OFFGRID SOS PORTAL</h1>"

    "<p>No Cellular / Internet Needed</p>"

    "<hr>"

    "<form action='/sos' method='GET'>"
    "<button class='btn sos'>"
    "CRITICAL SOS"
    "</button>"
    "</form>"

    "<form action='/medical' method='GET'>"
    "<button class='btn med'>"
    "MEDICAL ASSISTANCE"
    "</button>"
    "</form>"

    "</div>"

    "</body>"
    "</html>";

  server.send(
    200,
    "text/html",
    html
  );
}

// ============================================================
// WEB CRITICAL SOS
// ============================================================

void handleSOS() {

  Serial.println();
  Serial.println("[WEB] CRITICAL SOS REQUEST");

  sendLoRaSOS(
    "WEB_CRITICAL_SOS"
  );

  server.send(
    200,
    "text/html",
    "<h2 style='color:green;text-align:center;'>"
    "Alert Sent via LoRa!"
    "</h2>"
    "<p style='text-align:center;'>"
    "<a href='/'>Back</a>"
    "</p>"
  );
}

// ============================================================
// WEB MEDICAL SOS
// ============================================================

void handleMedical() {

  Serial.println();
  Serial.println("[WEB] MEDICAL ASSISTANCE REQUEST");

  sendLoRaSOS(
    "WEB_MEDICAL_AID"
  );

  server.send(
    200,
    "text/html",
    "<h2 style='color:orange;text-align:center;'>"
    "Medical Request Sent!"
    "</h2>"
    "<p style='text-align:center;'>"
    "<a href='/'>Back</a>"
    "</p>"
  );
}

// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("========================================");
  Serial.println("       EMERGENCY BOX TRANSMITTER");
  Serial.println("========================================");

  // ==========================================================
  // BUTTON
  // ==========================================================

  pinMode(
    BUTTON_PIN,
    INPUT_PULLDOWN
  );

  Serial.println(
    "[BUTTON] GPIO 4 READY"
  );

  // ==========================================================
  // OLED
  // ==========================================================

  Serial.println(
    "[OLED] Initializing..."
  );

  Wire.begin(
    OLED_SDA,
    OLED_SCL
  );

  if (
    !display.begin(
      SSD1306_SWITCHCAPVCC,
      0x3C
    )
  ) {

    Serial.println(
      "[OLED] ERROR - OLED NOT FOUND"
    );

  } else {

    Serial.println(
      "[OLED] OLED READY"
    );

    updateOLED(
      "SYSTEM BOOTING",
      "Initializing...",
      "Please Wait..."
    );
  }

  // ==========================================================
  // LORA
  // ==========================================================

  Serial.println();
  Serial.println(
    "[LORA] Initializing SX1278..."
  );

  int state = radio.begin(
    433.0,
    125.0,
    9,
    7,
    0x12,
    10,
    8
  );

  if (state == RADIOLIB_ERR_NONE) {

    Serial.println(
      "[LORA] SX1278 READY"
    );

    updateOLED(
      "LORA READY",
      "Initializing GPS",
      "Please Wait..."
    );

  } else {

    Serial.print(
      "[LORA] SX1278 FAILED"
    );

    Serial.print(
      " Error: "
    );

    Serial.println(state);

    updateOLED(
      "LORA ERROR!",
      "Check SX1278",
      "Error: " + String(state)
    );

    delay(3000);
  }

  // ==========================================================
  // GPS
  // ==========================================================

  Serial.println();
  Serial.println(
    "[GPS] Initializing GPS..."
  );

  gpsSerial.begin(
    9600,
    SERIAL_8N1,
    GPS_RX,
    GPS_TX
  );

  Serial.println(
    "[GPS] GPS SERIAL READY"
  );

  updateOLED(
    "GPS READY",
    "Starting WiFi",
    "Please Wait..."
  );

  // ==========================================================
  // WIFI ACCESS POINT
  // ==========================================================

  Serial.println();
  Serial.println(
    "[WIFI] Starting Access Point..."
  );

  WiFi.softAP(
    "Emergency-Box",
    "12345678"
  );

  Serial.print(
    "[WIFI] SSID: "
  );

  Serial.println(
    "Emergency-Box"
  );

  Serial.print(
    "[WIFI] IP: "
  );

  Serial.println(
    WiFi.softAPIP()
  );

  // ==========================================================
  // WEB SERVER
  // ==========================================================

  server.on(
    "/",
    handleRoot
  );

  server.on(
    "/sos",
    handleSOS
  );

  server.on(
    "/medical",
    handleMedical
  );

  server.begin();

  Serial.println(
    "[WEB] Web Server READY"
  );

  // ==========================================================
  // FINAL READY SCREEN
  // ==========================================================

  updateOLED(
    "EMERGENCY BOX",
    "SYSTEM READY",
    "Press SOS Button"
  );

  Serial.println();
  Serial.println("========================================");
  Serial.println("          SYSTEM READY");
  Serial.println("========================================");
  Serial.println();
}

// ============================================================
// LOOP
// ============================================================

void loop() {

  // ----------------------------------------------------------
  // WEB SERVER
  // ----------------------------------------------------------

  server.handleClient();

  // ----------------------------------------------------------
  // GPS DATA
  // ----------------------------------------------------------

  while (
    gpsSerial.available() > 0
  ) {

    gps.encode(
      gpsSerial.read()
    );
  }

  // ----------------------------------------------------------
  // HARDWARE BUTTON
  // ----------------------------------------------------------

  if (
    digitalRead(BUTTON_PIN) == HIGH
  ) {

    Serial.println();
    Serial.println(
      "[BUTTON] BUTTON PRESSED!"
    );

    // Show button status
    updateOLED(
      "BUTTON PRESSED!",
      "SOS ACTIVATED",
      "Please Wait..."
    );

    delay(1000);

    // Send SOS
    sendLoRaSOS(
      "HARDWARE_BUTTON_SOS"
    );

    // Wait until button released
    while (
      digitalRead(BUTTON_PIN) == HIGH
    ) {

      delay(10);
    }

    Serial.println(
      "[BUTTON] Button released"
    );

    delay(300);
  }
}