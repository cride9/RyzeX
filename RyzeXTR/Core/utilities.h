#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <cstddef>
#include <iostream>
#include "SDK/DataTyes/Color.h"
#include <fstream>

#define RYZEXCOLOR Color(222, 153, 42)
#define INRANGE(x,a,b)   (x >= a && x <= b)
#define GET_BYTE( x )    (GET_BITS(x[0]) << 4 | GET_BITS(x[1]))
#define GET_BITS( x )    (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))

namespace util {

	inline void Print(const char* csText) {
		// YES endl is slower than just a \n BUT IT ALSO CLEARS THE BUFFER (learn C ty)
#if _DEBUG
		std::cout << csText << std::endl;
#endif
	}

	inline void Print(const char* csText1, const char* csText2) {
		// YES endl is slower than just a \n BUT IT ALSO CLEARS THE BUFFER (learn C ty)
		std::cout << csText1 << csText2 << std::endl;
	}

	inline void LogConsole(const char* msg, Color color = Color(255, 255, 255)) {

		static constexpr int LS_MESSAGE = 0;

		using find_channel_fn_t = int(__cdecl*)(const char* name);
		static const auto s_channel_id = ((find_channel_fn_t)GetProcAddress(GetModuleHandleA("tier0.dll"), "LoggingSystem_FindChannel"))("Console");

		using log_direct_fn_t = int(__cdecl*)(int id, int severity, Color color, const char* msg);
		static const auto s_log_direct = (log_direct_fn_t)GetProcAddress(GetModuleHandleA("tier0.dll"), "LoggingSystem_LogDirect");

		static std::string prefix = "[RyzeX] ";
		s_log_direct(s_channel_id, LS_MESSAGE, RYZEXCOLOR, prefix.c_str());

		s_log_direct(s_channel_id, LS_MESSAGE, color, msg);
	}

	template <typename T, typename ... Args_t>
	inline constexpr T CallVFunc(void* thisptr, std::size_t nIndex, Args_t... argList) {

		using VirtualFn = T(__thiscall*)(void*, decltype(argList)...);
		return (*static_cast<VirtualFn**>(thisptr))[nIndex](thisptr, argList...);
	}

	template <typename T = void*>
	inline constexpr T GetVFunc(void* thisptr, std::size_t nIndex) {

		return (*static_cast<T**>(thisptr))[nIndex];
	}

	inline std::uintptr_t GetAbsoluteAddress(const std::uintptr_t uRelativeAddress) {

		return uRelativeAddress + 0x4 + *reinterpret_cast<std::int32_t*>(uRelativeAddress);
	}

	inline static uintptr_t FindSignature(const char* szModule, const char* szSignature) {

		const char* pat = szSignature;
		DWORD firstMatch = 0;
		DWORD rangeStart = (DWORD)GetModuleHandleA(szModule);
		MODULEINFO miModInfo;
		GetModuleInformation(GetCurrentProcess(), (HMODULE)rangeStart, &miModInfo, sizeof(MODULEINFO));
		DWORD rangeEnd = rangeStart + miModInfo.SizeOfImage;

		for (DWORD pCur = rangeStart; pCur < rangeEnd; pCur++) {

		if (!*pat)
				return firstMatch;

			if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == GET_BYTE(pat)) {

				if (!firstMatch)
					firstMatch = pCur;

				if (!pat[2])
					return firstMatch;

				if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
					pat += 3;

				else
					pat += 2;
			}
			else {

				pat = szSignature;
				firstMatch = 0;
			}
		}
		return 0u;
	}

	inline BYTE* ReadWavFileIntoMemory( std::string fname ) {
		BYTE* pb = nullptr;
		std::ifstream f( fname, std::ios::binary );

		f.seekg( 0, std::ios::end );
		size_t lim = f.tellg( );

		pb = new BYTE[ lim ];
		f.seekg( 0, std::ios::beg );
		f.read( ( char* )pb, lim );
		f.close( );

		return pb;
	}

	inline DWORD FindHudElement(const char* name) {

		static auto pThis = *reinterpret_cast<DWORD**>(util::FindSignature("client.dll", "B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08") + 1);

		static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(util::FindSignature("client.dll", "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));

		return find_hud_element(pThis, name);
	}

	inline void SetClan(const char* csTag) {

		static auto pSetClantag = reinterpret_cast<void(__fastcall*)(const char*, const char*)>(util::FindSignature("engine.dll", "53 56 57 8B DA 8B F9 FF"));
		pSetClantag(csTag, csTag);
	}

	inline void SetSkybox(const char* szSkybox) {

		static auto LoadNamedSky = reinterpret_cast<void(__fastcall*)(const char*)>(util::FindSignature("engine.dll", "55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45"));
		
		if (LoadNamedSky != nullptr)
			LoadNamedSky(szSkybox);
	}
}