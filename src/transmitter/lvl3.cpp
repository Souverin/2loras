#include <Arduino.h>
#include <RadioLib.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define LORA_CS    18
#define LORA_DIO0  26
#define LORA_RST   23
#define LORA_DIO1  33

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SX1276 radio = new Module(LORA_CS, LORA_DIO0, LORA_RST, LORA_DIO1);

const float FREQUENCY = 923.0;
const float BITRATE = 15.2;   // кбіт/с
const float FREQ_DEV = 15.2;  // кГц
const float RX_BW = 125.0;    // кГц
const int8_t TX_POWER = 5;

uint8_t payload[64];
uint32_t packetCounter = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n--- FSK TRANSMITTER (Level 3) ---");

    Wire.begin(21, 22);
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println("[OLED] Init Failed!");
        while (true) delay(1000);
    }
    
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("FSK TRANSMITTER");
    display.println("Init Radio...");
    display.display();

    int state = radio.beginFSK(FREQUENCY, BITRATE, FREQ_DEV, RX_BW, TX_POWER);
    if (state != RADIOLIB_ERR_NONE) {
        Serial.printf("[FSK] Init Failed, code: %d\n", state);
        display.println("Radio Init Failed!");
        display.display();
        while (true) delay(1000);
    }

    uint8_t syncWord[] = {0x12, 0xAD};
    radio.setSyncWord(syncWord, 2);
    radio.fixedPacketLengthMode(64);
    radio.setCRC(true, false); // Вмикаємо CRC у режимі CCITT

    Serial.println("[FSK] Ready to transmit.");
}

void loop() {
    packetCounter++;

    memset(payload, 0xAA, sizeof(payload));
    payload[0] = (packetCounter >> 24) & 0xFF;
    payload[1] = (packetCounter >> 16) & 0xFF;
    payload[2] = (packetCounter >> 8) & 0xFF;
    payload[3] = packetCounter & 0xFF;

    unsigned long startTime = micros();
    int state = radio.transmit(payload, 64);
    float txTimeMs = (micros() - startTime) / 1000.0;

    if (state == RADIOLIB_ERR_NONE) {
        Serial.printf("[TX #%u] Sent 64B | ToA: %.2f ms\n", packetCounter, txTimeMs);

        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("--- FSK TX ---");
        display.println();
        display.printf("Packet: #%u\n", packetCounter);
        display.printf("Size:   64 Bytes\n");
        display.printf("ToA:    %.1f ms\n", txTimeMs);
        display.display();
    } else {
        Serial.printf("[TX ERROR] Code: %d\n", state);
    }

    delay(2000);
}