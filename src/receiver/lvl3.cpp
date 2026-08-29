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

uint8_t rxBuffer[64];

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n--- FSK RECEIVER (Level 3) ---");

    Wire.begin(21, 22);
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println("[OLED] Init Failed!");
        while (true) delay(1000);
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("FSK RECEIVER");
    display.println("Init Radio...");
    display.display();

    int state = radio.beginFSK(FREQUENCY, BITRATE, FREQ_DEV, RX_BW, 10);
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

    Serial.println("[FSK] Listening for packets...");

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("--- FSK RX ---");
    display.println();
    display.println("Status: Listening...");
    display.display();
}

void loop() {
    int state = radio.receive(rxBuffer, 64);

    if (state == RADIOLIB_ERR_NONE) {
        float rssi = radio.getRSSI();
        uint32_t counter = (rxBuffer[0] << 24) | (rxBuffer[1] << 16) | (rxBuffer[2] << 8) | rxBuffer[3];

        Serial.printf("[RX #%u] RSSI: %.1f dBm | Status: OK\n", counter, rssi);

        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("--- FSK RX ---");
        display.println();
        display.printf("Packet: #%u\n", counter);
        display.printf("RSSI:   %.0f dBm\n", rssi);
        display.printf("Status: SUCCESS\n");
        display.display();
    } 
    else if (state != RADIOLIB_ERR_RX_TIMEOUT) {
        Serial.printf("[RX ERROR] Code: %d\n", state);
    }
}