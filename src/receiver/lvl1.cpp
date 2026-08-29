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



#define OLED_RESET -1

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



// IMPORTANT:

//

// The receiver must use EXACTLY THE SAME frequency

// as the transmitter.

//

// We therefore also use 923 MHz here.

const float LORA_FREQUENCY = 923.0;





void setup() {



    // --------------------------------------------------------

    // Start Serial Monitor

    // --------------------------------------------------------



    Serial.begin(115200);

    delay(1000);



    Serial.println();

    Serial.println("============================");

    Serial.println("LoRa RECEIVER");

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



        while (true) {

            delay(1000);

        }

    }



    display.clearDisplay();

    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(1);



    display.setCursor(0, 0);



    display.println("LoRa RECEIVER");

    display.println();

    display.println("Initializing...");



    display.display();





    // --------------------------------------------------------

    // Initialize LoRa

    // --------------------------------------------------------



    Serial.println("Initializing SX1276...");





    /*

     * The LoRa configuration must match the transmitter.

     *

     * Frequency:     923 MHz

     * Bandwidth:     125 kHz

     * Spreading:     9

     * Coding rate:   4/7

     * Sync word:     default

     *

     * Power is irrelevant for the receiver,

     * because the receiver does not transmit.

     *

     * We use 10 dBm here just as a normal initialization value.

     */



    int state = radio.begin(

        LORA_FREQUENCY,

        125.0,

        9,

        7,

        RADIOLIB_SX127X_SYNC_WORD,

        10,

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



    Serial.println("Waiting for packets...");





    display.clearDisplay();

    display.setCursor(0, 0);



    display.println("RECEIVER");

    display.println();



    display.print("Freq: ");

    display.print(LORA_FREQUENCY);

    display.println(" MHz");



    display.println();

    display.println("Waiting...");



    display.display();

}





void loop() {



    // --------------------------------------------------------

    // Variable that will contain received message

    // --------------------------------------------------------



    String receivedMessage;





    // --------------------------------------------------------

    // Wait for a LoRa packet

    //

    // radio.receive() blocks here until a packet arrives.

    // --------------------------------------------------------



    int state = radio.receive(receivedMessage);





    // --------------------------------------------------------

    // Packet received successfully

    // --------------------------------------------------------



    if (state == RADIOLIB_ERR_NONE) {



        // Get signal strength.

        //

        // RSSI is expressed in dBm.

        //

        // Example:

        // -40 dBm  = very strong

        // -70 dBm  = weaker

        // -100 dBm = very weak

        float rssi = radio.getRSSI();





        // Get signal-to-noise ratio.

        float snr = radio.getSNR();





        // ----------------------------------------------------

        // Serial Monitor

        // ----------------------------------------------------



        Serial.println();

        Serial.println("============================");

        Serial.println("PACKET RECEIVED");



        Serial.print("Message: ");

        Serial.println(receivedMessage);



        Serial.print("RSSI: ");

        Serial.print(rssi);

        Serial.println(" dBm");



        Serial.print("SNR: ");

        Serial.print(snr);

        Serial.println(" dB");





        // ----------------------------------------------------

        // OLED

        // ----------------------------------------------------



        display.clearDisplay();



        display.setCursor(0, 0);



        display.println("RECEIVER");

        display.println();



        display.println("Received:");

        display.println(receivedMessage);



        display.println();



        display.print("RSSI: ");

        display.print(rssi, 0);

        display.println(" dBm");



        display.print("SNR: ");

        display.print(snr, 1);

        display.println(" dB");



        display.display();





    } else {



        // ----------------------------------------------------

        // Reception error

        // ----------------------------------------------------



        Serial.print("Receive failed!");

        Serial.print(" Error code: ");

        Serial.println(state);





        display.clearDisplay();



        display.setCursor(0, 0);



        display.println("RECEIVER");

        display.println();



        display.println("Receive ERROR:");



        display.println(state);



        display.display();

    }

}