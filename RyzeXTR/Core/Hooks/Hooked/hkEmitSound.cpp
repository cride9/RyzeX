#include "../hooks.h"
#include "../../SDK/Menu/config.h"
#include "../../Features/Rage/Animations/Lagcompensation.h"
#include "../../Features/Visuals/ESP.h"

void __fastcall h::hkEmitSound(
	void* _this, 
	int edx, 
	IRecipientFilter& filter,
	int iEntIndex, 
	int iChannel, 
	const char* pSoundEntry, 
	HSOUNDSCRIPTHASH nSoundEntryHash,
	const char* pSample, 
	float flVolume, 
	int nSeed, 
	float flAttenuation, 
	int iFlags, 
	int iPitch, 
	const Vector* pOrigin, 
	const Vector* pDirection, 
	CUtlVector<Vector>* pUtlVecOrigins, 
	bool bUpdatePositions, 
	float soundtime, 
	int speakerentity, 
	int unk) {

	static auto original = detour::emitSound.GetOriginal<decltype(&h::hkEmitSound)>();

	original(_this, edx, filter, iEntIndex, iChannel, pSoundEntry, nSoundEntryHash, pSample, flVolume, nSeed, flAttenuation, iFlags, iPitch, pOrigin, pDirection, pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity, unk);

	if (iEntIndex < 65 && iEntIndex > 0) {
		CBaseEntity* pSoundEntity = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(iEntIndex));
			
		if (!pSoundEntity || !g::pLocal)
			return;
		if (pOrigin != nullptr) {
			visual::vecDormatPosition[iEntIndex] = (*pOrigin) - Vector(0.f, 0.f, ((*pOrigin).z - pSoundEntity->vecMins().z));
		}
	}
}