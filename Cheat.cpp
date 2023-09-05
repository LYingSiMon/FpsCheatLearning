#include "Cheat.h"
#include "Common.h"
#include "spdlog/spdlog.h"

#define PLAYER_OFFSET 0x2E4FDC + 0x10 * Index

HANDLE g_hProc = 0;
DWORD g_Pid = 0;
HWND g_ImguiHwnd;
HWND g_GameHwnd = 0;
WNDPROC g_GameWndProc = 0;
BOOL g_OpenOption = FALSE;
ULONG_PTR g_DllBase_ServerCss = 0, g_DllBase_Client = 0, g_DllBase_Engine = 0;
PlayerInfo g_PlayerInfo[20] = { 0 };
DWORD g_PlayerCount = 0;
FLOAT g_Matrix[4][4] = { 0 };

BOOL GetGameWnd()
{
	g_GameHwnd = ::FindWindowA("Valve001", "Counter-Strike Source");
	if (!g_GameHwnd)
	{
		spdlog::error("FindWindowA error:{}", GetLastError());
		return FALSE;
	}

	return TRUE;
}

BOOL GetGamePid()
{
	GetWindowThreadProcessId(g_GameHwnd, &g_Pid);
	if (!g_Pid)
	{
		spdlog::error("GetWindowThreadProcessId error:{}", GetLastError());
		return FALSE;
	}

	return TRUE;
}

BOOL GetGameProcHandle()
{
	g_hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, g_Pid);
	if (!g_hProc)
	{
		spdlog::error("OpenProcess error:{}", GetLastError());
		return FALSE;
	}
	
	return TRUE;
}

BOOL GetGameModuleBase()
{
	g_DllBase_ServerCss = GetModuleBaseByPid(g_Pid, "server_css.dll");
	if (!g_DllBase_ServerCss)
	{
		spdlog::error("server_css.dll GetModuleBaseByPid error");
		return FALSE;
	}
	spdlog::info("server_css.dll BaseAddress:{0:x}", g_DllBase_ServerCss);

	g_DllBase_Client = GetModuleBaseByPid(g_Pid, "client.dll");
	if (!g_DllBase_Client)
	{
		spdlog::error("client.dll GetModuleBaseByPid error");
		return FALSE;
	}
	spdlog::info("client.dll BaseAddress:{0:x}", g_DllBase_Client);

	g_DllBase_Engine = GetModuleBaseByPid(g_Pid, "engine.dll");
	if (!g_DllBase_Engine)
	{
		spdlog::error("engine.dll GetModuleBaseByPid error");
		return FALSE;
	}
	spdlog::info("engine.dll BaseAddress:{0:x}", g_DllBase_Engine);

	return TRUE;
}

BOOL GetPlayerNameById(DWORD Id, PCHAR Name, DWORD Size)
{
	ULONG_PTR NameBase = g_DllBase_Engine + 0x32735C;
	ULONG_PTR Temp = 0;
	BYTE Buffer[256] = { 0 };

	memset(Buffer, 0, 256);
	if (!ReadProcessMemory(g_hProc, (PVOID)NameBase, Buffer, sizeof(ULONG_PTR), 0))
	{
		return FALSE;
	}
	memcpy(&Temp, Buffer, sizeof(ULONG_PTR));

	memset(Buffer, 0, 256);
	if (!ReadProcessMemory(g_hProc, (PVOID)(Temp + 0x38), Buffer, sizeof(ULONG_PTR), 0))
	{
		return FALSE;
	}
	memcpy(&Temp, Buffer, sizeof(ULONG_PTR));

	memset(Buffer, 0, 256);
	Id = Id + Id * 4;
	if (!ReadProcessMemory(g_hProc, (PVOID)(Temp + Id * 8 + 0x14), Buffer, sizeof(ULONG_PTR), 0))
	{
		return FALSE;
	}
	memcpy(&Temp, Buffer, sizeof(ULONG_PTR));

	memset(Buffer, 0, 256);
	if (!ReadProcessMemory(g_hProc, (PVOID)Temp, Buffer, Size, 0))
	{
		return FALSE;
	}
	memcpy(Name, Buffer, Size);


	return TRUE;
}

BOOL GameXYZToScreenX1Y1Y2(FLOAT mcax, FLOAT mcay, FLOAT mcaz, FLOAT to[3], DWORD Index)
{
	RECT GameWndRect;
	::GetWindowRect(g_GameHwnd, &GameWndRect);

	FLOAT WndWidth = (FLOAT)GameWndRect.right - (FLOAT)GameWndRect.left;
	FLOAT WndHeight = (FLOAT)GameWndRect.bottom - (FLOAT)GameWndRect.top;
	FLOAT x1, y1, y2, w;

	x1 = g_Matrix[0][0] * (mcax)+g_Matrix[0][1] * mcay + g_Matrix[0][2] * mcaz + g_Matrix[0][3];
	y1 = g_Matrix[1][0] * mcax + g_Matrix[1][1] * mcay + g_Matrix[1][2] * mcaz + g_Matrix[1][3];
	y2 = g_Matrix[1][0] * mcax + g_Matrix[1][1] * mcay + g_Matrix[1][2] * (mcaz + 60) + g_Matrix[1][3];
	w = g_Matrix[3][0] * mcax + g_Matrix[3][1] * mcay + g_Matrix[3][2] * mcaz + g_Matrix[3][3];

	if (w < 0.01f)
		return FALSE;

	x1 = x1 / w;
	y1 = y1 / w;
	y2 = y2 / w;

	to[0] = (FLOAT)(WndWidth / 2.0 * x1) + (FLOAT)(x1 + WndWidth / 2.0);
	to[1] = (FLOAT)-(WndHeight / 2.0 * y1) + (FLOAT)(y1 + WndHeight / 2.0);
	to[2] = (FLOAT)-(WndHeight / 2.0 * y2) + (FLOAT)(y2 + WndHeight / 2.0);

	if (Index != -1)
	{
		g_PlayerInfo[Index].ScreenX1 = to[0];
		g_PlayerInfo[Index].ScreenY1 = to[1];
		g_PlayerInfo[Index].ScreenY2 = to[2];
	}

	return TRUE;
}

VOID GetGameMatrix()
{
	ULONG_PTR Offset_Matrix[] = { g_DllBase_Engine + 0x48BC9C };
	FLOAT Matrix[4][4] = { 0 };
	ReadProcMemByOffset(g_hProc, Offset_Matrix, 0, &g_Matrix, sizeof(g_Matrix));
	g_Matrix[0][2] = 0;
}

VOID GetPlayerCount()
{
	ULONG_PTR Offset_PlayerCount[] = { g_DllBase_ServerCss + 0x3CD4E8 };
	ReadProcMemByOffset(g_hProc, Offset_PlayerCount, 0, &g_PlayerCount, sizeof(DWORD));
}

VOID GetPlayerXYZ(DWORD Index)
{
	ULONG_PTR
		Offset_X[] = { g_DllBase_Client + PLAYER_OFFSET ,0x1F0 }, \
		Offset_Y[] = { g_DllBase_Client + PLAYER_OFFSET ,0x1F4 }, \
		Offset_Z[] = { g_DllBase_Client + PLAYER_OFFSET ,0x1F8 };

	ReadProcMemByOffset(g_hProc, Offset_X, 1, &g_PlayerInfo[Index].X, sizeof(FLOAT));
	ReadProcMemByOffset(g_hProc, Offset_Y, 1, &g_PlayerInfo[Index].Y, sizeof(FLOAT));
	ReadProcMemByOffset(g_hProc, Offset_Z, 1, &g_PlayerInfo[Index].Z, sizeof(FLOAT));
}

VOID GetPlayerCamp(DWORD Index)
{
	ULONG_PTR Offset_Camp[] = { g_DllBase_Client + PLAYER_OFFSET,0x8C };
	ReadProcMemByOffset(g_hProc, Offset_Camp, 1, &g_PlayerInfo[Index].Camp, sizeof(UCHAR));
}

VOID GetPlayerIdAndName(DWORD Index)
{
	ULONG_PTR Offset_Id[] = { g_DllBase_Client + PLAYER_OFFSET,0x50 };
	ReadProcMemByOffset(g_hProc, Offset_Id, 1, &g_PlayerInfo[Index].Id, sizeof(DWORD));
	GetPlayerNameById(g_PlayerInfo[Index].Id - 1, g_PlayerInfo[Index].Name, 0x10);
}

VOID GetPlayerHealth(DWORD Index)
{
	ULONG_PTR Offset_Health[] = { g_DllBase_Client + PLAYER_OFFSET,0xF70 };
	ReadProcMemByOffset(g_hProc, Offset_Health, 1, &g_PlayerInfo[Index].Health, sizeof(DWORD));
}