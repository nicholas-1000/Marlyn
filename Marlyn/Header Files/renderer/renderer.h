#pragma once
#include "../ocornut/imgui.h"
#include "../ocornut/imgui_impl_dx11.h"
#include "../ocornut/imgui_impl_win32.h"
#include "../globals/globals.h"
#include <rbx.h>
#include <win32.h>

/*functions*/
ImVec2 getrobloxsize_(HWND roblox)
{
	RECT rect;
	GetWindowRect(roblox, &rect);

	float w = static_cast<float>(rect.right - 16 - rect.left);
	float h = static_cast<float>(rect.bottom + 24 - rect.top);

	return { w, h };
}

namespace menu
{
	/*frontend*/

	void Label(std::string text)
	{ /* https://stackoverflow.com/questions/64653747/how-to-center-align-text-horizontally */
		auto windowWidth = ImGui::GetWindowSize().x;
		auto textWidth = ImGui::CalcTextSize(text.c_str()).x;

		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		ImGui::Text(text.c_str());
	}

	void _menu()
	{
		ImGui::Begin("Marlyn", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

		Label("Marlyn // https://github.com/nickisntreal/Marlyn");

		ImGui::Checkbox("ESP Dot", &globals::esp_box);

		ImGui::End();
	}
}

vec2 world2screen(vec3 p)
{
	if (!FindWindowW(0, L"Roblox"))
		exit(0); /* will exit process if roblox isnt running */

	unsigned int r_width, r_height;

	float matrix[16];
	ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<LPCVOID>(rbx::pre::visualengine + 0x120), &matrix, sizeof(matrix), 0);

	/* going to use imvec2 to get roblox dimensions */

	ImVec2 RobloxDimensions = getrobloxsize_(FindWindowW(0, L"Roblox"));

	/* this bit is from atlas */

	vec4 coords;
	coords.x = (p.x * matrix[0]) + (p.y * matrix[1]) + (p.z * matrix[2]) + matrix[3];
	coords.y = (p.x * matrix[4]) + (p.y * matrix[5]) + (p.z * matrix[6]) + matrix[7];
	coords.z = (p.x * matrix[8]) + (p.y * matrix[9]) + (p.z * matrix[10]) + matrix[11];
	coords.w = (p.x * matrix[12]) + (p.y * matrix[13]) + (p.z * matrix[14]) + matrix[15];

	if (coords.w < 0.1f)
		return { -1, -1 };

	vec3 c;
	c.x = coords.x / coords.w;
	c.y = coords.y / coords.w;
	c.z = coords.z / coords.w;

	return {
	(RobloxDimensions.x / 2 * c.x) + (c.x + RobloxDimensions.x / 2),
	-(RobloxDimensions.y / 2 * c.y) + (c.y + RobloxDimensions.y / 2)
	};
}

namespace M_renderer
{
	void _renderer()
	{
		std::vector<uintptr_t> Players; /*make a vector to store all the players into*/

		Players = []() {
			std::vector<uintptr_t> ret;

			for (std::uintptr_t child : rbx::marlyn_functions::getchildren(rbx::pre::players))
				ret.push_back(child);

			return ret;
		}();

		for (uintptr_t plr : Players)
		{
			/*returns the players character*/
			std::uintptr_t character = rbx::marlyn_functions::return_character(plr);

			/*humanoidrootpart is stored inside of Character ^^*/
			std::uintptr_t humanoidrootpart = rbx::marlyn_functions::return_child(character, "HumanoidRootPart");

			/*get hrp position*/
			vec3 humanoidrootpart_position = rbx::marlyn_functions::get_position(humanoidrootpart);

			/*world2screen the hrp position*/
			vec2 w2s_humanoidrootpart_position = world2screen(humanoidrootpart_position);

			if(globals::esp_box)
				ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(w2s_humanoidrootpart_position.x, w2s_humanoidrootpart_position.y), 5, ImColor(255, 255, 255));
		}
	}
}