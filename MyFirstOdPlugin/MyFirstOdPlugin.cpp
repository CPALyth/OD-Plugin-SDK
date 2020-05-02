
#include "pch.h"
#include "Plugin.h"

HWND g_hOllyDbg;	// OD主界面句柄
bool isLogging = false;	// 是否加入日志

void RenameCall(t_dump* pDump);
bool Str_IsBeginWith(const char* str1, const char* str2);
void logJcc();

/* [2020/05/02 23:11]-[Remark: 必须的导出函数] */
/* [菜单中显示的插件名]-[Return:None] */
extc __declspec(dllexport) cdecl int ODBG_Plugindata(char* shortName)
{
	_Mergequicknames();
	const char* pluginName = "CPALyth";
	strcpy_s(shortName, strlen(pluginName) + 1, pluginName);
	return PLUGIN_VERSION;
}

/* [2020/05/02 23:11]-[Remark: 必须的导出函数] */
/* [插件初始化，用于判断当前OD版本号和插件所支持的版本是否一致]-[Return:None] */
extc __declspec(dllexport) cdecl int ODBG_Plugininit(int ollydbgversion, HWND hw, ulong *features)
{

	if (ollydbgversion < PLUGIN_VERSION)
	{
		MessageBox(hw, "本插件不支持当前版本OD!", "CPALyth", MB_ICONERROR);
		return -1;
	}
	g_hOllyDbg = hw;
	return 0;
}

/* [2020/05/02 23:12]-[Remark: 显示对应的菜单选项] */
/* [显示菜单项]-[Return:None] */
extc __declspec(dllexport) cdecl int  ODBG_Pluginmenu(int origin, TCHAR data[4096], VOID *item)
{

	if (origin == PM_MAIN)		// 主菜单点击
	{
		strcpy(data, "0&JCC指令记录器,1&停止记录JCC指令");
	}
	if (origin == PM_DISASM)	// 反汇编窗口点击
	{
		strcpy(data, "0&重命名该函数");
	}
	if (origin == PM_MEMORY)	// 内存窗口点击
	{
		strcpy(data, "0&重命名该函数");
	}
	return 1;
}

/* [2020/05/02 23:14]-[Remark: 所有的菜单项被点击都会执行到这个函数] */
/* [菜单项被点击执行函数]-[Return:None] */
extc __declspec(dllexport) cdecl void ODBG_Pluginaction(int origin, int action, VOID *item)
{
	// 如果是在主窗口点击
	if (origin == PM_MAIN)
	{
		if (action == 0)
		{
			char szSug[100] = { 0 };
			strcpy_s(szSug, "********** CPALyth Jcc指令记录开始 **********");
			_Addtolist(0, 0, szSug);

			logJcc();
		}
		if (action == 1)
		{
			isLogging = false;

			char szSug[100] = { 0 };
			strcpy_s(szSug, "********** CPALyth Jcc指令记录结束 **********");
			_Addtolist(0, 0, szSug);
			//MessageBoxA(g_hOllyDbg, "记录已经停止", "CPALyth", MB_ICONINFORMATION);
		}
	}
	// 如果是在反汇编窗口点击
	if (origin == PM_DISASM)
	{
		if (action == 0)
		{
			t_dump* pDump = (t_dump*)item;
			RenameCall(pDump);
		}
	}
	// 如果是在内存窗口点击
	if (origin == PM_MEMORY)
	{

	}
}

/* [2020/05/01 16:06]-[Remark: None] */
/* [CALL重命名函数]-[Return:None] */
void RenameCall(t_dump* ptDump)
{
	DWORD dwSelAddr = ptDump->sel0;
	if (dwSelAddr == 0)	return;

	uchar pBuffer[MAXCMDSIZE];
	// 读取选中的第一行的机器码
	_Readmemory(pBuffer, dwSelAddr, MAXCMDSIZE, MM_SILENT);
	// 定义一个反汇编引擎
	t_disasm td;
	// 对选中进行反汇编
	ulong lSize = _Disasm(pBuffer, 16, dwSelAddr, NULL, &td, DISASM_ALL, NULL);
	if (!Str_IsBeginWith("call", td.result))	return;		// 不是CALL就返回
	uchar bufOffset[4];
	_Readmemory(bufOffset, dwSelAddr + 1, 4, MM_SILENT);
	// 转整数
	int nOffset;
	memcpy_s(&nOffset, 4, bufOffset, 4);
	// 计算CALL的目标地址
	int callTargetAddr = dwSelAddr + nOffset + 5;
	// 读取目标地址的标签内容到szUserInput
	char szUserInput[TEXTLEN] = { 0 };
	_Findlabel(callTargetAddr, szUserInput);	
	// 弹出对话框,让用户输入新的标签
	if (_Gettext((char*)"请输入数据",szUserInput,0,NM_NONAME,0) != -1)
	{
		// 插入标签
		_Insertname(callTargetAddr, NM_LABEL, szUserInput);
	}
}

/* [2020/05/02 22:12]-[Remark: None] */
/* [断下后都会调用的回调函数]-[Return:None] */
extc int ODBG_Pausedex(int reason, int extdata, t_reg *reg, DEBUG_EVENT* debugEvent)
{
	if (!isLogging)
	{
		return 1;
	}
	if (reason == PP_SINGLESTEP)
	{
		// 得到反汇编窗口的结构体
		t_dump* pDump = (t_dump*)_Plugingetvalue(VAL_CPUDASM);
		// 获取CPU中当前选中的指令长度
		ulong selectLen = pDump->sel1 - pDump->sel0;
		// 创建一个反汇编引擎
		t_disasm td;
		uchar cmd[MAXCMDSIZE];
		// 读取CPU当前选中的十六进制
		_Readmemory(&cmd, pDump->sel0, selectLen, MM_SILENT);
		// 反汇编选中的十六进制
		_Disasm(cmd, selectLen, pDump->addr, NULL, &td, DISASM_ALL, _Plugingetvalue(VAL_MAINTHREADID));
		if (Str_IsBeginWith("j", td.result))	// 指令以j开头
		{
			if (!Str_IsBeginWith("jmp", td.result))	// 但指令不为jmp,即条件跳转指令
			{

				if (strcmp(td.opinfo[1], "跳转已实现") == 0)
				{
					_Addtolist(pDump->sel0, 0, td.opinfo[1]);
				}
				else
				{
					_Addtolist(pDump->sel0, 1, td.opinfo[1]);
				}
			}
		}
		_Go(0, 0, STEP_IN, 0, 0);
	}
	return 1;
}

/* [2020/05/02 21:30]-[Remark: 记录所有跑过的指令,记录想知道的任何信息] */
/* [记录条件跳转指令的跳转情况]-[Return:None] */
void logJcc()
{
	isLogging = true;
	_Go(0, 0, STEP_IN, 0, 0);
}

/* [2020/05/01 18:49]-[Remark: None] */
/* [判断母串str2是否以子串str1开始]-[Return:None] */
bool Str_IsBeginWith(const char* str1, const char* str2)
{
	size_t lenpre = strlen(str1);	// 子串
	size_t lenstr = strlen(str2);	// 母串
	return lenstr < lenpre ? false : strncmp(str1, str2, lenpre) == 0;
}