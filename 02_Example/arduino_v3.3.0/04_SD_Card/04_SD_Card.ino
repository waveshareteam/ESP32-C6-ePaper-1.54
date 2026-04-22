#include "port_sdcard.h"
#include "port_i2c.h"
#include "epaper_config.h"

static I2cMasterBus *i2c_bus = NULL;
static bool sdcardinitflag = true;

char sdcard_test1[100] = { "HELLO WORLD " };
char sdcard_test2[100] = { "" };
uint32_t arg_count = 1;

void sdcard_LoopTask(void *arg) {
  if (0 == sdcardinitflag) {
    Serial.printf("sdcard Init failed\n");
  }
  while (1) {
    if (1 == sdcardinitflag) {
      snprintf(sdcard_test1, sizeof(sdcard_test1), "HELLO WORLD %ld", arg_count);
      Sdcard_WriteFile("/sdcard/sdcard.txt", sdcard_test1);
      Sdcard_ReadFile("/sdcard/sdcard.txt", sdcard_test2, NULL);
      if (!strcmp(sdcard_test1, sdcard_test2)) {
        Serial.printf("sdcard successful test ,read data : %s\n", sdcard_test2);
      } else {
        Serial.printf("sdcard failed test\n");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
    arg_count++;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  i2c_bus = I2cMasterBus::requestInstance(ESP32_I2C_SCL_PIN, ESP32_I2C_SDA_PIN, ESP32_I2C_DEV_NUM);
  assert(i2c_bus);
  sdcardinitflag = Sdcard_Init();
	xTaskCreatePinnedToCore(sdcard_LoopTask, "sdcard_LoopTask", 4 * 1024, NULL, 4, NULL,0);
}

void loop() {
}
