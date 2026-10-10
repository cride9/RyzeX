#include "InputSystem.h"
// used: std::this_thread
#include <thread>
// used: wndproc hook, inputsystem interface
#include "../Hooks/hooks.h"
// used: menu open state
#include "../SDK/Menu/gui.h"

#include <chrono>
using namespace std::chrono_literals;

bool IPT::Setup( )
{
	D3DDEVICE_CREATION_PARAMETERS creationParameters = { };
	while ( FAILED( i::DirectDevice->GetCreationParameters( &creationParameters ) ) )
		std::this_thread::sleep_for( 200ms );

	hWindow = creationParameters.hFocusWindow;

	if ( hWindow == nullptr )
		return false;
	focused = GetForegroundWindow() == hWindow;

	pOldWndProc = reinterpret_cast< WNDPROC >( SetWindowLongPtrW( hWindow, GWLP_WNDPROC, reinterpret_cast< LONG_PTR >( h::hkWndProc ) ) );

	if ( pOldWndProc == nullptr )
		return false;

	util::Print("Input system initialized");
	return true;
}

void IPT::Restore( )
{
	if ( pOldWndProc != nullptr )
	{
		SetWindowLongPtrW( hWindow, GWLP_WNDPROC, reinterpret_cast< LONG_PTR >( pOldWndProc ) );
		pOldWndProc = nullptr;
	}

	// reset input state
	i::InputSystem->EnableInput( true );
	ResetPressed();
}

void IPT::ResetPressed() {
	std::lock_guard<std::mutex> lock(keyMutex);
	arrKeyState.fill(EKeyState::UP);
}

bool IPT::Process( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	std::lock_guard<std::mutex> lock(keyMutex);
	if (uMsg == WM_KILLFOCUS || (uMsg == WM_ACTIVATEAPP && !wParam)) {
		focused = false;
		arrKeyState.fill(EKeyState::UP);
		return false;
	}
	if (uMsg == WM_SETFOCUS || (uMsg == WM_ACTIVATEAPP && wParam)) {
		focused = true;
		arrKeyState.fill(EKeyState::UP);
		return false;
	}

	// current active key
	int nKey = 0;
	// current active key state
	EKeyState state = EKeyState::NONE;

	switch ( uMsg )
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if ( wParam < 256U )
		{
			nKey = wParam;
			state = EKeyState::DOWN;
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if ( wParam < 256U )
		{
			nKey = wParam;
			state = EKeyState::UP;
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
		nKey = VK_LBUTTON;
		state = uMsg == WM_LBUTTONUP ? EKeyState::UP : EKeyState::DOWN;
		break;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
		nKey = VK_RBUTTON;
		state = uMsg == WM_RBUTTONUP ? EKeyState::UP : EKeyState::DOWN;
		break;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
		nKey = VK_MBUTTON;
		state = uMsg == WM_MBUTTONUP ? EKeyState::UP : EKeyState::DOWN;
		break;
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_XBUTTONDBLCLK:
		nKey = ( GET_XBUTTON_WPARAM( wParam ) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2 );
		state = uMsg == WM_XBUTTONUP ? EKeyState::UP : EKeyState::DOWN;
		break;
	default:
		return false;
	}

	if (nKey <= 0 || nKey >= arrKeyState.size())
		return false;
	// Ignore presses used to edit bindings, but always process releases.
	if (menu::open && nKey != VK_INSERT && nKey != VK_DELETE && state == EKeyState::DOWN)
		return false;
	if (state == EKeyState::DOWN && (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) && (lParam & (1LL << 30)))
		return false;
	if (state == EKeyState::DOWN && arrKeyState[nKey] != EKeyState::DOWN && cfg::m_iKeyStates[nKey] == TOGGLE)
		arrKeyBooleans[nKey] = !arrKeyBooleans[nKey];
	// A press toggles once; every consumer reads that same state.
	if ( state == EKeyState::UP && arrKeyState.at( nKey ) == EKeyState::DOWN ) // if swap states it will be pressed state
		arrKeyState.at( nKey ) = EKeyState::RELEASED;
	else
		arrKeyState.at( nKey ) = state;

	return true;
}
