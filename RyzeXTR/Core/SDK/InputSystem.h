#pragma once
// used: std::array
#include <array>
#include <Windows.h>
#include <mutex>

#include "../SDK/Menu/config.h"

enum class EKeyState : int
{
	NONE,
	DOWN,
	UP,
	RELEASED
};

enum EKeyInputType : int
{
	NO,
	HOLD,
	TOGGLE,
	ALWAYS_ON
};

/*
 * INPUT SYSTEM
 * listen and handle key states
 */
namespace IPT
{
	// Values
	/* current window */
	inline HWND	hWindow = nullptr;
	/* saved window messages handler */
	inline WNDPROC pOldWndProc = nullptr;
	/* last processed key states */
	inline std::array<EKeyState, 256U> arrKeyState = { };
	inline bool arrKeyBooleans[ 256 ];
	inline std::mutex keyMutex;
	inline bool focused = true;
	void ResetPressed();
	inline void SetKeyMode(std::uint32_t key, int mode) {
		if (key == 0 || key >= arrKeyState.size() || mode < NO || mode > ALWAYS_ON)
			return;
		std::lock_guard<std::mutex> lock(keyMutex);
		cfg::m_iKeyStates[key] = mode;
		arrKeyBooleans[key] = false;
	}

	// Get
	/* set our window messages proccesor */
	bool Setup( );
	/* restore window messages processor and clear saved pointer */
	void Restore( );
	/* process input window message and save keys states in array */
	bool Process( UINT uMsg, WPARAM wParam, LPARAM lParam );

	/* is given key being held */
	inline bool IsKeyDown( const std::uint32_t uButtonCode )
	{
		std::lock_guard<std::mutex> lock(keyMutex);
		return uButtonCode < arrKeyState.size() && arrKeyState[uButtonCode] == EKeyState::DOWN;
	}

	/* was given key released */
	inline bool IsKeyReleased( const std::uint32_t uButtonCode )
	{
		std::lock_guard<std::mutex> lock(keyMutex);
		if (uButtonCode < arrKeyState.size() && arrKeyState[uButtonCode] == EKeyState::RELEASED)
		{
			arrKeyState.at( uButtonCode ) = EKeyState::UP;
			return true;
		}

		return false;
	}

	inline bool HandleInput( const std::uint32_t uButtonCode )
	{
		if (uButtonCode == 0 || uButtonCode >= arrKeyState.size())
			return false;
		std::lock_guard<std::mutex> lock(keyMutex);

		if ( cfg::m_iKeyStates[ uButtonCode ] == HOLD ) {
			return focused && arrKeyState[uButtonCode] == EKeyState::DOWN;
		}
		else if ( cfg::m_iKeyStates[ uButtonCode ] == TOGGLE ) {

			return arrKeyBooleans[ uButtonCode ]; //GetKeyState(uButtonCode);
		}
		else if ( cfg::m_iKeyStates[ uButtonCode ] == ALWAYS_ON ) {
			return true;
		}

		return false;

	}
}
