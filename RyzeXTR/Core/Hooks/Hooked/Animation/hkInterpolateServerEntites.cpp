#include "../../hooks.h"
#include "../../../SDK/Entity.h"

void __fastcall h::hkInterpolateServerEntites(void* ecx, void* edx) {

	static auto original = detour::interpolateEntites.GetOriginal<decltype(&h::hkInterpolateServerEntites)>();

	CBaseEntity* pEnt = reinterpret_cast<CBaseEntity*>(edx);

	if (pEnt && pEnt->IsPlayer())
		util::Print("edx is a player lmao");

	return;

	return original(ecx, edx);
}