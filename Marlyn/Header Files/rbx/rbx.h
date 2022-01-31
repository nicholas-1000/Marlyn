#pragma once
#include <Windows.h>
#include <iostream>
#include <thread>

namespace rbx
{
	namespace r_functions
	{
		
	}

	namespace marlyn_functions
	{
		
	}

	void scan_game()
	{
		//visualengine : RobloxPlayerBeta.exe+3222C7C (bashar engine)
		//datamodel : RobloxPlayerBeta.exe+323B71C	(bashar engine)

		std::cout << GetModuleHandleA(nullptr) << std::endl; /* so we can see the base address of roblox */

		long long datamodel = 0;
		long long visualengine = 0;

		/* could probably do this another way, but this works fine for an open source project (NOT RELIABLE) */
		ReadProcessMemory(GetCurrentProcess(), (LPCVOID)reinterpret_cast<std::uint32_t>(GetModuleHandleA(nullptr) + 0x323B71C + 0x0), &datamodel, sizeof(datamodel), 0);
		ReadProcessMemory(GetCurrentProcess(), (LPCVOID)reinterpret_cast<std::uint32_t>(GetModuleHandleA(nullptr) + 0x3222C7C + 0x0), &visualengine, sizeof(visualengine), 0);
	}
}