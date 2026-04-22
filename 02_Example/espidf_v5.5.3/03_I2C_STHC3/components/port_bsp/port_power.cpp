#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include "port_power.h"
#include "epaper_config.h"

void BoardPower_Init() {
    ESP_ERROR_CHECK(gpio_set_direction(EPD_PWR_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(Audio_PWR_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(VBAT_PWR_PIN, GPIO_MODE_OUTPUT)); 
}

void BoardPower_EPD_ON() {
    gpio_set_level(EPD_PWR_PIN,1);
}

void BoardPower_EPD_OFF() {
    gpio_set_level(EPD_PWR_PIN,0);
}

void BoardPower_Audio_ON() {
    gpio_set_level(Audio_PWR_PIN,1);
}

void BoardPower_Audio_OFF() {
    gpio_set_level(Audio_PWR_PIN,0);
}

void BoardPower_VBAT_ON() {
    gpio_set_level(VBAT_PWR_PIN,1);
}

void BoardPower_VBAT_OFF() {
    gpio_set_level(VBAT_PWR_PIN,0);
}