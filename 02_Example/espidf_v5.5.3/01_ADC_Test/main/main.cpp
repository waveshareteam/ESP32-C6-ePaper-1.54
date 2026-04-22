#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <esp_timer.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_log.h>
#include "port_adc.h"
#include "port_power.h"
#include "port_i2c.h"
#include "epaper_config.h"
#include "esp_io_expander_tca9554.h"
#include "esp_io_expander_gpio_wrapper.h"

static esp_io_expander_handle_t io_expander = NULL;
static I2cMasterBus *i2c_bus = NULL;
#define TAG "adc test"

void Adc_LoopTask(void *arg) {
	while(1) {
		ESP_LOGW(TAG,"vol:%dmV,Capacity:%d%%",(uint16_t)(Get_VbatVoltage() * 1000),Get_Batterylevel());
		vTaskDelay(pdMS_TO_TICKS(1000));
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
	BoardAdc_Init();
	xTaskCreatePinnedToCore(Adc_LoopTask, "Adc_LoopTask", 4 * 1024, NULL, 4, NULL,0);
}
