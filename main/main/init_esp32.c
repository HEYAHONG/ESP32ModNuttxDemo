#include "init.h"
#include <nuttx/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/mount.h>

void app_init(void)
{
    printf("Chip is ESP32!\r\n");
#if CONFIG_ESP32_SPIFLASH
#if CONFIG_FS_SPIFFS
    printf("mount /dev/esp32flash to /var/esp!\r\n");
    mount("/dev/esp32flash", "/var/esp", "spiffs", 0, NULL);
#endif
#endif
}
