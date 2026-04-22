#include <Arduino.h>
#include <OneButton.h>
#include "port_i2c.h"
#include "epaper_config.h"
#include "src/exio/esp_io_expander_tca9554.h"
#include "src/ui/FacTest_ui.h"
#include "port_lvgl.h"
#include "port_display.h"
#include "port_codec.h"
#include "mood_30.h"

static I2cMasterBus *i2c_bus = NULL;
static esp_io_expander_handle_t io_expander = NULL;
lv_factest_ui src_ui;
OneButton boot_button;
OneButton power_button;
static EventGroupHandle_t FacTestEventGroup = NULL;  // 事件组句柄
static uint8_t *audio_data_ptr = NULL;
static bool audioflag = 1;
static bool is_music = 0;

static void Lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *color_p) {
  uint16_t *buffer = (uint16_t *)color_p;
  EPD_Clear();
  for (int y = area->y1; y <= area->y2; y++) {
    for (int x = area->x1; x <= area->x2; x++) {
      uint8_t color = (*buffer < 0x7fff) ? DRIVER_COLOR_BLACK : DRIVER_COLOR_WHITE;
      EPD_DrawColorPixel(x, y, color);
      buffer++;
    }
  }
  EPD_DisplayPart();
  lv_disp_flush_ready(disp);
}


void Audio_LoopTask(void *arg) {
  while (1) {
    EventBits_t even = xEventGroupWaitBits(FacTestEventGroup, (0x01) | (0x02), pdTRUE, pdFALSE, portMAX_DELAY);
    if (even & 0x01) {  //单击boot录音&播放
      if (Lvgl_lock(-1)) {
        lv_label_set_text(src_ui.screen_label_1, "Recording...");
        Lvgl_unlock();
      }
      Codec_RecordData(audio_data_ptr, 102400);
      if (Lvgl_lock(-1)) {
        lv_label_set_text(src_ui.screen_label_1, "Playing...");
        Lvgl_unlock();
      }
      Codec_PlaybackData(audio_data_ptr, 102400);
      if (Lvgl_lock(-1)) {
        lv_label_set_text(src_ui.screen_label_1, "Play Done");
        Lvgl_unlock();
      }
      audioflag = 1;
    }
    if (even & 0x02) {  //双击 pwr 播放音乐
      if (Lvgl_lock(-1)) {
        lv_label_set_text(src_ui.screen_label_1, "Play Music");
        Lvgl_unlock();
      }
      size_t bytes_write = 0;
      size_t bytes_max = (size_t)mood_30_pcm_len;
      uint8_t *data_ptr = (uint8_t *)mood_30_pcm;
      while ((1 == is_music) && (bytes_write < bytes_max)) {
        Codec_PlaybackData(data_ptr, 256);
        data_ptr += 256;
        bytes_write += 256;
      }
      if (Lvgl_lock(-1)) {
        lv_label_set_text(src_ui.screen_label_1, "Play Done");
        Lvgl_unlock();
      }
      audioflag = 1;
    }
  }
}


void InitializeButtons(void) {
  power_button.attachDoubleClick([]() {
    if (1 == audioflag) {
      audioflag = 0;
      is_music = 1;
      xEventGroupSetBits(FacTestEventGroup, (0x02));
    }
  });
  power_button.attachClick([]() {
    if (1 == is_music) {
      is_music = 0;
    }
  });
  boot_button.attachClick([]() {
    if (1 == audioflag) {
      audioflag = 0;
      xEventGroupSetBits(FacTestEventGroup, (0x01));
    }
  });
}

void Button_LoopTask(void *arg) {
  while (1) {
    EventBits_t even = xEventGroupWaitBits(FacTestEventGroup, (0x01), pdTRUE, pdFALSE, portMAX_DELAY);
    if (even & 0x01) {
    }
  }
}

void setup() {
  Serial.begin(115200);
  FacTestEventGroup = xEventGroupCreate();
  i2c_bus = I2cMasterBus::requestInstance(ESP32_I2C_SCL_PIN, ESP32_I2C_SDA_PIN, ESP32_I2C_DEV_NUM);
  assert(i2c_bus);
  audio_data_ptr = (uint8_t *)heap_caps_malloc(102400, MALLOC_CAP_DEFAULT);
  assert(audio_data_ptr);
  ESP_ERROR_CHECK(esp_io_expander_new_i2c_tca9554(i2c_bus->Get_I2cBusHandle(), ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_000, &io_expander));
  ESP_ERROR_CHECK(esp_io_expander_set_dir(io_expander, IO_EXPANDER_PIN_NUM_0 | IO_EXPANDER_PIN_NUM_1 | IO_EXPANDER_PIN_NUM_3, IO_EXPANDER_OUTPUT));
  ESP_ERROR_CHECK(esp_io_expander_set_level(io_expander, IO_EXPANDER_PIN_NUM_0 | IO_EXPANDER_PIN_NUM_1 | IO_EXPANDER_PIN_NUM_3, 1));
  power_button.setup(
    PWR_BUTTON_PIN,  // Input pin for the button
    INPUT_PULLUP,    // INPUT and enable the internal pull-up resistor
    true             // Button is active LOW
  );
  boot_button.setup(
    BOOT_BUTTON_PIN,  // Input pin for the button
    INPUT_PULLUP,     // INPUT and enable the internal pull-up resistor
    true              // Button is active LOW
  );
  InitializeButtons();
  PortLvgl_Start_Init();
  Lvgl_PortInit(Lvgl_flush_cb);
  if (Lvgl_lock(-1)) {
    setup_factest_ui(&src_ui);
    Lvgl_unlock();
  }
  Codec_StartInit();
  xTaskCreatePinnedToCore(Audio_LoopTask, "Audio_LoopTask", 4 * 1024, NULL, 4, NULL, 0);
}

void loop() {
  boot_button.tick();
  power_button.tick();
}
