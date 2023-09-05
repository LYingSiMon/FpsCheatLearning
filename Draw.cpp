#include "Draw.h"
#include "Common.h"
#include "Cheat.h"
#include "spdlog/spdlog.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

#include <d3d9.h>

static LPDIRECT3D9              g_pD3D = nullptr;
static LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
			return 0;
		g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
		g_ResizeHeight = (UINT)HIWORD(lParam);
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

bool CreateDeviceD3D(HWND hWnd)
{
	if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
		return false;

	// Create the D3DDevice
	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.Windowed = TRUE;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
	//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
	if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
		return false;

	return true;
}

void CleanupDeviceD3D()
{
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
	if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}

void ResetDevice()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
	if (hr == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}

DWORD WINAPI Thread_MonitorGameWnd(PVOID pParam)
{
	RECT GameWndRect;

	while (1)
	{
		// 获取游戏窗口位置
		GetWindowRect(g_GameHwnd, &GameWndRect);

		// 移动 imgui 窗口
		MoveWindow(
			g_ImguiHwnd, 
			GameWndRect.left, GameWndRect.top,
			GameWndRect.right - GameWndRect.left, GameWndRect.bottom - GameWndRect.top,
			TRUE);

		Sleep(100);
	}
}

BOOL StartImguiDraw()
{
	// 注册窗口类、创建窗口
	WNDCLASSEX wc = { 
		sizeof(wc),
		CS_CLASSDC, 
		WndProc,
		0L, 0L, 
		GetModuleHandle(nullptr), 
		nullptr, nullptr, nullptr, nullptr, 
		"lysm_imgui_class", 
		nullptr };
	if (0 == ::RegisterClassEx(&wc))
	{
		spdlog::error("RegisterClassEx error:{}", GetLastError());
		return FALSE;
	}
	g_ImguiHwnd = ::CreateWindowEx(
		WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_LAYERED,
		wc.lpszClassName, 
		"lysm_imgui_title",
		WS_POPUP, 
		0, 0, 100, 100, 
		nullptr, nullptr,
		wc.hInstance,
		nullptr);
	if (!g_ImguiHwnd)
	{
		spdlog::error("CreateWindowEx error:{}", GetLastError());
		return FALSE;
	}

	// 指定颜色透明
	if (0 == ::SetLayeredWindowAttributes(g_ImguiHwnd, RGB(0, 100, 200), NULL, LWA_COLORKEY))
	{
		spdlog::error("SetLayeredWindowAttributes error:{}", GetLastError());
		return FALSE;
	}

	// 绘制窗口跟随游戏窗口移动
	CreateThread(0, 0, Thread_MonitorGameWnd, 0, 0, 0);

	// 创建 d3d 设备
	if (!CreateDeviceD3D(g_ImguiHwnd))
	{
		spdlog::error("CreateDeviceD3D error");
		CleanupDeviceD3D();
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return FALSE;
	}

	// 显示窗口
	::ShowWindow(g_ImguiHwnd, SW_SHOWDEFAULT);
	::UpdateWindow(g_ImguiHwnd);

	// 初始化 imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// 设置 imgui 主题风格
	ImGui::StyleColorsDark();

	// 设置 dx 版本
	ImGui_ImplWin32_Init(g_ImguiHwnd);
	ImGui_ImplDX9_Init(g_pd3dDevice);

	// 消息循环
	bool done = false;
	HRESULT result;
	D3DCOLOR clear_col_dx;
	ImVec4 clear_color = ImGui::ColorConvertU32ToFloat4(IM_COL32(0, 100, 200, 255));
	while (!done)
	{
		// 消息循环
		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				done = true;
		}
		if (done)
			break;

		// 处理窗口大小变化
		if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
		{
			g_d3dpp.BackBufferWidth = g_ResizeWidth;
			g_d3dpp.BackBufferHeight = g_ResizeHeight;
			g_ResizeWidth = g_ResizeHeight = 0;
			ResetDevice();
		}

		// 启动 imgui
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// 构造游戏透视窗口
		bool perspective_window = true;
		if (perspective_window)
		{
			static bool use_work_area = true;
			static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground;

			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
			ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

			ImGui::Begin("Perspective Window", &perspective_window, flags);

			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			for (UINT i = 1; i < g_PlayerCount; ++i)
			{
				float x1 = g_PlayerInfo[i].ScreenX1 - (g_PlayerInfo[i].ScreenY2 - g_PlayerInfo[i].ScreenY1) / 4;
				float x2 = g_PlayerInfo[i].ScreenX1 + (g_PlayerInfo[i].ScreenY2 - g_PlayerInfo[i].ScreenY1) / 4;

				if (g_PlayerInfo[0].Camp == g_PlayerInfo[i].Camp)
				{
					draw_list->AddRect(
						ImVec2(x1, g_PlayerInfo[i].ScreenY1),
						ImVec2(x2, g_PlayerInfo[i].ScreenY2),
						IM_COL32(0, 255, 0, 255),
						2.0);
				}
				else
				{
					draw_list->AddRect(
						ImVec2(x1, g_PlayerInfo[i].ScreenY1),
						ImVec2(x2, g_PlayerInfo[i].ScreenY2),
						IM_COL32(255, 0, 0, 255),
						2.0);
				}
				
			}

			ImGui::End();
		}

		// 显示
		ImGui::EndFrame();
		g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		clear_col_dx = D3DCOLOR_RGBA(
			(int)(clear_color.x * clear_color.w * 255.0f),
			(int)(clear_color.y * clear_color.w * 255.0f),
			(int)(clear_color.z * clear_color.w * 255.0f),
			(int)(clear_color.w * 255.0f));
		g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
		if (g_pd3dDevice->BeginScene() >= 0)
		{
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			g_pd3dDevice->EndScene();
		}
		result = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
		if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
			ResetDevice();
	}

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	::DestroyWindow(g_ImguiHwnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);

	return TRUE;
}