#ifndef PORT_SDCARD_H
#define PORT_SDCARD_H


#ifdef __cplusplus
extern "C" {
#endif



bool Sdcard_Init(void);
esp_err_t Sdcard_ReadFile(const char *path,char *pxbuf,uint32_t *outLen);
esp_err_t Sdcard_WriteFile(const char *path, char *data);




#ifdef __cplusplus
}
#endif

#endif