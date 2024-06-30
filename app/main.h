#ifndef __MAIN_H__
#define __MAIN_H__


#define MACHINE_STRING	"ASRTO900-098"
#define FIRMWARE_STRING	"V1.1.5"

#define APP_VER   MACHINE_STRING"-"FIRMWARE_STRING"@"__DATE__" "__TIME__
#define SMS_VER   MACHINE_STRING"-"FIRMWARE_STRING
#define FW_VER	  __DATE__" "__TIME__

#define __noinit__ __attribute__((section("NO_INIT"),zero_init))

extern void bsp_init(void);
extern void app_PrintVersion(void);
extern void app_GetVersion(unsigned char *buf);

#endif
