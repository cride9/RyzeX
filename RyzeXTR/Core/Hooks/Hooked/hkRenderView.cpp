#include "../hooks.h"
#include "../../SDK/Menu/config.h"

void __fastcall h::hkRenderView(void* ecx, void* edx, CViewSetup& view, CViewSetup& hudViewSetup, int nClearFlags, int whatToDraw) {

	static auto original = detour::renderView.GetOriginal<decltype(&h::hkRenderView)>();

	if (cfg::misc::drawViewmodelOnScope)
		whatToDraw |= 1 << 0/*RENDERVIEW_DRAWVIEWMODEL*/;

	bool bBackup = i::Input->bCameraInThirdPerson;

	if (cfg::misc::thirdpersonDistance == 0) {
		i::Input->bCameraInThirdPerson = false;
		invokeFastcall<void>(adr(ecx), adr(edx), adr(original), ROP::ClientGadget_t::uReturnGadget, &view, &hudViewSetup, nClearFlags, whatToDraw);
		//original(ecx, edx, view, hudViewSetup, nClearFlags, whatToDraw);
		i::Input->bCameraInThirdPerson = bBackup;
	}
	else
		invokeFastcall<void>(adr(ecx), adr(edx), adr(original), ROP::ClientGadget_t::uReturnGadget, &view, &hudViewSetup, nClearFlags, whatToDraw);
}