#include "../../hooks.h"
#include "../../../../Dependecies/ImGui/imgui_impl_dx9.h"
#include "../../../SDK/Menu/gui.h"

HRESULT __stdcall h::Reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params) {

	static auto original = detour::D3DReset.GetOriginal<decltype(&h::Reset)>();

	ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto result = original(device, params);

	ImGui_ImplDX9_CreateDeviceObjects();

	return result;
}

long __stdcall h::EndScene(IDirect3DDevice9* device) {

	static auto original = detour::D3DEndScene.GetOriginal<decltype(&h::EndScene)>();

	static const auto  returnAddress = _ReturnAddress();

	const auto result = original(device);

	if (_ReturnAddress() == returnAddress)
		return result;

	if (!menu::setup)
		menu::SetupMenu(device);

	menu::Render();

	return result;
}