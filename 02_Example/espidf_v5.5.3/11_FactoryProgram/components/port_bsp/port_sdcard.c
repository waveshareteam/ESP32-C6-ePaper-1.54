#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <driver/sdmmc_host.h>
#include <sdmmc_cmd.h>
#include <esp_vfs_fat.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include "port_sdcard.h"
#include "epaper_config.h"

static const char *TAG = "sdcard";

sdmmc_card_t *card = NULL;  
static bool sdcard_slot = 0;


bool Sdcard_Init(void)
{
	if(sdcard_slot)
	return 1;
  	esp_vfs_fat_sdmmc_mount_config_t mount_config = {};
    mount_config.format_if_mount_failed = false;     //If the hook fails, create a partition table and format the SD car
    mount_config.max_files = 5;                      //Maximum number of open files
    mount_config.allocation_unit_size = 512;    	 //Similar to sector size
#if (SCREEN_INIT_INHIBIT_SD_SPI_INIT == 1)
    spi_bus_config_t bus_cfg = {};
    bus_cfg.mosi_io_num = SD_MOSI_CMD_PIN;
    bus_cfg.miso_io_num = SD_MISO_D0_PIN;
    bus_cfg.sclk_io_num = SD_CLK_PIN;
    bus_cfg.quadwp_io_num = -1;
    bus_cfg.quadhd_io_num = -1;
    bus_cfg.max_transfer_sz = 4000;
    ESP_ERROR_CHECK_WITHOUT_ABORT(spi_bus_initialize(EPD_SPI_NUM, &bus_cfg, SDSPI_DEFAULT_DMA));
#endif 
  	sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SD_CS_PIN;
    slot_config.host_id = EPD_SPI_NUM;
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = EPD_SPI_NUM;

  	ESP_ERROR_CHECK_WITHOUT_ABORT(esp_vfs_fat_sdspi_mount(SDlist, &host, &slot_config, &mount_config, &card));

  	if(card != NULL) {
  	  	sdmmc_card_print_info(stdout, card);
		sdcard_slot = 1;
		return 1;
	}
	return 0;
}

float sd_card_get_value(void) {
  	if(card != NULL) {
  	  	return (float)(card->csd.capacity)/2048/1024; 
  	}
  	return -1;
}

esp_err_t Sdcard_WriteFile(const char *path, char *data)
{
  	esp_err_t err;
  	if(card == NULL)
  	{
  	  	return ESP_ERR_NOT_FOUND;
  	}
  	err = sdmmc_get_status(card); //First check if there is an SD card
  	if(err != ESP_OK)
  	{
  	  	return err;
  	}
  	FILE *f = fopen(path, "w"); //Get path address
  	if(f == NULL)
  	{
  	  	ESP_LOGI(TAG,"path:Write Wrong path");
  	  	return ESP_ERR_NOT_FOUND;
  	}
  	fprintf(f, data); //write in
  	fclose(f);
  	return ESP_OK;
}

esp_err_t Sdcard_ReadFile(const char *path,char *pxbuf,uint32_t *outLen)
{
  	esp_err_t err;
  	if(card == NULL)
  	{
  	  	ESP_LOGI(TAG,"card == NULL");
  	  	return ESP_ERR_NOT_FOUND;
  	}
  	err = sdmmc_get_status(card); //First check if there is an SD card
  	if(err != ESP_OK)
  	{
  	  	ESP_LOGI(TAG,"card == NO");
  	  	return err;
  	}
  	FILE *f = fopen(path, "rb");
  	if (f == NULL)
  	{
  	  	ESP_LOGI(TAG,"Read Wrong path");
  	  	return ESP_ERR_NOT_FOUND;
  	}
  	fseek(f, 0, SEEK_END);     //Move the pointer to the back
  	uint32_t unlen = ftell(f);
  	//fgets(pxbuf, unlen, f);  //Read text
  	fseek(f, 0, SEEK_SET);     //Move the pointer to the front
  	uint32_t poutLen = fread((void *)pxbuf,1,unlen,f);
  	if(outLen != NULL)
  	*outLen = poutLen;
  	fclose(f);
  	return ESP_OK;
}
