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
CheckTool astyle
[ $? -eq 0 ] || exit;


astyle --style=allman --indent=spaces=4 --pad-oper --pad-header --unpad-paren --suffix=none --align-pointer=name  --convert-tabs  --recursive --ascii --verbose main/*.cpp
astyle --style=allman --indent=spaces=4 --pad-oper --pad-header --unpad-paren --suffix=none --align-pointer=name  --convert-tabs  --recursive --ascii --verbose main/*.hpp
astyle --style=allman --indent=spaces=4 --pad-oper --pad-header --unpad-paren --suffix=none --align-pointer=name  --convert-tabs  --recursive --ascii --verbose main/*.h
astyle --style=allman --indent=spaces=4 --pad-oper --pad-header --unpad-paren --suffix=none --align-pointer=name  --convert-tabs  --recursive --ascii --verbose main/*.c
