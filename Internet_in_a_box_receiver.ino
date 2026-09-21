---------------------------FINAL RECEIVER CODE--------------------------------------------

#include <Arduino.h>
#include <RadioLib.h>

// ============================================================
// LoRa-02 / Ra-02 SX1278
// ============================================================

#define LORA_CS    10
#define LORA_DIO0  2
#define LORA_RST   5

SX1278 radio = new Module(
  LORA_CS,
  LORA_DIO0,
  LORA_RST
);

// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("========================================");
  Serial.println("       RESCUE RECEIVER NODE");
  Serial.println("========================================");

  Serial.println();
  Serial.println("[LORA] Initializing SX1278...");

  int state = radio.begin(
    433.0,   // Frequency
    125.0,   // Bandwidth
    9,       // Spreading Factor
    7,       // Coding Rate
    0x12,    // Sync Word
    10,      // Output Power
    8        // Preamble
  );

  if (state == RADIOLIB_ERR_NONE) {

    Serial.println("[LORA] SX1278 READY");

    Serial.println();
    Serial.println("========================================");
    Serial.println("       RECEIVER SYSTEM READY");
    Serial.println("========================================");

    Serial.println();
    Serial.println("[STATUS] Listening for SOS...");
    Serial.println();

  } else {

    Serial.print("[ERROR] SX1278 INIT FAILED");
    Serial.print(" Error: ");
    Serial.println(state);

    Serial.println();
    Serial.println("[CHECK] Check LoRa wiring.");

    while (true) {
      delay(1000);
    }
  }
}

// ============================================================
// LOOP
// ============================================================

void loop() {

  String incomingData;

  Serial.println("[LORA] Waiting for packet...");

  int state = radio.receive(incomingData);

  if (state == RADIOLIB_ERR_NONE) {

    Serial.println();
    Serial.println("========================================");
    Serial.println("      !!! EMERGENCY ALERT !!!");
    Serial.println("========================================");

    Serial.println();

    Serial.println("[RECEIVER] Packet received!");

    Serial.print("[PAYLOAD] ");
    Serial.println(incomingData);

    Serial.print("[RSSI] ");
    Serial.print(radio.getRSSI());
    Serial.println(" dBm");

    Serial.print("[SNR] ");
    Serial.print(radio.getSNR());
    Serial.println(" dB");

    Serial.println();

    Serial.println("========================================");
    Serial.println("      SOS ALERT RECEIVED");
    Serial.println("========================================");

    Serial.println();

  } 
  else {

    Serial.print("[LORA] Receive error: ");
    Serial.println(state);
  }

  delay(100);
}
