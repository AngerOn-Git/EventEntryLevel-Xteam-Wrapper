#include "stdafx.h"
#include "EventEntryLevel.h"
#include "Offset.h"
#include "Util.h"
#include <string>
#include <vector>
#include <sstream>

#include <Windows.h>
#include <cstdint>

#define CHECK_CC_LEVEL 0x007E4AE1

int m_EventsUseReset = GetPrivateProfileIntA("General", "byReset", 0, "./eel.ini");

int m_BloodCastleEntryLevelCommon[7][2];
int m_BloodCastleEntryLevelSpecial[7][2];
int m_DevilSquareEntryLevelCommon[6][2];
int m_DevilSquareEntryLevelSpecial[6][2];
int m_ChaosCastleEntryLevelCommon[6][2];
int m_ChaosCastleEntryLevelSpecial[6][2];
int m_KalimaEntryLevelCommon[7][2];
int m_KalimaEntryLevelSpecial[7][2];
int m_IllusionTempleEntryLevelMin[5];
int m_IllusionTempleEntryLevelMax[5];

void LoadEventEntryLevel_arr(int* eventPtrMin, int* eventPtrMax, int count, std::string eventName) {
	for (int i = 0; i < count; i++) {
		std::string configNameMin = "EntryLevelMin" + std::to_string(i);
		std::string configNameMax = "EntryLevelMax" + std::to_string(i);

		int min = GetPrivateProfileIntA(eventName.c_str(), configNameMin.c_str(), 0, "./eel.ini");
		int max = GetPrivateProfileIntA(eventName.c_str(), configNameMax.c_str(), 400, "./eel.ini");

		eventPtrMin[i] = min;
		eventPtrMax[i] = max;
	}
}

void LoadEventEntryLevel(int eventPtrCommon[][2], int eventPtrSpecial[][2], int count, std::string eventName) {
	for (int i = 0; i < count; i++) {
		std::string configNameC = "EntryLevelCommon" + std::to_string(i);
		std::string configNameS = "EntryLevelSpecial" + std::to_string(i);

		char elc[50];
		char els[50];

		GetPrivateProfileStringA(eventName.c_str(), configNameC.c_str(), "0,400", elc, 50, "./eel.ini");
		GetPrivateProfileStringA(eventName.c_str(), configNameS.c_str(), "0,400", els, 50, "./eel.ini");

		// Parse EntryLevelCommon
		std::stringstream ssCommon(elc);
		std::string segment;
		int j = 0;
		while (std::getline(ssCommon, segment, ',') && j < 2) {
			try {
				eventPtrCommon[i][j] = std::stoi(segment);
			}
			catch (...) {
				eventPtrCommon[i][j] = j * 400;
			}
			j++;
		}

		// Parse EntryLevelSpecial (if provided)
		if (eventPtrSpecial != nullptr) {
			std::stringstream ssSpecial(els);
			j = 0;
			while (std::getline(ssSpecial, segment, ',') && j < 2) {
				try {
					eventPtrSpecial[i][j] = std::stoi(segment);
				}
				catch (...) {
					eventPtrSpecial[i][j] = j * 400;
				}
				j++;
			}
		}
	}
}

DWORD MainDllAddrResets = 0x0;

void __declspec (naked) fixCheckLevelToReset_CC() {
	static DWORD jumpBack = 0x007E4AE5;
	_asm {
		mov ecx, MainDllAddrResets
		movzx edx, word ptr[ecx]
		JMP jumpBack
	}
}

void __declspec (naked) fixCheckLevelToReset_SMT() {
	static DWORD jumpBack = 0x005A230E;
	_asm {
		mov ecx, MainDllAddrResets
		movzx edx, word ptr[ecx]
		JMP jumpBack
	}
}

void __declspec (naked) fixCheckLevelToReset_DS() {
	static DWORD jumpBack = 0x008787A4;
	_asm {
		mov ecx, MainDllAddrResets
		movzx edx, word ptr[ecx]
		JMP jumpBack
	}
}

void __declspec (naked) fixCheckLevelToReset_BC() {
	static DWORD jumpBack = 0x0087469D;
	_asm {
		mov ecx, MainDllAddrResets
		movzx edx, word ptr[ecx]
		JMP jumpBack
	}
}

void __declspec (naked) fixCheckLevelToReset_IL() {
	static DWORD jumpBack = 0x0086938A;
	_asm {
		mov ecx, MainDllAddrResets
		movzx edx, word ptr[ecx]
		JMP jumpBack
	}
}

DWORD jumpBack_IL_ML = 0x0;
DWORD resets = 0;
void fixCheckLevelToReset_IL_ML_ex() {
	resets = *(DWORD*)(MainDllAddrResets);
}
void __declspec (naked) fixCheckLevelToReset_IL_ML() {
	_asm {
		call fixCheckLevelToReset_IL_ML_ex
		mov eax, resets
		xor ecx, ecx
		mov ecx, eax
		JMP jumpBack_IL_ML
	}
}

void loadIllusionInfo() {
	MemoryCpy(0x00D48EAC, m_IllusionTempleEntryLevelMin, sizeof(m_IllusionTempleEntryLevelMin));

	MemoryCpy(0x00D48EC0, m_IllusionTempleEntryLevelMax, sizeof(m_IllusionTempleEntryLevelMax));
}

void InitEventEntryLevel(DWORD _MAINDLL) // OK
{
	MainDllAddrResets = _MAINDLL + 0x1785C;

	if (m_EventsUseReset > 0) {
		jumpBack_IL_ML = _MAINDLL + 0x24E0;
		DWORD IL_ML_addr = _MAINDLL + 0x24D3;

		SetCompleteHook(0xE9, 0x007E4ADB, &fixCheckLevelToReset_CC); //CC
		SetCompleteHook(0xE9, 0x005A2304, &fixCheckLevelToReset_SMT); //DS BC IL
		SetCompleteHook(0xE9, 0x0087879A, &fixCheckLevelToReset_DS); //DS
		SetCompleteHook(0xE9, 0x00874694, &fixCheckLevelToReset_BC); //BC
		SetCompleteHook(0xE9, 0x00869380, &fixCheckLevelToReset_IL); //IL
		SetCompleteHook(0xE9, IL_ML_addr, &fixCheckLevelToReset_IL_ML); //IL_ML
		SetByte(_MAINDLL + 0x24E6, 0x7F); // Compare IL_ML to greater than
	}

	LoadEventEntryLevel(m_BloodCastleEntryLevelCommon, m_BloodCastleEntryLevelSpecial, 7, "BloodCastle");
	LoadEventEntryLevel(m_DevilSquareEntryLevelCommon, m_DevilSquareEntryLevelSpecial, 6, "DevilSquare");
	LoadEventEntryLevel(m_ChaosCastleEntryLevelCommon, m_ChaosCastleEntryLevelSpecial, 6, "ChaosCastle");
	LoadEventEntryLevel(m_KalimaEntryLevelCommon, m_KalimaEntryLevelSpecial, 7, "kalima");
	LoadEventEntryLevel_arr((int*)m_IllusionTempleEntryLevelMin, (int*)m_IllusionTempleEntryLevelMax, 5, "IllusionTemple");

	for (int n = 0; n < 7; n++)
	{
		SetDword(0x008740FB + (0x1A * n), m_BloodCastleEntryLevelCommon[n][0]);
		SetDword(0x00874108 + (0x1A * n), m_BloodCastleEntryLevelCommon[n][1]);
		SetDword(0x008741CB + (0x1A * n), m_BloodCastleEntryLevelSpecial[n][0]);
		SetDword(0x008741D8 + (0x1A * n), m_BloodCastleEntryLevelSpecial[n][1]);
	}

	for (int n = 0; n < 6; n++)
	{
		SetDword(0x00877C0B + (0x1A * n), m_DevilSquareEntryLevelCommon[n][0]);
		SetDword(0x00877C18 + (0x1A * n), m_DevilSquareEntryLevelCommon[n][1]);
		SetDword(0x00877CC1 + (0x1A * n), m_DevilSquareEntryLevelSpecial[n][0]);
		SetDword(0x00877CCE + (0x1A * n), m_DevilSquareEntryLevelSpecial[n][1]);
	}

	MemoryCpy(0x00D457C8, m_ChaosCastleEntryLevelCommon, sizeof(m_ChaosCastleEntryLevelCommon));

	MemoryCpy(0x00D457F8, m_ChaosCastleEntryLevelSpecial, sizeof(m_ChaosCastleEntryLevelSpecial));

	MemoryCpy(0x00D4DCF0, m_KalimaEntryLevelCommon, sizeof(m_KalimaEntryLevelCommon));

	MemoryCpy(0x00D4DD28, m_KalimaEntryLevelSpecial, sizeof(m_KalimaEntryLevelSpecial));

	loadIllusionInfo();

	int ilMl = GetPrivateProfileIntA("IllusionTemple", "EntryLevelMin5", 400, "./eel.ini") - 1; //JG -> fix Compare IL_ML to greater than
	DWORD iL_checkMl = _MAINDLL + 0x24E2;
	SetDword(iL_checkMl, (DWORD)ilMl);

	SetDword(0x007E4BB8, 0x270F); // Chaos Castle MaxLevel

	SetDword(0x008FF543, 0x270F); // Kalima MaxLevel
}