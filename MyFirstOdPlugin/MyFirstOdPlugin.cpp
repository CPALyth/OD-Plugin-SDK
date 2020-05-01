
#include "pch.h"
#include "Plugin.h"

//************************************
// Method:菜单中显示的插件名
// Description: 必须的导出函数
//************************************
extern "C" __declspec(dllexport) cdecl int ODBG_Plugindata(char* shortName)
{
	_Mergequicknames();
	const char* pluginName = "FuncNameLabel";
	strcpy_s(shortName, strlen(pluginName) + 1, pluginName);
	return PLUGIN_VERSION;
}

//OD主界面句柄
HWND g_hOllyDbg;
//************************************
// Method:插件初始化，用于判断当前OD版本号和插件所支持的版本是否一致
// Description:必须的导出函数 
//************************************
extern "C" __declspec(dllexport) cdecl int ODBG_Plugininit(int ollydbgversion, HWND hw, ulong *features)
{

	if (ollydbgversion < PLUGIN_VERSION)
	{
		MessageBox(hw, "本插件不支持当前版本OD!", "MyFirstPlugin", MB_ICONERROR);
		return -1;
	}
	g_hOllyDbg = hw;
	return 0;
}

//************************************
// Method:显示菜单项
// Description:显示对应的菜单选项
//************************************
extern "C" __declspec(dllexport) cdecl int  ODBG_Pluginmenu(int origin, TCHAR data[4096], VOID *item)
{

	if (origin == PM_MAIN)		// 主菜单点击
	{
		strcpy(data, "0&顶部菜单子菜单一,1&顶部菜单子菜单二");
	}
	if (origin == PM_DISASM)
	{
		strcpy(data, "鼠标右键主菜单{0&鼠标右键子菜单一{3&三级菜单1,4&三级菜单2},1&鼠标右键子菜单二}");
	}
	return 1;
}

//************************************
// Method:菜单项被点击执行函数
// Description:所有的菜单项被点击都会执行到这个函数
//************************************
extern "C" __declspec(dllexport) cdecl void ODBG_Pluginaction(int origin, int action, VOID *item)
{
	//如果是在主窗口点击
	if (origin == PM_MAIN)
	{
		if (action == 0)
		{
			MessageBoxA(g_hOllyDbg, "顶部菜单子菜单一", "www.bcdaren.com", MB_ICONINFORMATION);
		}
		if (action == 1)
		{
			MessageBoxA(g_hOllyDbg, "顶部菜单子菜单二", "www.bcdaren.com", MB_ICONINFORMATION);
		}
	}
	//如果是在反汇编窗口点击
	if (origin == PM_DISASM)
	{
		if (action == 0)
		{
			MessageBoxA(g_hOllyDbg, "鼠标右键子菜单一", "www.bcdaren.com", MB_ICONINFORMATION);
		}
		if (action == 1)
		{
			MessageBoxA(g_hOllyDbg, "鼠标右键子菜单二", "www.bcdaren.com", MB_ICONINFORMATION);
		}
	}
}