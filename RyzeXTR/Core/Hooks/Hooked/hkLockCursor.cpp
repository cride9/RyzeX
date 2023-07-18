#include "../hooks.h"
#include "../../SDK/Menu/gui.h"

void __fastcall h::hkLockCursor(ISurface* thisptr, int edx) {

	static auto original = detour::lockCursor.GetOriginal<decltype(&h::hkLockCursor)>();

	if (menu::open) {

		i::Surface->UnLockCursor();
		return;
	}

	detour::lockCursor.CallOriginal<void>(ROP::ClientGadget_t::uReturnGadget, (void*)thisptr, edx);
}