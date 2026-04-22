#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <esp_timer.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_log.h>
#include "port_power.h"
#include "port_i2c.h"
#include "port_display.h"
#include "epaper_config.h"
#include "port_lvgl.h"
#include "FacTest_ui.h"
#include "esp_io_expander_tca9554.h"
#include "esp_io_expander_gpio_wrapper.h"

static esp_io_expander_handle_t io_expander = NULL;
static I2cMasterBus *i2c_bus = NULL;
lv_factest_ui src_ui;

#define TAG "batt pwr test"

static void Lvgl_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * color_p) {
  	uint16_t *buffer = (uint16_t *)color_p;
    EPD_Clear();
  	for(int y = area->y1; y <= area->y2; y++) {
  	 	for(int x = area->x1; x <= area->x2; x++) {
  	 	   	uint8_t color = (*buffer < 0x7fff) ? DRIVER_COLOR_BLACK : DRIVER_COLOR_WHITE;
  	 	   	EPD_DrawColorPixel(x,y,color);
  	 	   	buffer++;
  	 	}
  	}
  	EPD_DisplayPart();
	lv_disp_flush_ready(disp);
}

void Lvgl_LoopTask(void *arg) {
	char buffer[50];
	uint32_t arg_vol = 1;
	while(1) {
		snprintf(buffer,sizeof(buffer),"%ld",arg_vol);
		if(Lvgl_lock(-1)) {
            lv_label_set_text(src_ui.screen_label_2, buffer);
            Lvgl_unlock();
        }
		vTaskDelay(pdMS_TO_TICKS(1500));
		arg_vol++;
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
	PortLvgl_Start_Init();
    Lvgl_PortInit(Lvgl_flush_cb);
	if(Lvgl_lock(-1)) {
        setup_factest_ui(&src_ui);
        Lvgl_unlock();
    }
	xTaskCreatePinnedToCore(Lvgl_LoopTask, "Lvgl_LoopTask", 4 * 1024, NULL, 4, NULL,0);
}
