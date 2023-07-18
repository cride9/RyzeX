#include "../../hooks.h"

void __fastcall h::hkProcessMovement(void* ecx, void* edx, CBaseEntity* pEntity, CMoveData* pMove) {

	static auto original = detour::processMovement.GetOriginal<decltype(&h::hkProcessMovement)>();

	pMove->bGameCodeMovedPlayer = false;
	detour::processMovement.CallOriginal<void>(ROP::ClientGadget_t::uReturnGadget, ecx, edx, pEntity, pMove);
}
