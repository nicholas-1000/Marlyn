#pragma once
#include "../renderer/renderer.h"
#include <d3d11.h>

#include <iostream>
#include <ShObjIdl_core.h>
#include <TlHelp32.h>

#include <Windows.h>
#include <tchar.h>
#include <dwmapi.h>




struct handle_data {
	unsigned long process_id;
	HWND window_handle;
};

BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam)
{ /*somewhere on stackoverflow*/

	handle_data& data = *(handle_data*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	char name[255];
	GetWindowTextA(handle, name, sizeof(name));

	if (data.process_id != process_id || strcmp(name, "Roblox") != NULL)
		return TRUE;
	data.window_handle = handle;
	return FALSE;
}


HWND find_main_window(unsigned long process_id)
{ /*somewhere on stackoverflow*/ 

	handle_data data;
	data.process_id = process_id;
	data.window_handle = 0;
	EnumWindows(enum_windows_callback, (LPARAM)&data);
	return data.window_handle;
}

// Data
static ID3D11Device* g_pd3dDevice = NULL;
static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
static IDXGISwapChain* g_pSwapChain = NULL;
static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool shown = true;

/*functions*/
ImVec2 getrobloxsize(HWND roblox)
{
	RECT rect;
	GetWindowRect(roblox, &rect);

	float w = static_cast<float>(rect.right - 16 - rect.left);
	float h = static_cast<float>(rect.bottom + 24 - rect.top);

	return { w, h };
}

HWND create_window() {
	HWND roblox = find_main_window(GetCurrentProcessId());
	ImVec2 rsize = getrobloxsize(roblox);

	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("ex"), NULL };
	::RegisterClassEx(&wc);
	HWND hwnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST, wc.lpszClassName, L"westeria", WS_POPUP | WS_VISIBLE, 0, 0, 1920, 1080, NULL, NULL, wc.hInstance, NULL);
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
	}
	return hwnd;
}

void _imgui(HWND wnd) 
{
	/*create context to setup*/
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg] = ImColor(30, 30, 30);

	ImGui_ImplWin32_Init(wnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
}


void _thread(HWND wnd, HWND hwnd)
{
	ImVec4 clear_color = ImVec4(NULL, NULL, NULL, NULL);

	MSG message;
	while (PeekMessage(&message, 0u, 0u, 0u, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	RECT s;
	GetWindowRect(wnd, &s);

	ImVec2 rsize = getrobloxsize(wnd);

	if (wnd != GetForegroundWindow() && hwnd != GetForegroundWindow())
		MoveWindow(hwnd, 0u, 0u, 0u, 0u, TRUE);
	else
	{
		rsize.y = rsize.y - 63;
		MoveWindow(hwnd, s.left + 8, s.top + 31, int(rsize.x), int(rsize.y), TRUE);
	}

	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		shown = !shown;

		if (shown)
		{
			long winlong = GetWindowLong(hwnd, GWL_EXSTYLE);
			if (winlong != WS_EX_LAYERED | WS_EX_TOPMOST)
				SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TOPMOST);

		}
		else if (!shown)
		{
			long winlong = GetWindowLong(hwnd, GWL_EXSTYLE);

			if (winlong != WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT)
				SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT);
		}
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	M_renderer::_renderer();

	if (shown)
	{
		menu::_menu();
	}

	ImGui::EndFrame();
	FLOAT color[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };

	ImGui::Render();
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
	g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, color);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	g_pSwapChain->Present(0, NULL);
}

int start_imgui()
{
	HWND roblox = find_main_window(GetCurrentProcessId());
	ImVec2 rsize = getrobloxsize(roblox);


	HWND hwnd = create_window();
	SetLayeredWindowAttributes(hwnd, RGB(NULL, NULL, NULL), 255, LWA_ALPHA);

	MARGINS margin = { -1 };
	DwmExtendFrameIntoClientArea(hwnd, &margin);
	_imgui(hwnd);

	while (true)
	{
		_thread(roblox, hwnd);
	}
}


// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			CreateRenderTarget();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
