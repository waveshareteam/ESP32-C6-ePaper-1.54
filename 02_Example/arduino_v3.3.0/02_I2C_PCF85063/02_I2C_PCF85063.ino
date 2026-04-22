#include "pcf85063a.h"
#include "port_i2c.h"
#include "epaper_config.h"

static I2cMasterBus *i2c_bus = NULL;
static pcf85063a_dev_t pcf85063;   //  rtc句柄
static bool pcf85063initflag = 1;  //  rtc初始化成功标志位

void Rtc_LoopTask(void *arg) {
  if (pcf85063initflag) {
    pcf85063a_datetime_t datatime = {};
    datatime.year = 2026;
    datatime.month = 1;
    datatime.day = 1;
    datatime.hour = 8;
    datatime.min = 0;
    datatime.sec = 0;
    pcf85063a_set_time_date(&pcf85063, datatime);
  }
  while (1) {
    if (pcf85063initflag) {
      pcf85063a_datetime_t current_time = {};
      pcf85063a_get_time_date(&pcf85063, &current_time);
      Serial.printf("%04d/%02d/%02d %02d:%02d:%02d\n", current_time.year, current_time.month, current_time.day, current_time.hour,
               current_time.min, current_time.sec);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  i2c_bus = I2cMasterBus::requestInstance(ESP32_I2C_SCL_PIN, ESP32_I2C_SDA_PIN, ESP32_I2C_DEV_NUM);
  assert(i2c_bus);
  esp_err_t ret = pcf85063a_init(&pcf85063, i2c_bus->Get_I2cBusHandle(), PCF85063A_ADDRESS);
  if (ret != ESP_OK) {
    Serial.printf("Failed to initialize PCF85063A (error: %d)\n", ret);
    pcf85063initflag = 0;
  }
  xTaskCreatePinnedToCore(Rtc_LoopTask, "Rtc_LoopTask", 4 * 1024, NULL, 4, NULL, 0);
}

void loop() {
}
