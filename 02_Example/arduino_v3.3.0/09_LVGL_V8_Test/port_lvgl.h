#ifndef PORT_LVGL_H
#define PORT_LVGL_H

#include "lvgl.h"

#define LVGL_TICK_PERIOD_MS    5
#define LVGL_TASK_MAX_DELAY_MS 500
#define LVGL_TASK_MIN_DELAY_MS 50


#ifdef __cplusplus
extern "C" {
#endif


typedef void (*DispFlushCb)(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);

void Lvgl_PortInit(DispFlushCb flush_cb);
bool Lvgl_lock(int timeout_ms);
void Lvgl_unlock(void);


#ifdef __cplusplus
}
#endif


#endif