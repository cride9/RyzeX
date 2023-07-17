#include "../hooks.h"
#include "../../SDK/Menu/gui.h"

void __fastcall h::hkLockCursor(ISurface* thisptr, int edx) {

	static auto original = detour::lockCursor.GetOriginal<decltype(&h::hkLockCursor)>();

	if (menu::open) {

		i::Surface->UnLockCursor();
		return;
	}

	//invokeFastcall<void>(adr(thisptr), edx, adr(original), ROP::ClientGadget_t::uReturnGadget);
	original(thisptr, edx);
}