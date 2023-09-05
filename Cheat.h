#pragma once
#include <Windows.h>

struct PlayerInfo {
	DWORD Id;
	CHAR Name[0x10];
	UCHAR Camp;
	DWORD Health;
	FLOAT X;
	FLOAT Y;
	FLOAT Z;
	FLOAT ScreenX1;
	FLOAT ScreenY1;
	FLOAT ScreenY2;
};

extern HANDLE g_hProc;
extern DWORD g_Pid ;
extern HWND g_ImguiHwnd;
extern HWND g_GameHwnd;
extern WNDPROC g_GameWndProc;
extern BOOL g_OpenOption;
extern ULONG_PTR g_DllBase_ServerCss, g_DllBase_Client, g_DllBase_Engine;
extern PlayerInfo g_PlayerInfo[20];
extern DWORD g_PlayerCount;
extern FLOAT g_Matrix[4][4];

BOOL GetGameWnd();
BOOL GetGamePid();
BOOL GetGameProcHandle();
BOOL GetGameModuleBase();

BOOL GetPlayerNameById(DWORD Id, PCHAR Name, DWORD Size);
BOOL GameXYZToScreenX1Y1Y2(FLOAT mcax, FLOAT mcay, FLOAT mcaz, FLOAT to[3],DWORD Index);
VOID GetGameMatrix();
VOID GetPlayerCount();
VOID GetPlayerXYZ(DWORD Index);
VOID GetPlayerCamp(DWORD Index);
VOID GetPlayerIdAndName(DWORD Index);
VOID GetPlayerHealth(DWORD Index);