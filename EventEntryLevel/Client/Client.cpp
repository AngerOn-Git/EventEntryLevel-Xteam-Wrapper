#include "stdafx.h"
#include "Offset.h"
#include "Util.h"
#include "EventEntryLevel.h"

#include <Windows.h>
#include <cstdint>

#include <tlhelp32.h>

const wchar_t* GetWC(const char* c)
{
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	mbstowcs(wc, c, cSize);

	return wc;
}

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!_wcsicmp(GetWC(modEntry.szModule), modName))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

DWORD MAINDLL = 0x0;

void _EntryProc(HINSTANCE hins)
{
}

DWORD lastMainState = 0;
DWORD lastReconnectState = -1;
void check() {
	if (*(DWORD*)(MAIN_SCREEN_STATE) != lastMainState) {
		if (*(DWORD*)(MAIN_SCREEN_STATE) == 5) {
			loadIllusionInfo();
		}
	}
	lastMainState = *(DWORD*)(MAIN_SCREEN_STATE);

	DWORD reconnectState = MAINDLL + 0x17880;

	if (*(DWORD*)(reconnectState) != reconnectState) {
		if (*(DWORD*)(reconnectState) == 0) {
			loadIllusionInfo();
		}
	}
	lastReconnectState = *(DWORD*)(reconnectState);

}


#pragma optimize("", off)

DWORD mainPStruct = 0x0;
BYTE callType = 0x0;

void callPreviewsStructs_call() {
	//static DWORD mainPStruct = MAINDLL + 0xC160;
	check();
	_asm {
		call mainPStruct
	}
}

void callPreviewsStructs_jmp() {
	//static DWORD mainPStruct = MAINDLL + 0xC160;
	check();
	_asm {
		jmp mainPStruct
	}
}

extern "C" _declspec(dllexport) void EntryProc(){
	MAINDLL = GetModuleBaseAddress(GetCurrentProcessId(), L"Main.dll");
	InitEventEntryLevel(MAINDLL);
	mainPStruct =  (DWORD)0x005B96E9 + *(DWORD*)(0x005B96E9) + (DWORD)0x4;
	callType = *(BYTE*)(0x005B96E8);
	if (callType == 0xE8) {
		SetCompleteHook(0xE8, 0x005B96E8, &callPreviewsStructs_call);
	}
	else {
		SetCompleteHook(0xE8, 0x005B96E8, &callPreviewsStructs_jmp);
	}
}

#pragma optimize("", on)

BOOL APIENTRY DllMain(HANDLE hModule,DWORD ul_reason_for_call,LPVOID lpReserved) // OK
{
	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			//_EntryProc((HINSTANCE)hModule);
			break;
		case DLL_PROCESS_DETACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
	}

	return 1;
}
