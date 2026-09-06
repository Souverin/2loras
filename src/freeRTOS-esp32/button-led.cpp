#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_SDA      21
#define OLED_SCL      22
#define OLED_ADDRESS  0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BUTTON_PIN 12
#define LED_PIN    25

const uint32_t intervals[] = {250, 500, 1000, 2000};
const uint8_t INTERVALS_COUNT = sizeof(intervals) / sizeof(intervals[0]);

// Структура для оновлення стану екрана
struct DisplayState {
  uint32_t selectedInterval;
  uint32_t activeInterval;
  bool ledState;
};

QueueHandle_t intervalQueue;     // Для зв'язку Button -> LED
QueueHandle_t displayUpdateQueue; // Для зв'язку (Button + LED) -> Display

// Задача 1: Кнопка (Core 0)
void TaskButton(void *pvParameters) {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  uint8_t currentIndex = 0;
  bool lastState = HIGH;
  uint32_t lastPressTime = 0;
  bool waitingForDouble = false;
  
  for (;;) {
    bool currentState = digitalRead(BUTTON_PIN);
    uint32_t now = millis();

    if (lastState == HIGH && currentState == LOW) {
      vTaskDelay(pdMS_TO_TICKS(50));
      
      if (waitingForDouble && (now - lastPressTime <= 300)) {
        currentIndex = (currentIndex == 0) ? (INTERVALS_COUNT - 1) : (currentIndex - 1);
        waitingForDouble = false;
        
        uint32_t newInt = intervals[currentIndex];
        
        // 1. Повідомляємо світлодіод
        xQueueSend(intervalQueue, &newInt, portMAX_DELAY);
        
        // 2. Повідомляємо дисплей про зміну Selected
        DisplayState stateMsg = { .selectedInterval = newInt, .activeInterval = 0, .ledState = false };
        xQueueSend(displayUpdateQueue, &stateMsg, 0);
      } else {
        waitingForDouble = true;
        lastPressTime = now;
      }
    }

    if (waitingForDouble && (now - lastPressTime > 300)) {
      currentIndex = (currentIndex + 1) % INTERVALS_COUNT;
      waitingForDouble = false;
      
      uint32_t newInt = intervals[currentIndex];
      
      xQueueSend(intervalQueue, &newInt, portMAX_DELAY);
      
      DisplayState stateMsg = { .selectedInterval = newInt, .activeInterval = 0, .ledState = false };
      xQueueSend(displayUpdateQueue, &stateMsg, 0);
    }

    lastState = currentState;
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

// Задача 2: Світлодіод (Core 1, Високий пріоритет)
void TaskLED(void *pvParameters) {
  pinMode(LED_PIN, OUTPUT);
  uint32_t currentInterval = 250; 
  bool ledState = LOW;

  for (;;) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);

    // Повідомляємо дисплей про зміну стану LED або активного інтервалу
    DisplayState stateMsg = { .selectedInterval = 0, .activeInterval = currentInterval, .ledState = ledState };
    xQueueSend(displayUpdateQueue, &stateMsg, 0);

    vTaskDelay(pdMS_TO_TICKS(currentInterval));

    uint32_t newInterval;
    if (xQueueReceive(intervalQueue, &newInterval, 0) == pdTRUE) {
      currentInterval = newInterval;
    }
  }
}

// Задача 3: Екран (Core 1, Низький пріоритет) — реактивна на події з черги
void TaskDisplay(void *pvParameters) {
  DisplayState currentState = { .selectedInterval = 250, .activeInterval = 250, .ledState = false };
  DisplayState incomingState;

  for (;;) {
    // Чекаємо на нові дані з черги екрана (поки даних немає, задача спить і НЕ витрачає CPU!)
    if (xQueueReceive(displayUpdateQueue, &incomingState, portMAX_DELAY) == pdTRUE) {
      
      // Оновлюємо тільки ті поля, які були реально передані
      if (incomingState.selectedInterval != 0) {
        currentState.selectedInterval = incomingState.selectedInterval;
      }
      if (incomingState.activeInterval != 0) {
        currentState.activeInterval = incomingState.activeInterval;
      }
      currentState.ledState = incomingState.ledState;

      // Малюємо
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      
      display.setCursor(0, 0);
      display.print("Selected: ");
      display.print(currentState.selectedInterval);
      display.print(" ms");

      display.setCursor(0, 20);
      display.print("Active:   ");
      display.print(currentState.activeInterval);
      display.print(" ms");

      display.setCursor(0, 45);
      display.print("LED State: ");
      display.print(currentState.ledState ? "ON" : "OFF");

      display.display();
    }
  }
}

void setup() {
  Serial.begin(115200);

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
  }

  // Створення черг
  intervalQueue = xQueueCreate(5, sizeof(uint32_t));
  displayUpdateQueue = xQueueCreate(10, sizeof(DisplayState));

  // Запуск задач
  xTaskCreatePinnedToCore(TaskButton,  "Button",  2048, NULL, 1, NULL, 0); 
  xTaskCreatePinnedToCore(TaskDisplay, "Display", 4096, NULL, 1, NULL, 1); 
  xTaskCreatePinnedToCore(TaskLED,     "LED",     2048, NULL, 2, NULL, 1); 
}

void loop() {
  vTaskDelete(NULL);
}