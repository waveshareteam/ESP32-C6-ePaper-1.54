#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include "user_app.h"
#include "port_power.h"
#include "port_i2c.h"
#include "epaper_config.h"
#include "FacTest_ui.h"
#include "pcf85063a.h"
#include "button.h"
#include "port_sdcard.h"
#include "port_lvgl.h"
#include "port_adc.h"
#include "port_shtc3.h"
#include "port_codec.h"
#include "esp_io_expander_tca9554.h"
#include "esp_io_expander_gpio_wrapper.h"
#include "port_display.h"

#define TAG "user_app"

lv_factest_ui src_ui;
static esp_io_expander_handle_t io_expander = NULL;
static I2cMasterBus *i2c_bus = NULL;
static uint8_t *audio_data_ptr = NULL;
static EventGroupHandle_t FacTestEventGroup = NULL; // 事件组句柄
static Button *boot_button = nullptr;
static Button *power_button = nullptr;
static bool sdcardinitflag = 1;
static char Lvgl_buffer[60];
static pcf85063a_dev_t pcf85063;    //  rtc句柄
static bool pcf85063initflag = 1;   //  rtc初始化成功标志位
static bool adcinitflag = 1;
static bool shtc3initflag = 1;
static bool audiouiflag = 0;
static bool whiteflag = 0;


void Led_LoopTask(void *arg);
void Lvgl_LoopTask(void *arg);
void InitializeButtons(void); /* button 初始化 */
void Button_LoopTask(void *arg);
void Audio_LoppTask(void *arg);

void UserApp_Init() {
    audio_data_ptr = (uint8_t *)heap_caps_malloc(102400, MALLOC_CAP_DEFAULT);
    assert(audio_data_ptr);
    i2c_bus = I2cMasterBus::requestInstance(ESP32_I2C_SCL_PIN,ESP32_I2C_SDA_PIN,ESP32_I2C_DEV_NUM);
    assert(i2c_bus);
    FacTestEventGroup = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_io_expander_new_i2c_tca9554(i2c_bus->Get_I2cBusHandle(), ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_000, &io_expander));
    ESP_ERROR_CHECK(esp_io_expander_gpio_wrapper_append_handler(io_expander, GPIO_NUM_MAX));
    BoardPower_Init();
    BoardPower_EPD_ON();
    BoardPower_Audio_ON();
    BoardPower_VBAT_ON();
    PortDisplay_Init();
    esp_err_t ret = pcf85063a_init(&pcf85063, i2c_bus->Get_I2cBusHandle(), PCF85063A_ADDRESS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize PCF85063A (error: %d)", ret);
        pcf85063initflag = 0;
    }
    boot_button = new Button(BOOT_BUTTON_PIN);
    power_button = new Button(PWR_BUTTON_PIN);
    while(0 == gpio_get_level(PWR_BUTTON_PIN)) { // 等待上电按钮释放
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    BoardAdc_Init();
    Shtc3_Init(i2c_bus);
    sdcardinitflag = Sdcard_Init();
    PortLvgl_Start_Init();
    InitializeButtons();
    ESP_ERROR_CHECK(gpio_set_direction(EXIO_NUM_3, GPIO_MODE_OUTPUT));
    gpio_set_level(EXIO_NUM_3,1);
    Codec_StartInit();
}

void UserUi_Init() {
    setup_factest_ui(&src_ui);
}

void UserApp_Start_Init() {
    xTaskCreatePinnedToCore(Led_LoopTask, "Led_LoopTask", 4 * 1024, NULL, 4, NULL,0);
    xTaskCreatePinnedToCore(Lvgl_LoopTask, "Lvgl_LoopTask", 5 * 1024, NULL, 2, NULL,0);
    xTaskCreatePinnedToCore(Button_LoopTask, "Button_LoopTask", 4 * 1024, NULL, 2, NULL,0);
    xTaskCreatePinnedToCore(Audio_LoppTask, "Audio_LoppTask", 4 * 1024, NULL, 2, NULL,0);
}

void Led_LoopTask(void *arg) {
    ESP_ERROR_CHECK(gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT));
    for(;;) {
        gpio_set_level(LED_PIN,0);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level(LED_PIN,1);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void Lvgl_LoopTask(void *arg) {
    uint32_t times = 0;
    uint32_t shtc3_time = 0;
    uint32_t rtc_time = 0;
    uint32_t adc_time = 0;
    float t,h;
    if(pcf85063initflag) {
        pcf85063a_datetime_t datatime = {};
        datatime.year = 2026;
        datatime.month = 1;
        datatime.day = 1;
        datatime.hour = 8;
        datatime.min = 0;
        datatime.sec = 0;
        pcf85063a_set_time_date(&pcf85063, datatime);
        if(Lvgl_lock(-1)) {
            lv_label_set_text(src_ui.screen_label_1, "08");
            lv_label_set_text(src_ui.screen_label_2, "00");
            Lvgl_unlock();
        }
    }
    if(0 == sdcardinitflag) {
        if(Lvgl_lock(-1)) {
            lv_label_set_text(src_ui.screen_label_5, "No Card");
            Lvgl_unlock();
        }
    } else {
        char sdcard_test1[20] = {"waveshare.com"};
        char sdcard_test2[20] = {""};
        Sdcard_WriteFile("/sdcard/sdcard.txt",sdcard_test1);
        Sdcard_ReadFile("/sdcard/sdcard.txt",sdcard_test2,NULL);
        if(!strcmp(sdcard_test1,sdcard_test2)) {
            lv_label_set_text(src_ui.screen_label_5, "passed");
        } else {
            lv_label_set_text(src_ui.screen_label_5, "failed");
        }
    }
    if(1 == adcinitflag) {
        snprintf(Lvgl_buffer, sizeof(Lvgl_buffer), "%d%%", Get_Batterylevel());
        if(Lvgl_lock(-1)) {
            lv_label_set_text(src_ui.screen_label_8, Lvgl_buffer);
            Lvgl_unlock();
        }
    }
    if(1 == shtc3initflag) {
        Shtc3_ReadTempHumi(&t,&h);
        if((t != -1000) && (h != -1000)) {
            snprintf(Lvgl_buffer, sizeof(Lvgl_buffer), "%d°", (int)t);
            if(Lvgl_lock(-1)) {
                lv_label_set_text(src_ui.screen_label_3, Lvgl_buffer);
                Lvgl_unlock();
            }
            snprintf(Lvgl_buffer, sizeof(Lvgl_buffer), "%d%%", (int)h);
            if(Lvgl_lock(-1)) {
                lv_label_set_text(src_ui.screen_label_4, Lvgl_buffer);
                Lvgl_unlock();
            }
        } else {
            if(Lvgl_lock(-1)) {
                lv_label_set_text(src_ui.screen_label_3, "N");
                lv_label_set_text(src_ui.screen_label_4, "N");
                Lvgl_unlock();
            }
        }
    }
    for(;;) {
        if((times - rtc_time) >= 60) {
            rtc_time = times;
            if(pcf85063initflag) {
                pcf85063a_datetime_t current_time = {};
                pcf85063a_get_time_date(&pcf85063, &current_time);
                snprintf(Lvgl_buffer, sizeof(Lvgl_buffer), "%02d", current_time.hour);
                if(Lvgl_lock(-1)) {
                    lv_label_set_text(src_ui.screen_label_1, Lvgl_buffer);
                    Lvgl_unlock();
                }
                snprintf(Lvgl_buffer, sizeof(Lvgl_buffer), "%02d", current_time.min);
                if(Lvgl_lock(-1)) {
                    lv_label_set_text(src_ui.screen_label_2, Lvgl_buffer);
                    Lvgl_unlock();
                }
            }
        }
        if((times - adc_time) >= 10) {
            adc_time = times;
            snprintf(Lvgl_buffer, sizeof(Lvgl_buffer), "%d%%", Get_Batterylevel());
            if(Lvgl_lock(-1)) {
                lv_label_set_text(src_ui.screen_label_8, Lvgl_buffer);
                Lvgl_unlock();
            }
        }
        if((times - shtc3_time) >= 10) {
            shtc3_time = times;
            Shtc3_ReadTempHumi(&t,&h);
            if((t != -1000) && (h != -1000)) {
                snprintf(Lvgl_buffer, sizeof(Lvgl_buffer), "%d°", (int)t);
                if(Lvgl_lock(-1)) {
                    lv_label_set_text(src_ui.screen_label_3, Lvgl_buffer);
                    Lvgl_unlock();
                }
                snprintf(Lvgl_buffer, sizeof(Lvgl_buffer), "%d%%", (int)h);
                if(Lvgl_lock(-1)) {
                    lv_label_set_text(src_ui.screen_label_4, Lvgl_buffer);
                    Lvgl_unlock();
                }
            } else {
                if(Lvgl_lock(-1)) {
                    lv_label_set_text(src_ui.screen_label_3, "N");
                    lv_label_set_text(src_ui.screen_label_4, "N");
                    Lvgl_unlock();
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
        times++;
    }
}

void InitializeButtons(void) {
    boot_button->OnClick([]() {
        xEventGroupSetBits(FacTestEventGroup, (0x10));
    });

    boot_button->OnLongPress([]() {
        xEventGroupSetBits(FacTestEventGroup, (0x04));
    });

    power_button->OnLongPress([]() {
        xEventGroupSetBits(FacTestEventGroup, (0x01));
    });

    power_button->OnClick([]() {
        xEventGroupSetBits(FacTestEventGroup, (0x02));
    });
}

void Audio_LoppTask(void *arg) {
    for(;;) {
        EventBits_t even = xEventGroupWaitBits(FacTestEventGroup,(0x10),pdTRUE,pdFALSE,portMAX_DELAY);
        if(even & 0x10) { //boot 单击
            if(1 == audiouiflag) {
                Codec_RecordData(audio_data_ptr,102400);
                Codec_PlaybackData(audio_data_ptr,102400);
            }
        }
    } 
}

void Button_LoopTask(void *arg) {
    for(;;) {
        EventBits_t even = xEventGroupWaitBits(FacTestEventGroup,(0x01) | (0x02) | (0x04) | (0x08),pdTRUE,pdFALSE,portMAX_DELAY);
        if(even & 0x01) {         //长按PWR关机
            if(Lvgl_lock(-1)) {
                lv_label_set_text(src_ui.screen_label_9, "OFF");
                Lvgl_unlock();
            }
            BoardPower_VBAT_OFF();
        }
        if(even & 0x02) {         //单击PWR
            if(0 == audiouiflag) {
                audiouiflag = 1;
                if(Lvgl_lock(-1)) {
                    lv_obj_remove_flag(src_ui.screen_cont_2, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(src_ui.screen_cont_1, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(src_ui.screen_cont_3, LV_OBJ_FLAG_HIDDEN);
                    Lvgl_unlock();
                }
            } else {
                audiouiflag = 0;
                if(Lvgl_lock(-1)) {
                    lv_obj_remove_flag(src_ui.screen_cont_1, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(src_ui.screen_cont_2, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(src_ui.screen_cont_3, LV_OBJ_FLAG_HIDDEN);
                    Lvgl_unlock();
                }
            }
        }
        if(even & 0x04) {         //长按BOOT
            if(0 == whiteflag) {
                whiteflag = 1;
                if(Lvgl_lock(-1)) {
                    lv_obj_remove_flag(src_ui.screen_cont_3, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(src_ui.screen_cont_1, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(src_ui.screen_cont_2, LV_OBJ_FLAG_HIDDEN);
                    Lvgl_unlock();
                }
            } else {
                whiteflag = 0;
                if(Lvgl_lock(-1)) {
                    lv_obj_remove_flag(src_ui.screen_cont_1, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(src_ui.screen_cont_2, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(src_ui.screen_cont_3, LV_OBJ_FLAG_HIDDEN);
                    Lvgl_unlock();
                }
            }
        }
    }
}