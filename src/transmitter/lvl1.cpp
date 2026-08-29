#include <Arduino.h>

#include <RadioLib.h>

#include <Wire.h>

#include <Adafruit_GFX.h>

#include <Adafruit_SSD1306.h>



// ============================================================

// LoRa SX1276 pins on LILYGO TTGO LoRa32 V2.1

// ============================================================



#define LORA_CS    18

#define LORA_DIO0  26

#define LORA_DIO1  33





// ============================================================

// OLED configuration

// ============================================================



#define SCREEN_WIDTH 128

#define SCREEN_HEIGHT 64



// -1 means that OLED reset pin is not controlled separately.

#define OLED_RESET -1



// Most TTGO LoRa32 V2.1 OLED displays use I2C address 0x3C.

#define OLED_ADDRESS 0x3C



Adafruit_SSD1306 display(

    SCREEN_WIDTH,

    SCREEN_HEIGHT,

    &Wire,

    OLED_RESET

);





// ============================================================

// Create SX1276 radio object

// ============================================================



SX1276 radio = new Module(

    LORA_CS,

    LORA_DIO0,

    LORA_RST,

    LORA_DIO1

);





// ============================================================

// USER SETTINGS

// ============================================================



// Both devices MUST use exactly the same frequency.

//

// We use 923 MHz because one of your boards is marked 923 MHz.

const float LORA_FREQUENCY = 923.0;



// Transmission power in dBm.

//

// Try changing this value to experiment:

//

//  2 dBm  -> very weak

//  5 dBm  -> weak

//  10 dBm -> medium

//  14 dBm -> strong

//  17 dBm -> maximum continuous PA_BOOST level according to RadioLib

//

// Start with 5 and increase it later.

const int8_t TX_POWER = 5;





// ============================================================

// Message counter

// ============================================================



int messageNumber = 0;





void setup() {



    // --------------------------------------------------------

    // Start Serial Monitor

    // --------------------------------------------------------



    Serial.begin(115200);

    delay(1000);



    Serial.println();

    Serial.println("============================");

    Serial.println("LoRa TRANSMITTER");

    Serial.println("============================");





    // --------------------------------------------------------

    // Initialize OLED

    // --------------------------------------------------------

    Wire.begin(21, 22);

    if (!display.begin(

            SSD1306_SWITCHCAPVCC,

            OLED_ADDRESS

        )) {



        Serial.println("OLED initialization failed!");



        // Stop here if OLED cannot be initialized.

        while (true) {

            delay(1000);

        }

    }



    display.clearDisplay();

    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(1);



    display.setCursor(0, 0);

    display.println("LoRa TRANSMITTER");

    display.println();

    display.println("Initializing...");



    display.display();





    // --------------------------------------------------------

    // Initialize LoRa

    // --------------------------------------------------------



    Serial.println("Initializing SX1276...");



    /*

     * begin() parameters:

     *

     * frequency      = 923 MHz

     * bandwidth      = 125 kHz

     * spreading      = 9

     * coding rate    = 4/7

     * sync word      = default

     * power          = TX_POWER

     * preamble       = 8 symbols

     * gain           = automatic

     */



    int state = radio.begin(

        LORA_FREQUENCY,

        125.0,

        9,

        7,

        RADIOLIB_SX127X_SYNC_WORD,

        TX_POWER,

        8,

        0

    );





    // --------------------------------------------------------

    // Check initialization result

    // --------------------------------------------------------



    if (state != RADIOLIB_ERR_NONE) {



        Serial.print("LoRa initialization failed!");

        Serial.print(" Error code: ");

        Serial.println(state);



        display.clearDisplay();

        display.setCursor(0, 0);



        display.println("LoRa ERROR!");

        display.println();

        display.print("Code: ");

        display.println(state);



        display.display();



        while (true) {

            delay(1000);

        }

    }





    // --------------------------------------------------------

    // LoRa initialized successfully

    // --------------------------------------------------------



    Serial.println("LoRa initialized successfully!");



    Serial.print("Frequency: ");

    Serial.print(LORA_FREQUENCY);

    Serial.println(" MHz");



    Serial.print("TX power: ");

    Serial.print(TX_POWER);

    Serial.println(" dBm");





    display.clearDisplay();

    display.setCursor(0, 0);



    display.println("TRANSMITTER");

    display.println();



    display.print("Freq: ");

    display.print(LORA_FREQUENCY);

    display.println(" MHz");



    display.print("Power: ");

    display.print(TX_POWER);

    display.println(" dBm");



    display.println();

    display.println("READY");



    display.display();



    delay(2000);

}





void loop() {



    // Increase message counter.

    messageNumber++;





    // --------------------------------------------------------

    // Create message

    // --------------------------------------------------------



    String message =

        "Hello! #" + String(messageNumber);





    // --------------------------------------------------------

    // Print message to Serial Monitor

    // --------------------------------------------------------



    Serial.println();

    Serial.println("----------------------------");



    Serial.print("Sending: ");

    Serial.println(message);





    // --------------------------------------------------------

    // Show message on OLED

    // --------------------------------------------------------



    display.clearDisplay();



    display.setCursor(0, 0);



    display.println("TRANSMITTER");

    display.println();



    display.print("Power: ");

    display.print(TX_POWER);

    display.println(" dBm");



    display.println();



    display.println("Sending:");

    display.println(message);



    display.display();





    // --------------------------------------------------------

    // Transmit the LoRa packet

    // --------------------------------------------------------



    int state = radio.transmit(message);





    // --------------------------------------------------------

    // Check transmission result

    // --------------------------------------------------------



    if (state == RADIOLIB_ERR_NONE) {



        Serial.println("Transmission successful!");



        display.println();

        display.println("Sent OK");



    } else {



        Serial.print("Transmission failed!");

        Serial.print(" Error code: ");

        Serial.println(state);



        display.println();

        display.print("ERROR: ");

        display.println(state);

    }



    display.display();





    // --------------------------------------------------------

    // Wait before sending next packet

    // --------------------------------------------------------



    delay(2000);

} 