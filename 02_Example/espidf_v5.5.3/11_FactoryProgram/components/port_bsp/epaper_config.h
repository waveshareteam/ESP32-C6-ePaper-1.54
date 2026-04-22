#ifndef EPAPER_CONFIG_H
#define EPAPER_CONFIG_H

#define EPD_SPI_NUM        SPI2_HOST
#define ESP32_I2C_DEV_NUM  I2C_NUM_0

#define EPD_WIDTH  200
#define EPD_HEIGHT 200

/*EPD port Init*/
#define EPD_DC_PIN      GPIO_NUM_15
#define EPD_CS_PIN      GPIO_NUM_7
#define EPD_SCK_PIN     GPIO_NUM_6
#define EPD_MOSI_PIN    GPIO_NUM_5
#define EPD_RST_PIN     GPIO_NUM_11
#define EPD_BUSY_PIN    GPIO_NUM_10
#define EPD_MISO_PIN    GPIO_NUM_4
#define EPD_TP_RST_PIN  GPIO_NUM_NC
#define EPD_TP_INT_PIN  GPIO_NUM_NC

/*EXIO*/
#define EXIO_NUM_0      (gpio_num_t)(GPIO_NUM_MAX)
#define EXIO_NUM_1      (gpio_num_t)(GPIO_NUM_MAX + 1)
#define EXIO_NUM_2      (gpio_num_t)(GPIO_NUM_MAX + 2)
#define EXIO_NUM_3      (gpio_num_t)(GPIO_NUM_MAX + 3)
#define EXIO_NUM_4      (gpio_num_t)(GPIO_NUM_MAX + 4)
#define EXIO_NUM_5      (gpio_num_t)(GPIO_NUM_MAX + 5)
#define EXIO_NUM_6      (gpio_num_t)(GPIO_NUM_MAX + 6)
#define EXIO_NUM_7      (gpio_num_t)(GPIO_NUM_MAX + 7)

/*DEV POWER init*/
#define EPD_PWR_PIN     EXIO_NUM_0
#define Audio_PWR_PIN   EXIO_NUM_1
#define VBAT_PWR_PIN    EXIO_NUM_5

#define BOOT_BUTTON_PIN GPIO_NUM_9
#define PWR_BUTTON_PIN  GPIO_NUM_2

#define LED_PIN         EXIO_NUM_4


/*ESP32 I2C Init*/
#define ESP32_I2C_SDA_PIN GPIO_NUM_18
#define ESP32_I2C_SCL_PIN GPIO_NUM_8

/*sdcard*/
#define SD_MISO_D0_PIN      GPIO_NUM_4
#define SD_MOSI_CMD_PIN     GPIO_NUM_5
#define SD_CLK_PIN          GPIO_NUM_6
#define SD_CS_PIN           GPIO_NUM_3

#define SCREEN_INIT_INHIBIT_SD_SPI_INIT 0
#define SDlist "/sdcard" 

/*i2c dev*/
#define I2C_RTC_DEV_Address        0x51
#define I2C_SHTC3_DEV_Address      0x70
#define I2C_FT6336_DEV_Address     0x38         

#endif // !USER_CONFIG_H