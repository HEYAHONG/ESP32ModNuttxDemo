#include "init.h"
#include <nuttx/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/mount.h>
#include "wireless/wapi.h"

void app_init(void)
{
    printf("Chip is ESP32!\r\n");
#if CONFIG_ESP32_SPIFLASH
#if CONFIG_FS_SPIFFS
    printf("mount /dev/esp32flash to /var/esp!\r\n");
    mount("/dev/esp32flash", "/var/esp", "spiffs", 0, NULL);
#endif
#endif

#ifdef CONFIG_ESP32_WIFI
    //WIFI设置
    {
        //获取一个套接字用于调用驱动接口
        int sock = wapi_make_socket();
        const char *sta = NULL;
        const char *ap = NULL;
#ifdef CONFIG_ESP32_WIFI_STATION
        //仅WIFI STA
        sta = "wlan0";
#endif // CONFIG_ESP32_WIFI_STATION
#ifdef CONFIG_ESP32_WIFI_SOFTAP
        //仅WIFI AP
        ap = "wlan0";
#endif // CONFIG_ESP32_WIFI_SOFTAP
#ifdef CONFIG_ESP32_WIFI_STATION_SOFTAP_COEXISTENCE
        //WIFI STA与WIFI AP共存
        sta = "wlan0";
        ap = "waln1";
#endif // CONFIG_ESP32_WIFI_STATION_SOFTAP_COEXISTENCE



        if (sta != NULL)
        {
            //打印ip信息
            struct in_addr ip = {0}, netmask = {0};
            printf("Wifi station device:%s!\r\n", sta);
            wapi_get_ip(sock, sta, &ip);
            printf("Wifi station ip:%d.%d.%d.%d!\r\n", ((ip.s_addr >> 0) & 0xFF), ((ip.s_addr >> 8) & 0xFF), ((ip.s_addr >> 16) & 0xFF), ((ip.s_addr >> 24) & 0xFF));
            wapi_get_netmask(sock, sta, &netmask);
            printf("Wifi station netmask:%d.%d.%d.%d!\r\n", ((netmask.s_addr >> 0) & 0xFF), ((netmask.s_addr >> 8) & 0xFF), ((netmask.s_addr >> 16) & 0xFF), ((netmask.s_addr >> 24) & 0xFF));
        }

        if (ap != NULL)
        {
            printf("Wifi softap device:%s!\r\n", ap);
        }

        //关闭套接字
        close(sock);
    }
#endif

}
