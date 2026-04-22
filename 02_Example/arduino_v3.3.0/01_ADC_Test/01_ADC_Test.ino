#include "port_adc.h"

void Adc_LoopTask(void *arg) {
  while (1) {
    Serial.printf("vol:%dmV,Capacity:%d%%\n", (uint16_t)(Get_VbatVoltage() * 1000), Get_Batterylevel());
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  BoardAdc_Init();
  xTaskCreatePinnedToCore(Adc_LoopTask, "Adc_LoopTask", 4 * 1024, NULL, 4, NULL, 0);
}

void loop() {
}
