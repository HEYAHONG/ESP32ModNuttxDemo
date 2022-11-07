
include .config

ESPTOOL_PORT ?= /dev/ttyUSB0
ESPTOOL_BAUD ?= 115200
ESPTOOL_BINDIR ?= $(shell pwd)/boot/

ifeq ($(CONFIG_ESP32_SPIFLASH),y)
SPIFFSGEN ?= spiffsgen.py
SPIFFSROOT ?= $(shell pwd)/spiffs
SPIFFSBIN ?= $(shell pwd)/spiffs.bin
SPIFFSOBJNAMELEN ?=  $(shell expr $(CONFIG_SPIFFS_NAME_MAX) + 1 )
SPIFFSGENFLAGS := --follow-symlinks --meta-len 0  --obj-name-len ${SPIFFSOBJNAMELEN} ${CONFIG_ESP32_STORAGE_MTD_SIZE} "${SPIFFSROOT}"  "${SPIFFSBIN}"
endif

.PHONY:  all
all:checkconfig
	@${MAKE} -C nuttx

.PHONY:  monitor
monitor:checkconfig
	@idf_monitor.py -p ${ESPTOOL_PORT}  -b ${ESPTOOL_BAUD}

.PHONY:  putty
putty : checkconfig
ifeq (${DISPLAY},)
	@echo 当前不处于桌面环境，不可使用putty.
	@exit 2
endif
ifeq ($(shell which putty),)
	@echo 未安装putty，请安装putty.
	@exit 2
endif
	@putty -serial -sercfg ${ESPTOOL_BAUD},8,n,1 ${ESPTOOL_PORT} 

.PHONY:  flash
flash:checkconfig
ifeq ($(CONFIG_ESP32_SPIFLASH),y)
#生成spiffs.bin
	${SPIFFSGEN} ${SPIFFSGENFLAGS}
#烧录spiffs.bin
	@esptool.py -c esp32 -p ${ESPTOOL_PORT}  -b ${ESPTOOL_BAUD}  write_flash -fs detect ${CONFIG_ESP32_STORAGE_MTD_OFFSET} ${SPIFFSBIN}
endif
#执行烧录
	@${MAKE} -C nuttx flash ESPTOOL_PORT=${ESPTOOL_PORT} ESPTOOL_BAUD=${ESPTOOL_BAUD} ESPTOOL_BINDIR=${ESPTOOL_BINDIR}

.PHONY:  clean
clean:checkconfig
	@${MAKE} -C nuttx clean

.PHONY:  distclean
distclean:checkconfig
	@${MAKE} -C nuttx distclean
	@echo  "请退出当前终端并在新打开的终端中执行bootstrap.sh"

.PHONY:  menuconfig
menuconfig:checkconfig
	@${MAKE} -C nuttx menuconfig
	@cp -rf ${PROJECT_PATH}/nuttx/.config  ${PROJECT_PATH}/

.PHONY:  checkconfig
checkconfig:check


.PHONY:  check
check:
	@[ -f nuttx/Makefile ] || echo 请执行bootstrap.sh
	@[ -f nuttx/Makefile ] || exit
	@[ -n "${PROJECT_PATH}" ] || echo 请执行bootstrap.sh
	@[ -n "${PROJECT_PATH}" ] || exit

