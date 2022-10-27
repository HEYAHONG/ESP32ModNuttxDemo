
#include <nuttx/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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

    //执行nsh
    printf("now start shell!\r\n");
    system("nsh");

    //防止因exit而导致shell退出
    while (true)
    {
        printf("shell exited,now start shell!\r\n");
        system("sh");
    }
    return 0;
}
