#ifndef __INIT_H__
#define __INIT_H__
#include <nuttx/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef __WEAK
#define __WEAK __attribute__((weak))
#endif

#ifndef __WEAKREF
#define __WEAKREF(x) __attribute__((weakref(x)))
#endif

/*
初始化。由主应用调用。
*/
void init(void);

/*
app初始化,根据不同的芯片移植不同的初始化过程。
*/
void app_init(void);


#ifdef __cplusplus
}
#endif

#endif
