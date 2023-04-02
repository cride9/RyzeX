#include "../hooks.h"
#include "../../SDK/Menu/config.h"

void __fastcall h::hkEmitSound(void* _this, int edx, void* filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char* pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk) {

	static auto original = detour::emitSound.GetOriginal<decltype(&h::hkEmitSound)>();

	if (strstr(pSoundEntry, "bulletimpact") && cfg::misc::bulletImpact) {
		i::DebugOverlay->AddBoxOverlay(
			Vector(pOrigin->x, pOrigin->y, pOrigin->z),
			Vector(-2.0f, -2.0f, -2.0f),
			Vector(2.0f, 2.0f, 2.0f),
			Vector(0.0f, 0.0f, 0.0f),
			0.f,
			0.f,
			255.f,
			155.f,
			4.f
		);
	}

	original(_this, edx, filter, iEntIndex, iChannel, pSoundEntry, nSoundEntryHash, pSample, flVolume, nSeed, flAttenuation, iFlags, iPitch, pOrigin, pDirection, pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity, unk);
}