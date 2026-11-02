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

	// Win32 keyboard messages normally carry VK_SHIFT/VK_CONTROL/VK_MENU,
	// while the binding widget records their left/right virtual-key codes.
	int modifier = 0, leftModifier = 0, rightModifier = 0;
	const auto keyData = static_cast<std::uintptr_t>(lParam);
	switch (nKey) {
	case VK_SHIFT: case VK_LSHIFT: case VK_RSHIFT:
		modifier = VK_SHIFT; leftModifier = VK_LSHIFT; rightModifier = VK_RSHIFT;
		if (nKey == VK_SHIFT) nKey = ((keyData >> 16) & 0xffU) == 0x36U ? VK_RSHIFT : VK_LSHIFT;
		break;
	case VK_CONTROL: case VK_LCONTROL: case VK_RCONTROL:
		modifier = VK_CONTROL; leftModifier = VK_LCONTROL; rightModifier = VK_RCONTROL;
		if (nKey == VK_CONTROL) nKey = (keyData & (1U << 24)) ? VK_RCONTROL : VK_LCONTROL;
		break;
	case VK_MENU: case VK_LMENU: case VK_RMENU:
		modifier = VK_MENU; leftModifier = VK_LMENU; rightModifier = VK_RMENU;
		if (nKey == VK_MENU) nKey = (keyData & (1U << 24)) ? VK_RMENU : VK_LMENU;
		break;
	}
	if (nKey <= 0 || nKey >= arrKeyState.size())
		return false;
	// Ignore presses used to edit bindings, but always process releases.
	if (menu::open && nKey != VK_INSERT && nKey != VK_DELETE && state == EKeyState::DOWN)
		return false;
	if (state == EKeyState::DOWN && (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) && (lParam & (1LL << 30)))
		return false;
	const auto update = [&](int key, EKeyState next) {
		if (next == EKeyState::DOWN && arrKeyState[key] != EKeyState::DOWN && cfg::m_iKeyStates[key] == TOGGLE)
			arrKeyBooleans[key] = !arrKeyBooleans[key];
		arrKeyState[key] = next == EKeyState::UP && arrKeyState[key] == EKeyState::DOWN ? EKeyState::RELEASED : next;
	};
	update(nKey, state);
	// Generic Shift/Ctrl/Alt bindings remain held until both sides are up.
	if (modifier) update(modifier, arrKeyState[leftModifier] == EKeyState::DOWN || arrKeyState[rightModifier] == EKeyState::DOWN
		? EKeyState::DOWN : EKeyState::UP);

	return true;
}
