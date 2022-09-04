#include "../../hooks.h"
#include "../../../SDK/Entity.h"
#include "../../../Features/Rage/Animations/LocalAnimation.h"

void __vectorcall h::hkUpdateAnimationState(void* animstatePointer, void* edx, float z, float y, float x, void* unknown1) {

	static auto original = detour::animationState.GetOriginal<decltype(&h::hkUpdateAnimationState)>();

	const auto pAnimstate = reinterpret_cast<CAnimState*>(animstatePointer);

	if (pAnimstate->iLastUpdateFrame == i::GlobalVars->iFrameCount)
		pAnimstate->iLastUpdateFrame--;

	const auto pEnt = pAnimstate->pEntity;

	if (pEnt != g::pLocal)
		return original(animstatePointer, edx, z, y, x, unknown1);

	/* z angle formula = viewangle.y + roll value */
	/* roll works more like goalfeetyaw, changing it to a static value like 0.f will CAUSE ROLL not fix it */
	return original(animstatePointer, edx, z, localanim.localdata.vecViewAngle.y, localanim.localdata.vecViewAngle.x, unknown1);
}