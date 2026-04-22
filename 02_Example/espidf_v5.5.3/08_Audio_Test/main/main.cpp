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
#include "button.h"
#include "port_lvgl.h"
#include "FacTest_ui.h"
#include "esp_io_expander_tca9554.h"
#include "esp_io_expander_gpio_wrapper.h"
#include "port_codec.h"
#include "mood_30.h"

static esp_io_expander_handle_t io_expander = NULL;
static I2cMasterBus *i2c_bus = NULL;
static Button *boot_button = nullptr;
static Button *power_button = nullptr;
lv_factest_ui src_ui;
static EventGroupHandle_t FacTestEventGroup = NULL; // 事件组句柄
static uint8_t *audio_data_ptr = NULL;
static bool audioflag = 1;
static bool is_music = 0;

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

void Audio_LoopTask(void *arg) {
	while(1) {
		EventBits_t even = xEventGroupWaitBits(FacTestEventGroup,(0x01) | (0x02),pdTRUE,pdFALSE,portMAX_DELAY);
        if(even & 0x01) {         //单击boot录音&播放
            if(Lvgl_lock(-1)) {
				lv_label_set_text(src_ui.screen_label_1, "Recording...");
                Lvgl_unlock();
            }
			Codec_RecordData(audio_data_ptr,102400);
			if(Lvgl_lock(-1)) {
				lv_label_set_text(src_ui.screen_label_1, "Playing...");
                Lvgl_unlock();
            }
            Codec_PlaybackData(audio_data_ptr,102400);
			if(Lvgl_lock(-1)) {
				lv_label_set_text(src_ui.screen_label_1, "Play Done");
                Lvgl_unlock();
            }
			audioflag = 1;
        }
		if(even & 0x02) {         //双击 pwr 播放音乐
            if(Lvgl_lock(-1)) {
				lv_label_set_text(src_ui.screen_label_1, "Play Music");
                Lvgl_unlock();
            }
			size_t bytes_write = 0;
			size_t bytes_max = (size_t)mood_30_pcm_len;
			uint8_t *data_ptr = (uint8_t *)mood_30_pcm;
			while((1 == is_music) && (bytes_write < bytes_max)) {
				Codec_PlaybackData(data_ptr,256);
				data_ptr += 256;
				bytes_write += 256;
			}
			if(Lvgl_lock(-1)) {
				lv_label_set_text(src_ui.screen_label_1, "Play Done");
                Lvgl_unlock();
            }
			audioflag = 1;
		}
	}
}

void InitializeButtons(void) {
    power_button->OnDoubleClick([]() {
        if(1 == audioflag) {
			audioflag = 0;
			is_music = 1;
			xEventGroupSetBits(FacTestEventGroup, (0x02));
		}
    });
	power_button->OnClick([]() {
        if(1 == is_music) {
			is_music = 0;
		}
    });
	boot_button->OnClick([]() {
		if(1 == audioflag) {
			audioflag = 0;
			xEventGroupSetBits(FacTestEventGroup, (0x01));
		}
	});
}

extern "C" void app_main(void) {
	audio_data_ptr = (uint8_t *)heap_caps_malloc(102400, MALLOC_CAP_DEFAULT);
    assert(audio_data_ptr);
	FacTestEventGroup = xEventGroupCreate();
   	i2c_bus = I2cMasterBus::requestInstance(ESP32_I2C_SCL_PIN,ESP32_I2C_SDA_PIN,ESP32_I2C_DEV_NUM);
    assert(i2c_bus);
	ESP_ERROR_CHECK(esp_io_expander_new_i2c_tca9554(i2c_bus->Get_I2cBusHandle(), ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_000, &io_expander));
    ESP_ERROR_CHECK(esp_io_expander_gpio_wrapper_append_handler(io_expander, GPIO_NUM_MAX));
	BoardPower_Init();
   	BoardPower_EPD_ON();
    BoardPower_Audio_ON();
    BoardPower_VBAT_ON();
	boot_button = new Button(BOOT_BUTTON_PIN);
    power_button = new Button(PWR_BUTTON_PIN);
	while(0 == gpio_get_level(PWR_BUTTON_PIN)) { // 等待上电按钮释放
        vTaskDelay(pdMS_TO_TICKS(100));
    }
	PortLvgl_Start_Init();
	InitializeButtons();
    Lvgl_PortInit(Lvgl_flush_cb);
	if(Lvgl_lock(-1)) {
        setup_factest_ui(&src_ui);
        Lvgl_unlock();
    }
	Codec_StartInit();
	ESP_ERROR_CHECK(gpio_set_direction(EXIO_NUM_3, GPIO_MODE_OUTPUT));
    gpio_set_level(EXIO_NUM_3,1);
	xTaskCreatePinnedToCore(Audio_LoopTask, "Audio_LoopTask", 4 * 1024, NULL, 4, NULL,0);
}
