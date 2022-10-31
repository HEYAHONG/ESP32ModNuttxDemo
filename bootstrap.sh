#!/bin/bash

#检查工具是否存在,$1为待检查的工具名。
function CheckTool
{
	[  -n "$1"  ] ||
	{
		echo -e  "\033[41;37mCheckTool 参数错误!!\033[40;37m";
		return 255;
	};
	ToolPath=`which $1`;
	[ -e "$ToolPath" ] ||
	{
		 echo -e "\033[41;37m$1 不存在，请先安装此工具\033[40;37m";
		 return 255;
	};
	return 0;
}

#检查必要的工具
CheckTool git
[ $? -eq 0 ] || exit;
CheckTool install
[ $? -eq 0 ] || exit;
CheckTool patch
[ $? -eq 0 ] || exit;
CheckTool find
[ $? -eq 0 ] || exit;
CheckTool dirname
[ $? -eq 0 ] || exit;
CheckTool readlink
[ $? -eq 0 ] || exit;
CheckTool ln
[ $? -eq 0 ] || exit;
CheckTool sed
[ $? -eq 0 ] || exit;
CheckTool python3
[ $? -eq 0 ] || exit;
CheckTool make
[ $? -eq 0 ] || exit;
CheckTool kconfig-mconf
[ $? -eq 0 ] || exit;
CheckTool grep
[ $? -eq 0 ] || exit;
#设置ROOT_PATH变量

self_path=""

# shellcheck disable=SC2128  # ignore array expansion warning
if [ -n "${BASH_SOURCE-}" ]
then
self_path="${BASH_SOURCE}"
elif [ -n "${ZSH_VERSION-}" ]
then
self_path="${(%):-%x}"
else
echo -e "\033[41;37m不能获取工作目录\033[40;37m";
return 1
fi

# shellcheck disable=SC2169,SC2169,SC2039  # unreachable with 'dash'
if [[ "$OSTYPE" == "darwin"* ]]; then
# convert possibly relative path to absolute
script_dir="$(realpath_int "${self_path}")"
# resolve any ../ references to make the path shorter
script_dir="$(cd "${script_dir}" || exit 1; pwd)"
else
# convert to full path and get the directory name of that
script_name="$(readlink -f "${self_path}")"
script_dir="$(dirname "${script_name}")"
fi

ROOT_PATH="${script_dir}";
SDK_PATH=${ROOT_PATH}/tools/esp-idf/

echo -e  "\033[44;37m当前目录:${ROOT_PATH}\033[40;37m";

python3 -m pip install -U pip || python3  ${ROOT_PATH}/tools/bin/get-pip.py
python3 -m pip install -U virtualenv

export PROJECT_PATH="${script_dir}"

echo "设置 PROJECT_PATH 为 '${PROJECT_PATH}'"

echo -e  "\033[44;37m正在初始化SDK环境\033[40;37m";
# 更换下载地址
export IDF_GITHUB_ASSETS="dl.espressif.com/github_assets"
# 检查是否从github.com下载，否则修改url
if [ -z "`git config --get --worktree remote.origin.url |grep github.com`"  ] ;
then echo "源代码不是从github.com下载,将修改Url。" ;
git config  --worktree --replace-all remote.origin.url https://github.com/HEYAHONG/ESP32ModNuttxDemo.git
fi

cd ${SDK_PATH}
function InstallFailure
{
	echo -e "初始化失败,请手动安装SDK开发环境。"
	exit;
}
git submodule update --init --force --recursive
[ $? -eq 0 ] || InstallFailure
./install.sh
[ $? -eq 0 ] || InstallFailure
source ./export.sh
cd ${ROOT_PATH}

export PATH=${ROOT_PATH}/tools/bin/:${ROOT_PATH}/tools/esptool/:${ROOT_PATH}/tools/esp-idf/components/spiffs/:$PATH

echo -e  "\033[44;37mSDK环境初始化完毕!\033[40;37m";

#安装支持补丁
echo -e "\033[44;37m安装补丁\033[40;37m";

cd $ROOT_PATH/nuttx
git clean -f
[ $? -eq 0 ] || exit
git reset --hard
[ $? -eq 0 ] || exit
cd $ROOT_PATH/patches/nuttx
for i in `find -name "*.patch"`
do
echo applying $i
patch  -p1 -d $ROOT_PATH/nuttx < $ROOT_PATH/patches/nuttx/$i;
[ $? -eq 0 ] || exit;
done
cd $ROOT_PATH

cd $ROOT_PATH/apps
git clean -f
[ $? -eq 0 ] || exit
git reset --hard
[ $? -eq 0 ] || exit
cd $ROOT_PATH/patches/apps
for i in `find -name "*.patch"`
do
echo applying $i
patch  -p1 -d $ROOT_PATH/apps < $ROOT_PATH/patches/apps/$i;
[ $? -eq 0 ] || exit;
done
cd $ROOT_PATH

echo -e  "\033[44;37mSDK补丁安装完毕!\033[40;37m";


echo -e "\033[44;37m准备目录\033[40;37m";

#if [ -f $ROOT_PATH/nuttx/.config ]
#then
#make -C $ROOT_PATH/nuttx distclean
#[ $? -eq 0 ] || exit;
#fi

ln -sf $ROOT_PATH/main $ROOT_PATH/apps/
[ $? -eq 0 ] || exit;
$ROOT_PATH/nuttx/tools/configure.sh esp32-devkitc:wifinsh
[ $? -eq 0 ] || exit;
rm -rf $ROOT_PATH/nuttx/.config
[ $? -eq 0 ] || exit;
ln -sf $ROOT_PATH/.config $ROOT_PATH/nuttx/.config
[ $? -eq 0 ] || exit;

echo -e "\033[44;37m准备目录完毕!\033[40;37m";

echo -e "\033[44;37m环境初始化完毕!请使用make命令编译!\033[40;37m";

${SHELL}
