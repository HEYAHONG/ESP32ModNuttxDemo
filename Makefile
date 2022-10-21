
ESPTOOL_PORT ?= /dev/ttyUSB0
ESPTOOL_BAUD ?= 115200
ESPTOOL_BINDIR ?= $(shell pwd)/boot/

.PHONY:  all

all:checkconfig
	@${MAKE} -C nuttx

.PHONY:  flash
flash:checkconfig
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
	@cp ${PROJECT_PATH}/nuttx/.config  ${PROJECT_PATH}/

.PHONY:  checkconfig
checkconfig:check


.PHONY:  check
check:
	@[ -f nuttx/Makefile ] || echo 请执行bootstrap.sh
	@[ -f nuttx/Makefile ] || exit
	@[ -n "${PROJECT_PATH}" ] || echo 请执行bootstrap.sh
	@[ -n "${PROJECT_PATH}" ] || exit

