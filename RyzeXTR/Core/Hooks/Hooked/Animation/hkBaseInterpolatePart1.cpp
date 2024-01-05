#include "../../hooks.h"
#include "../../../Features/Rage/Animations/Lagcompensation.h"
#include "../../../Features/Rage/exploits.h"

enum
{
	INTERPOLATE_STOP = 0,
	INTERPOLATE_CONTINUE
};

int __fastcall h::hkBaseInterpolatePart1(CBaseEntity* pEntity, void* edx, float& currentTime, Vector& oldOrigin, Vector& oldAngles, int& bNoMoreChanges) {

	static auto original = detour::baseInterpolatePart1.GetOriginal<decltype(&hkBaseInterpolatePart1)>();

	if ( exploits::bIsCurrentlyCharging )
		return INTERPOLATE_STOP;

	return original(pEntity, edx, currentTime, oldOrigin, oldAngles, bNoMoreChanges);
}