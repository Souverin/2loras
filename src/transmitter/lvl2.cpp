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
const int8_t TX_POWER = 5;

// Створюємо фіксований буфер розміром 64 байти
uint8_t payload[64]; // <--- НОВЕ


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
    display.println("TRANSMITTER");
    display.println("Initializing...");
    display.display();

    // Заповнюємо 64 байти символами 'A'
    memset(payload, 'A', sizeof(payload)); // <--- НОВЕ

    int state = radio.begin(
        LORA_FREQUENCY,
        500.0,
        12,
        7,
        RADIOLIB_SX127X_SYNC_WORD,
        TX_POWER,
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
    // Вимірюємо точний час виконання функції transmit()
    unsigned long startTime = micros(); // <--- НОВЕ
    int state = radio.transmit(payload, 64); // <--- ЗМІНЕНО (Відправка 64 байт)
    float txTimeMs = (micros() - startTime) / 1000.0; // <--- НОВЕ (Час у мс)

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("--- TRANSMITTER ---");
    display.println();

    if (state == RADIOLIB_ERR_NONE) {
        display.println("Status: Sent OK");
        display.println("Size:   64 Bytes");
        display.println();
        display.print("TX Time: ");
        display.print(txTimeMs, 1); // <--- НОВЕ (Вивід часу на OLED)
        display.println(" ms");
    } else {
        display.print("ERROR: ");
        display.println(state);
    }

    display.display();
    delay(2000);
}