#include <Arduino.h>
#include <RadioLib.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define LORA_CS    18
#define LORA_DIO0  26
#define LORA_DIO1  33

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SX1276 radio = new Module(LORA_CS, LORA_DIO0, LORA_RST, LORA_DIO1);

const float LORA_FREQUENCY = 923.0;


void setup() {
    Serial.begin(115200);
    delay(1000);

    Wire.begin(21, 22); // <--- ЗМІНЕНО (Явне визначення I2C)

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        while (true) { delay(1000); }
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("RECEIVER");
    display.println("Waiting...");
    display.display();

    int state = radio.begin(
        LORA_FREQUENCY,
        500.0,
        12,
        7,
        RADIOLIB_SX127X_SYNC_WORD,
        10,
        8,
        0
    );

    if (state != RADIOLIB_ERR_NONE) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("LoRa ERROR!");
        display.print("Code: ");
        display.println(state);
        display.display();
        while (true) { delay(1000); }
    }
}


void loop() {
    uint8_t buffer[64]; // <--- ЗМІНЕНО (Буфер під 64 байти)

    unsigned long startTime = micros(); // <--- НОВЕ
    int state = radio.receive(buffer, 64); // <--- ЗМІНЕНО
    float rxTimeMs = (micros() - startTime) / 1000.0; // <--- НОВЕ (Тривалість прийому)

    if (state == RADIOLIB_ERR_NONE) {
        float rssi = radio.getRSSI();
        float snr = radio.getSNR();

        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("--- RECEIVER ---");
        display.println();
        display.print("RSSI: ");
        display.print(rssi, 0);
        display.println(" dBm");

        display.print("SNR:  ");
        display.print(snr, 1);
        display.println(" dB");

        display.print("Time: ");
        display.print(rxTimeMs, 1); // <--- НОВЕ (Вивід часу на OLED)
        display.println(" ms");

        display.display();

    } else if (state == RADIOLIB_ERR_RX_TIMEOUT) { // <--- НОВЕ (Фіксуємо таймаут без збою дисплея)
        // Продовжуємо очікування
    } else {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("RECEIVER ERROR");
        display.print("Code: ");
        display.println(state);
        display.display();
    }
}