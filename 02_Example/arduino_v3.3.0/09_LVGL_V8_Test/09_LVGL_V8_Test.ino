#include <Arduino.h>
#include "epaper_config.h"
#include "src/ui/FacTest_ui.h"
#include "port_lvgl.h"
#include "port_display.h"

lv_factest_ui src_ui;

static void Lvgl_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
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

void Lvgl_LoopTask(void *arg) {
  char buffer[50];
  uint32_t arg_vol = 1;
  while (1) {
    snprintf(buffer, sizeof(buffer), "%ld", arg_vol);
    if (Lvgl_lock(-1)) {
      lv_label_set_text(src_ui.screen_label_2, buffer);
      Lvgl_unlock();
    }
    vTaskDelay(pdMS_TO_TICKS(1500));
    arg_vol++;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  PortLvgl_Start_Init();
  Lvgl_PortInit(Lvgl_flush_cb);
  if (Lvgl_lock(-1)) {
    setup_factest_ui(&src_ui);
    Lvgl_unlock();
  }
  xTaskCreatePinnedToCore(Lvgl_LoopTask, "Lvgl_LoopTask", 4 * 1024, NULL, 4, NULL, 0);
}

void loop() {
}
