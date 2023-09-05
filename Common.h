#pragma once
#include <Windows.h>

ULONG_PTR GetModuleBaseByPid(DWORD Pid, PCHAR ModuleName);

BOOL ReadProcMemByOffset(HANDLE hProc, ULONG_PTR Offset[], DWORD OffsetCount,PVOID Result, DWORD Size);