#pragma once
#include <Windows.h>
#include <iostream>
#include <thread>
#include <vector>
#include <TlHelp32.h>

struct vec2 {
	float x;
	float y;
};

struct vec3 {
	float x;
	float y;
	float z;
};

struct vec4 {
	float x;
	float y;
	float z;
	float w;
};


namespace rbx
{
	namespace pre
	{
		std::uintptr_t visualengine;
		std::uintptr_t game;
		std::uintptr_t workspace;
		std::uintptr_t players;
	}

	namespace marlyn_functions
	{

		template <typename T>
		static T read(LPCVOID address, SIZE_T size = sizeof(T));

		template<typename T>
		T read(LPCVOID add, SIZE_T s)
		{
			T buf;
			ReadProcessMemory(GetCurrentProcess(), add, &buf, s, 0);
			return buf;
		}

		const char* getclass(int self)
		{
			return (const char*)(*(int(**)(void))(*(int*)self + 16))();
		}

		int find_child_of_ins(int ins, const char* name)
		{
			DWORD start = *(DWORD*)(ins + 0x2C);
			DWORD end = *(DWORD*)(start + 4);

			for (int i = *(int*)start; i != end; i += 8)
			{
				if (memcmp(getclass(*(int*)i), name, strlen(name)) == 0)
				{
					return *(int*)i;
				}
			}
		}

		std::vector<DWORD> getchildren(DWORD ins)
		{
			std::vector<DWORD> children;
			DWORD start = *reinterpret_cast<DWORD*>(ins + 0x2C);
			DWORD end = *reinterpret_cast<DWORD*>(start + 4);

			for (DWORD i = *reinterpret_cast<DWORD*>(start); i < end; i += 8)
				children.push_back(*reinterpret_cast<DWORD*>(i));
		
			return children;
		}

		uintptr_t return_character(uintptr_t player)
		{
			return *reinterpret_cast<std::uintptr_t*>(player + 0x7C);
		}

		vec3 get_position(DWORD ins)
		{ /*SORRY i was too lazy to rewrite this for this project (credits: atlas/crishoux)*/
			std::uintptr_t primitive = read<std::uintptr_t>(reinterpret_cast<LPCVOID>(ins + 0xC0));
			return read<vec3>(reinterpret_cast<LPCVOID>(primitive + 0xfc));
		}

		std::string get_string(const char* s)
		{ /*CREDITS TO BDDIT*/
			std::string newS;
			size_t counter = 0;
			unsigned char c;
			while (c = *reinterpret_cast<unsigned char*>((DWORD)s + counter))
			{
				counter++;
				newS.push_back(c);
			}
			return newS;
		}

		std::string get_name(DWORD ins)
		{
			std::uintptr_t str = *reinterpret_cast<std::uintptr_t*>(ins + 0x28);
			if (*reinterpret_cast<size_t*>((DWORD)str + 20) >= 16)
			{
				return get_string((const char*)*reinterpret_cast<std::uintptr_t*>(str));
			}
			else
			{
				return get_string((const char*)str);
			}

		}

		DWORD return_child(DWORD instance, std::string name) {
			std::vector<DWORD> children = getchildren(instance);

			for (DWORD i = 0; i < children.size(); i++)
			{
				if (get_name(children[i]) == name) {
					return children[i];
				}
			}
		}
		
	}

	void scan_game()
	{
		//visualengine : RobloxPlayerBeta.exe+322042C (bashar engine)
		//datamodel : RobloxPlayerBeta.exe+3238F0C (bashar engine)

		std::cout << GetModuleHandleA(nullptr) << std::endl; /* so we can see the base address of roblox */

		long long datamodel = 0;
		long long visualengine = 0;

		/* could probably do this another way, but this works fine for an open source project (NOT RELIABLE) */

		ReadProcessMemory(GetCurrentProcess(), (LPCVOID)(reinterpret_cast<std::uint32_t>(GetModuleHandleA(nullptr)) + 0x3238F0C), &datamodel, sizeof(datamodel), 0);
		ReadProcessMemory(GetCurrentProcess(), (LPCVOID)(reinterpret_cast<std::uint32_t>(GetModuleHandleA(nullptr)) + 0x322042C), &visualengine, sizeof(visualengine), 0);

		rbx::pre::visualengine = visualengine;
		rbx::pre::game = datamodel;

		printf("datamodel: %x\n", datamodel);
		printf("visualengine: %x\n", visualengine);

		/*now we'll use datamodel to get other stuff like : workspace, players, camera, etc. using the functions above*/

		pre::workspace = marlyn_functions::find_child_of_ins(datamodel, "Workspace");
		printf("workspace: %x\n", pre::workspace);

		pre::players = marlyn_functions::find_child_of_ins(datamodel, "Players");
		printf("players: %x\n", pre::players);
	}
}