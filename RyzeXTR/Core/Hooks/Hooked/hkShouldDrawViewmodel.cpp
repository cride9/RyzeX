#include "../hooks.h"
#include "../../SDK/Menu/config.h"

bool __fastcall h::hkShouldDrawViewmodel(void* ecx, void* edx) {

	static auto original = detour::drawViewmodel.GetOriginal<decltype(&h::hkShouldDrawViewmodel)>();

	return cfg::misc::drawViewmodelOnScope ? true : invokeFastcall<bool>(adr(ecx), adr(edx), adr(original), ROP::ClientGadget_t::uReturnGadget);
}
