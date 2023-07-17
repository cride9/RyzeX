#include "../hooks.h"
#include "../../Features/Misc/enginepred.h"

void __fastcall h::hkPlayerMove(void* ecx, void* edx) {

	static auto original = detour::playerMove.GetOriginal<decltype(&h::hkPlayerMove)>();

	//invokeFastcall<void>(adr(ecx), adr(edx), adr(original), ROP::ClientGadget_t::uReturnGadget);
	original(ecx, edx);
}