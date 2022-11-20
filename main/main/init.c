#include "init.h"
#include "stdlib.h"
#include "stdio.h"
#include "RC.h"


void init(void)
{
    printf("Nuttx App init!\r\n");
    //打印banner
    {
        char *banner = (char *)RCGetHandle("banner");
        if (banner != NULL)
            printf("\r\n%s\r\n", banner);
    }


    //调用app初始化
    app_init();
}

