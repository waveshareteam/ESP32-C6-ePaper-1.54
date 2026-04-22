#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <esp_timer.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_log.h>
#include "port_power.h"
#include "port_i2c.h"
#include "epaper_config.h"
#include "port_sdcard.h"
#include "esp_io_expander_tca9554.h"
#include "esp_io_expander_gpio_wrapper.h"

static esp_io_expander_handle_t io_expander = NULL;
static I2cMasterBus *i2c_bus = NULL;
static bool sdcardinitflag = true;
#define TAG "sdcard test"

char sdcard_test1[100] = {"HELLO WORLD "};
char sdcard_test2[100] = {""};
uint32_t arg_count = 1;

void sdcard_LoopTask(void *arg) {
    if(0 == sdcardinitflag) {
        ESP_LOGE(TAG,"sdcard Init failed");
    }
	while(1) {
		if(1 == sdcardinitflag) {
            snprintf(sdcard_test1,sizeof(sdcard_test1),"HELLO WORLD %ld",arg_count);
            Sdcard_WriteFile("/sdcard/sdcard.txt",sdcard_test1);
            Sdcard_ReadFile("/sdcard/sdcard.txt",sdcard_test2,NULL);
            if(!strcmp(sdcard_test1,sdcard_test2)) {
                ESP_LOGW(TAG,"sdcard successful test ,read data : %s",sdcard_test2);
            } else {
                ESP_LOGE(TAG,"sdcard failed test");
            }
        }
		vTaskDelay(pdMS_TO_TICKS(1000));
        arg_count++;
	}
}

extern "C" void app_main(void) {
   	i2c_bus = I2cMasterBus::requestInstance(ESP32_I2C_SCL_PIN,ESP32_I2C_SDA_PIN,ESP32_I2C_DEV_NUM);
    assert(i2c_bus);
	ESP_ERROR_CHECK(esp_io_expander_new_i2c_tca9554(i2c_bus->Get_I2cBusHandle(), ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_000, &io_expander));
    ESP_ERROR_CHECK(esp_io_expander_gpio_wrapper_append_handler(io_expander, GPIO_NUM_MAX));
	BoardPower_Init();
   	BoardPower_EPD_ON();
    BoardPower_Audio_ON();
    BoardPower_VBAT_ON();
	sdcardinitflag = Sdcard_Init();
	xTaskCreatePinnedToCore(sdcard_LoopTask, "sdcard_LoopTask", 4 * 1024, NULL, 4, NULL,0);
}
