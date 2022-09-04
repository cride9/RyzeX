#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <cstddef>
#include <iostream>
#include "SDK/DataTyes/Color.h"

#define RYZEXCOLOR Color(222, 153, 42)
#define INRANGE(x,a,b)   (x >= a && x <= b)
#define GET_BYTE( x )    (GET_BITS(x[0]) << 4 | GET_BITS(x[1]))
#define GET_BITS( x )    (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))

namespace util {

	inline void Print(const char* csText) {
		// YES endl is slower than just a \n BUT IT ALSO CLEARS THE BUFFER (learn C ty)
		std::cout << csText << std::endl;
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
}