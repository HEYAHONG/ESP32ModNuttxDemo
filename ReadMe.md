# 说明

本工程为使用nuttx 进行编程的Demo。用于搭建一个基础框架用于快速开发应用。

## 源代码下载

本源代码只能通过github.com下载(或者在下载子模块前修改git远程仓库地址),其它方式下载请酌情修改.gitmodules。

由于本源代码包含第三方源代码,故直接下载可能有部分源代码缺失，需要通过以下方法解决:

- 在进行git clone 使用--recurse-submodules参数。

- 若已通过git clone下载,则在源代码目录中执行以下命令下载子模块:

  ```bash
   git submodule update --init --recursive
  ```
## SPIFFS

 spiffs是一个用于嵌入式目标上的SPI NOR flash设备的文件系统。挂载spiffs后可直接以文件的形式进行数据的存储(可读可写)。

### 预置的镜像

可预先将一些文件放入文件系统,可直接在挂载后访问(可读可写)。

目录:[spiffs](spiffs)

### 挂载

暂时并未自动挂载。设备文件为/dev/esp32flash(需要在Kconfig中启用SPI Flash(CONFIG_ESP32_SPIFLASH))。

```bash
#将/dev/esp32flash挂载至/var/esp,可在sh中执行以下指令,挂载后即可在目录访问。
mount -t spiffs /dev/esp32flash /var/esp
```

# 编译环境

本工程主要支持Linux / WSL。注意:由于nuttx编译过程中需要下载大量数据,请保持网络通畅(尤其是[github.com](https://github.com))。

本工程中采用esp-idf的安装脚本安装工具链,nuttx采用esp-idf的工具链编译,但不使用esp-idf的idf.py而是make。

## 编译环境安装

参考 [esp-idf](https://docs.espressif.com/projects/esp-idf/) 与 [nuttx](https://nuttx.apache.org/) 的说明。

对于ubuntu 22.04 可采用以下指令安装(如缺少其它命令，请根据提示操作):

```bash
sudo apt-get install build-essential make cmake python3-pip python3-venv curl wget kconfig-frontends git patch findutils sed
```

# 编译

编译之前需要执行工程目录下的bootstrap.sh(同一个终端只需要执行一次)。

## 常用操作

### 构建

```bash
make
```

### 烧录

```bash
#直接烧录
make flash
#使用ESPTOOL_PORT指定烧录端口，不指定默认为/dev/ttyUSB0。 
#使用ESPTOOL_BAUD指定烧录端口，不指定默认为115200。
make flash ESPTOOL_PORT=串口设备 ESPTOOL_BAUD=波特率
```

### Kconfig配置

```bash
make menuconfig
```

### 清理

```bash
make clean
```

### 清理所有

此清理除了清理生成的.o文件,还会清理下载的文件。对于处理莫名奇妙出现的编译错误非常有用，清理完成后需要在新的终端执行bootstrap.sh。

```bash
make distclean
```

若此命令仍然不能解决问题（如Kconfig有误导致路径错误）,请直接删除nuttx/.config并关闭终端，然后在新的终端执行bootstrap.sh。
