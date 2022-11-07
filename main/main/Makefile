include $(APPDIR)/Make.defs

PROGNAME  = main
PRIORITY  = 100
STACKSIZE = CONFIG_DEFAULT_TASK_STACKSIZE
MODULE    = y


MAINSRC = main.c

CSRCS = init.c

ifeq ($(CONFIG_ARCH_CHIP_ESP32),y)

CSRCS += init_esp32.c

else

CSRCS += init_dummy.c

endif



include $(APPDIR)/Application.mk
