#include "../../hooks.h"

void __fastcall h::hkProcessMovement(void* ecx, void* edx, CBaseEntity* pEntity, CMoveData* pMove) {

	static auto original = detour::processMovement.GetOriginal<decltype(&h::hkProcessMovement)>();

	pMove->bGameCodeMovedPlayer = false;
	invokeFastcall<void>(adr(ecx), adr(edx), adr(original), ROP::ClientGadget_t::uReturnGadget, pEntity, pMove);
	//original(ecx, edx, pEntity, pMove);
}
