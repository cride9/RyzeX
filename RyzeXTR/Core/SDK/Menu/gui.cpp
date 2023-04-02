#include "gui.h"
#include "fontBytes.h"	
#include "../../globals.h"
#include "config.h"
#include <string>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProcess(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	if (GetAsyncKeyState(VK_INSERT) & 1)
		menu::open = !menu::open;

	if (menu::open && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {

		return 1L;
	}

	return CallWindowProc(menu::originalWindowProcess, hWnd, msg, wParam, lParam);
}

void Style() {

	ImGuiStyle& st = ImGui::GetStyle();
	ImGuiIO& io = ImGui::GetIO();

	io.ConfigWindowsResizeFromEdges = true;

	ImFontConfig cfg;
	cfg.FontDataOwnedByAtlas = false;

	menu::logoFont = io.Fonts->AddFontFromMemoryTTF(byteFonts::logoFont, 50.f, 50.f);
	menu::xtrFont = io.Fonts->AddFontFromMemoryTTF(byteFonts::xtrFont, 17.f, 17.f);
	menu::childFont = io.Fonts->AddFontFromMemoryTTF(byteFonts::childFont, 15.f, 15.f);
	menu::tabFont = io.Fonts->AddFontFromMemoryTTF(byteFonts::childFont, 18.f, 18.f);

	st.WindowMinSize = ImVec2(1195 / 2, 760 / 2);
	st.WindowPadding = ImVec2(5, 5);

	st.Colors[ImGuiCol_WindowBg] = ImColor(12, 12, 12);
	st.Colors[ImGuiCol_Border] = ImColor(40, 40, 40);
	st.Colors[ImGuiCol_BorderShadow] = ImColor(0, 0, 0);

	st.Colors[ImGuiCol_ChildBg] = ImColor(22, 22, 22);
	st.Colors[ImGuiCol_Text] = ImColor(212, 212, 212);

	st.Colors[ImGuiCol_ButtonHovered] = ImColor(31, 31, 31);
	st.Colors[ImGuiCol_ButtonActive] = ImColor(38, 38, 38);
	st.Colors[ImGuiCol_Button] = ImColor(22, 22, 22);

	st.Colors[ImGuiCol_FrameBgActive] = ImColor(40, 36, 41);
	st.Colors[ImGuiCol_FrameBgHovered] = ImColor(40, 36, 41);
	st.Colors[ImGuiCol_FrameBg] = ImColor(40, 36, 41);

	st.Colors[ImGuiCol_SliderGrab] = ImColor(111, 203, 243);
	st.Colors[ImGuiCol_SliderGrabActive] = ImColor(111, 203, 243);

	st.Colors[ImGuiCol_CheckMark] = ImColor(255, 255, 255);

	st.Colors[ImGuiCol_HeaderActive] = ImColor(38, 38, 38);
	st.Colors[ImGuiCol_HeaderHovered] = ImColor(31, 31, 31);
	st.Colors[ImGuiCol_Header] = ImColor(40, 36, 41);

	st.FramePadding = ImVec2(0.2f, 0.2f);
	st.SelectableTextAlign = ImVec2(0.02f, 0.f);
	st.ScrollbarRounding = 0.f;
	st.ScrollbarSize = 1.f;
	st.ColorButtonPosition = ImGuiDir_Right;
}

void menu::Render() noexcept {

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();

	if (menu::open) {

		HandleMenuElements();
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

bool menu::SetupWindowClass(const char* szWindowClassName) noexcept {

	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW;
	windowClass.lpfnWndProc = DefWindowProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandle(NULL);
	windowClass.hIcon = NULL;
	windowClass.hCursor = NULL;
	windowClass.hbrBackground = NULL;
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = szWindowClassName;
	windowClass.hIconSm = NULL;

	if (!RegisterClassEx(&windowClass))
		return false;

	return true;
}

void menu::DestroyWindowClass() noexcept {

	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

bool menu::SetupWindow(const char* szWindowName) noexcept {

	//                    lpClassName,				lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam
	window = CreateWindow(windowClass.lpszClassName, szWindowName, WS_OVERLAPPEDWINDOW, 0, 0, 500, 700, 0, 0, windowClass.hInstance, 0);

	if (!window)
		return false;

	return true;
}

void menu::DestroyWindow() noexcept {

	if (window)
		DestroyWindow(window);
}

bool menu::SetupDirectX() noexcept {

	const auto handle = GetModuleHandle("d3d9.dll");

	if (!handle)
		return false;

	using Fn = LPDIRECT3D9(__stdcall*)(UINT);
	const auto create = reinterpret_cast<Fn>(GetProcAddress(handle, "Direct3DCreate9"));

	if (!create)
		return false;

	d3d9 = create(D3D_SDK_VERSION);

	if (!d3d9)
		return false;

	D3DPRESENT_PARAMETERS parameters = {};
	parameters.BackBufferWidth = 0;
	parameters.BackBufferHeight = 0;
	parameters.BackBufferFormat = D3DFMT_UNKNOWN;
	parameters.BackBufferCount = 0;
	parameters.MultiSampleType = D3DMULTISAMPLE_NONE;
	parameters.MultiSampleQuality = NULL;
	parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	parameters.hDeviceWindow = window;
	parameters.Windowed = 1;
	parameters.EnableAutoDepthStencil = 0;
	parameters.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
	parameters.Flags = NULL;
	parameters.FullScreen_RefreshRateInHz = 0;
	parameters.PresentationInterval = 0;

	if (d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, window, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT, &parameters, &device) < 0)
		return false;

	return true;
}

void menu::DestroyDirectX() noexcept {

	if (device) {
		device->Release();
		device = NULL;
	}

	if (d3d9) {
		d3d9->Release();
		d3d9 = NULL;
	}
}

void menu::Setup() {

	if (!SetupWindowClass("CoolHack001"))
		throw std::runtime_error("Failed to create window class");

	if (!SetupWindow("CoolHack Window"))
		throw std::runtime_error("Failed to create window");

	if (!SetupDirectX())
		throw std::runtime_error("Failed to create directx");

	DestroyWindow();
	DestroyWindowClass();
}

void menu::SetupMenu(LPDIRECT3DDEVICE9 device) noexcept {

	auto parameters = D3DDEVICE_CREATION_PARAMETERS{};
	device->GetCreationParameters(&parameters);

	window = parameters.hFocusWindow;

	originalWindowProcess = reinterpret_cast<WNDPROC>(SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProcess)));

	ImGui::CreateContext();

	Style();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);

	setup = true;
}

void menu::Destroy() noexcept {

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(originalWindowProcess));

	DestroyDirectX();
}
