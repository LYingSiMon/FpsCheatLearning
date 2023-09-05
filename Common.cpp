#include "pch.h"
#include "Common.h"

#include <tlhelp32.h>
#include <psapi.h>

ULONG_PTR GetModuleBaseByPid(DWORD Pid, PCHAR ModuleName)
{
	HANDLE  hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, Pid);
	MODULEENTRY32 Module = { sizeof(MODULEENTRY32) };

	if (!Module32First(hModuleSnap, &Module))
	{
		return 0;
	}
	do
	{
		if (_stricmp(ModuleName, Module.szModule) == 0)
		{
			return (ULONG_PTR)Module.modBaseAddr;
		}
	} while (Module32Next(hModuleSnap, &Module));

	return 0;
}

BOOL ReadProcMemByOffset(HANDLE hProc, ULONG_PTR Offset[], DWORD OffsetCount, PVOID Result, DWORD Size)
{
	BYTE Buffer[256] = { 0 };
	ULONG_PTR Temp = 0;

	for (UINT i = 0; i <= OffsetCount; ++i)
	{
		Temp = *(PULONG_PTR)Buffer;
		Temp += Offset[i];

		memset(Buffer, 0, 256);
		if (i == OffsetCount)
		{
			if (!ReadProcessMemory(hProc, (PVOID)Temp, Buffer, Size, 0))
			{
				return FALSE;
			}
		}
		else
		{
			if (!ReadProcessMemory(hProc, (PVOID)Temp, Buffer, sizeof(ULONG_PTR), 0))
			{
				return FALSE;
			}
		}
	}

	memcpy(Result, Buffer, Size);

	return TRUE;
}