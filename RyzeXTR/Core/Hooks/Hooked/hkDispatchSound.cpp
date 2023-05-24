#include "../hooks.h"
#include "../../Features/Visuals/ESP.h"
#include "../../Interface/Interfaces/IEngineSoundClient.h"

void __cdecl h::hkDispatchSound(SoundInfoSOURCESDK_t* info) {

	static auto original = detour::dispatchSound.GetOriginal<decltype(&hkDispatchSound)>();

 	if (info->vOrigin.IsZero())
		return original(info);

	if (info->nEntityIndex  < 1 || info->nEntityIndex  > i::GlobalVars->nMaxClients || info->nEntityIndex == i::EngineClient->GetLocalPlayer())
		return original(info);

	CBaseEntity* plr = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(info->nEntityIndex));
	if (!plr)
		return original(info);

	if (plr->IsDormant())
	{
		Vector pos = info->vOrigin;
		visual::vecDormatPosition[info->nEntityIndex] = pos;
	}

	return original(info);
}