#include "../../hooks.h"
#include "../../../SDK/Entity.h"
#include "../../../globals.h"

void __fastcall h::hkCalculateView(void* entityPointer, void* edx, Vector& eyeOrigin, Vector& eyeAngles, float& zNear, float& zFar, float& fov) {

	static auto original = detour::calculateView.GetOriginal<decltype(&h::hkCalculateView)>();

	const auto pEnt = reinterpret_cast<CBaseEntity*>(entityPointer);

	if (pEnt != g::pLocal)
		return detour::calculateView.CallOriginal<void>(ROP::ClientGadget_t::uReturnGadget, entityPointer, edx, &eyeOrigin, &eyeAngles, &zNear, &zFar, &fov);

	if (!pEnt->IsAlive())
		return detour::calculateView.CallOriginal<void>(ROP::ClientGadget_t::uReturnGadget, entityPointer, edx, &eyeOrigin, &eyeAngles, &zNear, &zFar, &fov);

	const auto backup = pEnt->UseNewAnimationState();

	pEnt->UseNewAnimationState() = false;

	detour::calculateView.CallOriginal<void>(ROP::ClientGadget_t::uReturnGadget, entityPointer, edx, &eyeOrigin, &eyeAngles, &zNear, &zFar, &fov);

	pEnt->UseNewAnimationState() = backup;
}