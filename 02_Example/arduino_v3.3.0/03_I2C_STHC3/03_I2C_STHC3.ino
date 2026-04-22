#include "port_shtc3.h"
#include "port_i2c.h"
#include "epaper_config.h"

static I2cMasterBus *i2c_bus = NULL;

void Shtc3_LoopTask(void *arg) {
  float t, h;
  while (1) {
    Shtc3_ReadTempHumi(&t, &h);
    if (t != -1000 && h != -1000) {
      Serial.printf("temperature:%.2f°,humidity:%.2f%%\n", t, h);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  i2c_bus = I2cMasterBus::requestInstance(ESP32_I2C_SCL_PIN, ESP32_I2C_SDA_PIN, ESP32_I2C_DEV_NUM);
  assert(i2c_bus);
  Shtc3_Init(i2c_bus);
  xTaskCreatePinnedToCore(Shtc3_LoopTask, "Shtc3_LoopTask", 4 * 1024, NULL, 4, NULL, 0);
}

void loop() {
}
