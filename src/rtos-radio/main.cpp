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

enum ButtonEventType {
  EVENT_NONE,
  EVENT_SINGLE_CLICK,
  EVENT_DOUBLE_CLICK
};

// Структура події, яка містить тип та час натискання
struct ButtonMessage {
  ButtonEventType type;
  unsigned long pressTimestampMs; // Мітка часу в мілісекундах (millis)
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
  unsigned long initialPressTimestamp = 0;
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
          initialPressTimestamp = millis();
        }
        lastPressTime = now;
      }
    }
    lastState = currentState;

    if (clickCount > 0 && (xTaskGetTickCount() - lastPressTime > doubleClickDelay)) {
      ButtonMessage msg;
      msg.type = (clickCount == 1) ? EVENT_SINGLE_CLICK : EVENT_DOUBLE_CLICK;
      msg.pressTimestampMs = initialPressTimestamp;
      xQueueSend(buttonQueue, &msg, portMAX_DELAY);
      clickCount = 0;
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

// ---------------------------------------------------------
// 2. RADIO TASK (RadioLib)
// ---------------------------------------------------------
void vRadioTask(void *pvParameters) {
  ButtonMessage receivedMsg;

  for (;;) {
    if (xQueueReceive(buttonQueue, &receivedMsg, portMAX_DELAY) == pdPASS) {
      String message = "";
      unsigned long nowMs = millis();
      unsigned long latencyMs = nowMs - receivedMsg.pressTimestampMs;

      if (receivedMsg.type == EVENT_SINGLE_CLICK) {
        message = "BUTTON SINGLE";
      } else if (receivedMsg.type == EVENT_DOUBLE_CLICK) {
        message = "BUTTON DOUBLE";
      }


      Serial.printf(
        "[Radio Task] Sending '%s' (Latency: %lu ms) via RadioLib... ",
        message.c_str(),
        latencyMs
      );
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
  buttonQueue = xQueueCreate(5, sizeof(ButtonMessage));

  // Створення задач
  xTaskCreate(vButtonTask, "Button Task", 2048, NULL, 2, NULL);
  xTaskCreate(vRadioTask, "Radio Task", 3072, NULL, 1, NULL);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}