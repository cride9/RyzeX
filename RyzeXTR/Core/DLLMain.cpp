#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdint>
#include <thread>
#include <iostream>
#include "Hooks/hooks.h"
#include "SDK/Menu/gui.h"
#include "Interface/interfaces.h"
#include "SDK/NetVar/Netvar.h"
#include "hungarians.h"
#include "globals.h"
#include "SDK/math.h"
#include "SDK/Menu/config.h"
#include <mmsystem.h>
#include "memeSounds.h"
DWORD WINAPI CheatThread(PVOID);

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {

		DisableThreadLibraryCalls(hinstDLL);

		auto thread = CreateThread(nullptr, 0, CheatThread, hinstDLL, 0, nullptr);

		if (thread)
			CloseHandle(thread);
	}

	return TRUE;
}

void OpenConsole() {

	AllocConsole();
	freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
	SetConsoleTitleA("Alpha Console");
}

void CloseConsole() {

	fclose((FILE*)stdin);
	fclose((FILE*)stdout);

	HWND hw_ConsoleHwnd = GetConsoleWindow();
	FreeConsole();
	PostMessageW(hw_ConsoleHwnd, WM_CLOSE, 0, 0);
}

void SetupFonts() {

	g::fonts::NameESP = i::Surface->FontCreate();
	g::fonts::HealthESP = i::Surface->FontCreate();
	g::fonts::FlagESP = i::Surface->FontCreate();

	i::Surface->SetFontGlyphSet(g::fonts::NameESP, "Verdana", 12, FW_NORMAL, 0, 0, FONTFLAG_OUTLINE);
	i::Surface->SetFontGlyphSet(g::fonts::HealthESP, "Verdana", 10, FW_NORMAL, 0, 0, FONTFLAG_OUTLINE);
	i::Surface->SetFontGlyphSet(g::fonts::FlagESP, "Small Fonts", 11, FW_NORMAL, 0, 0, FONTFLAG_OUTLINE);
}

DWORD WINAPI CheatThread(PVOID hinstDLL) {

	if (!GetModuleHandleA("serverbrowser.dll"))
		Sleep(200);

#if _DEBUG
	OpenConsole();
#endif

	Config2->CreateMainDirectory();
	Config2->RefreshSounds();
	Config2->RefreshConfigs();
	i::SetupInterfaces();
	SetupFonts();
	n::SetupNetvars();
	//PlaySound(reinterpret_cast<LPCSTR>(memeSound::BUWAWA), NULL, SND_MEMORY | SND_ASYNC);
	menu::Setup();
	M::Setup();
	h::SetupHooks();
	g::entityListener.Setup();

#if NDEBUG
	//util::LogConsole("[RELEASE] Built date: " __DATE__ " at " __TIME__ "\n");
#endif
#if _DEBUG
	//util::LogConsole("[DEVELOPER] Built date: " __DATE__ " at " __TIME__ "\n");
#endif
#if ALPHA
	//util::LogConsole("[ALPHA] Built date: " __DATE__ " at " __TIME__ "\n");
#endif

#if _DEBUG

	while (!GetAsyncKeyState(VK_DELETE))
		Sleep(200);

	g::entityListener.Destroy();
	menu::open = false;
	h::DestroyHooks();
	menu::Destroy();
	i::EngineClient->ClientCmdUnrestricted("cl_fullupdate");

#endif

#if NDEBUG || ALPHA

	while (true)
		Sleep(5000);
#endif

#if _DEBUG
	CloseConsole();
#endif

	FreeLibraryAndExitThread(static_cast<HMODULE>(hinstDLL), 0);

	return TRUE;
}
