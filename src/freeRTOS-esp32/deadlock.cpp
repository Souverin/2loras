#include <Arduino.h>
#include <esp_task_wdt.h>

SemaphoreHandle_t mutexA;
SemaphoreHandle_t mutexB;

// Таймаут вочдога в секундах
#define WDT_TIMEOUT_SEC 3

void TaskAlpha(void *pvParameters) {
  // Підписуємо поточну задачу на Task Watchdog
  esp_task_wdt_add(NULL);

  for (;;) {
    // Скидаємо таймер вочдога
    esp_task_wdt_reset();

    Serial.println("[Task Alpha] Намагається захопити Mutex A...");
    if (xSemaphoreTake(mutexA, portMAX_DELAY) == pdTRUE) {
      Serial.println("[Task Alpha] УСПІХ: Mutex A захоплено!");

      vTaskDelay(pdMS_TO_TICKS(100));

      Serial.println("[Task Alpha] Намагається захопити Mutex B...");
      // Застрягне тут. esp_task_wdt_reset() більше не викликатиметься
      xSemaphoreTake(mutexB, portMAX_DELAY);

      xSemaphoreGive(mutexB);
      xSemaphoreGive(mutexA);
    }
  }
}

void TaskBeta(void *pvParameters) {
  esp_task_wdt_add(NULL);

  for (;;) {
    esp_task_wdt_reset();

    Serial.println("[Task Beta] Намагається захопити Mutex B...");
    if (xSemaphoreTake(mutexB, portMAX_DELAY) == pdTRUE) {
      Serial.println("[Task Beta] УСПІХ: Mutex B захоплено!");

      vTaskDelay(pdMS_TO_TICKS(100));

      Serial.println("[Task Beta] Намагається захопити Mutex A...");
      xSemaphoreTake(mutexA, portMAX_DELAY);

      xSemaphoreGive(mutexA);
      xSemaphoreGive(mutexB);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n=== Deadlock Test з активним Watchdog ===");

  // Ініціалізація WDT для ESP-IDF v4.x: (таймаут в секундах, trigger_panic = true)
  esp_task_wdt_init(WDT_TIMEOUT_SEC, true);

  mutexA = xSemaphoreCreateMutex();
  mutexB = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(TaskAlpha, "TaskAlpha", 2048, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(TaskBeta,  "TaskBeta",  2048, NULL, 2, NULL, 1);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}