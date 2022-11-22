
#include <nuttx/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "init.h"
#if CONFIG_APP_MAIN_MQTTCLIENT  == 1
#include "appmqtt.h"
#endif
static bool isinit = false;
int main(int argc, FAR char *argv[])
{
    {
        //检查是否启动过main
        if (isinit)
        {
            printf("main is already running!\r\n");
            return 0;
        }
        else
        {
            isinit = true;
        }

    }


    //执行nsh（直接退出）
    system("nsh -c exit");

    //执行初始化
    init();

#if CONFIG_APP_MAIN_MQTTCLIENT == 1
    MQTT_Init();
#endif

    //打开shell
    while (true)
    {
        system("sh");
    }
    return 0;
}
