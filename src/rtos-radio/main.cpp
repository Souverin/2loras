#include <Arduino.h>
#include <RadioLib.h>

// Піни для TTGO LoRa32 v2.1
#define LORA_CS    18
#define LORA_DIO0  26
#define RST   14
#define LORA_DIO1  33

#define BUTTON_PIN 12

// Створення об'єкта модуля SX1276 через RadioLib
SX1276 radio = new Module(LORA_CS, LORA_DIO0, RST, LORA_DIO1);

enum ButtonEvent {
  EVENT_NONE,
  EVENT_SINGLE_CLICK,
  EVENT_DOUBLE_CLICK
};

QueueHandle_t buttonQueue;

// ---------------------------------------------------------
// 1. BUTTON TASK
// ---------------------------------------------------------
void vButtonTask(void *pvParameters) {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  const TickType_t debounceDelay = pdMS_TO_TICKS(50);
  const TickType_t doubleClickDelay = pdMS_TO_TICKS(350);

  bool lastState = HIGH;
  TickType_t lastPressTime = 0;
  int clickCount = 0;

  for (;;) {
    bool currentState = digitalRead(BUTTON_PIN);

    if (lastState == HIGH && currentState == LOW) {
      vTaskDelay(debounceDelay);
      if (digitalRead(BUTTON_PIN) == LOW) {
        TickType_t now = xTaskGetTickCount();
        if (now - lastPressTime < doubleClickDelay) {
          clickCount++;
        } else {
          clickCount = 1;
        }
        lastPressTime = now;
      }
    }
    lastState = currentState;

    if (clickCount > 0 && (xTaskGetTickCount() - lastPressTime > doubleClickDelay)) {
      ButtonEvent eventToSend = (clickCount == 1) ? EVENT_SINGLE_CLICK : EVENT_DOUBLE_CLICK;
      xQueueSend(buttonQueue, &eventToSend, portMAX_DELAY);
      clickCount = 0;
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

// ---------------------------------------------------------
// 2. RADIO TASK (RadioLib)
// ---------------------------------------------------------
void vRadioTask(void *pvParameters) {
  ButtonEvent receivedEvent;

  for (;;) {
    if (xQueueReceive(buttonQueue, &receivedEvent, portMAX_DELAY) == pdPASS) {
      String message = "";

      if (receivedEvent == EVENT_SINGLE_CLICK) {
        message = "BUTTON SINGLE";
      } else if (receivedEvent == EVENT_DOUBLE_CLICK) {
        message = "BUTTON DOUBLE";
      }

      Serial.print("[Radio Task] Transmitting via RadioLib: ");
      Serial.print(message);
      Serial.print(" ... ");

      // Відправка пакета через RadioLib
      int state = radio.transmit(message);

      if (state == RADIOLIB_ERR_NONE) {
        Serial.println("SUCCESS!");
      } else {
        Serial.print("FAILED, code: ");
        Serial.println(state);
      }
    }
  }
}

// ---------------------------------------------------------
// SETUP
// ---------------------------------------------------------
void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("\n=== FreeRTOS Queue & RadioLib Init ===");

  // Ініціалізація LoRa радіомодуля (868.0 МГц)
  int state = radio.begin(868.0);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("Radio initialized successfully!");
  } else {
    Serial.print("Radio initialization failed, code: ");
    Serial.println(state);
  }

  // Створення черги
  buttonQueue = xQueueCreate(5, sizeof(ButtonEvent));

  // Створення задач
  xTaskCreate(vButtonTask, "Button Task", 2048, NULL, 2, NULL);
  xTaskCreate(vRadioTask, "Radio Task", 3072, NULL, 1, NULL);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}