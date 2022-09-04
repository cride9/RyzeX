#include "../../hooks.h"
#include "../../../SDK/Entity.h"
#include "../../../globals.h"
#include "../../../Features/Rage/Animations/LocalAnimation.h"
#include "../../../Features/Rage/Animations/lagcompensation.h"
#include "../../../Features/Rage/antiaim.h"
#include "../../../Features/Misc/enginepred.h"

void __fastcall	h::hkUpdateClientSideAnimations(void* entityPointer, void* edx) {

	static auto original = detour::clientAnimation.GetOriginal<decltype(&h::hkUpdateClientSideAnimations)>();

	const auto pEnt = reinterpret_cast<CBaseEntity*>(entityPointer);

	if (!pEnt || !pEnt->AnimState())
		return original(entityPointer, edx);

	if (pEnt == g::pLocal) {

		static float UpdateTime = 0.f;

		if (localanim.update) {

			pEnt->GetAnimationLayers(localanim.localdata.AnimationLayer);

			pEnt->AnimState()->Update(g::pCmd->angViewPoint);
			original(entityPointer, edx);

			if (!i::ClientState->nChokedCommands) {

				localanim.localdata.flGoalFeetYaw = pEnt->AnimState()->flGoalFeetYaw;
				pEnt->GetPoseParameters(localanim.localdata.flPoseParameters);
			}
			localanim.update = false;
		}
		pEnt->SetAbsAngles(Vector(0.f, localanim.localdata.flGoalFeetYaw, 0.f));
		pEnt->SetPoseParameters(localanim.localdata.flPoseParameters);
		pEnt->SetAnimationLayers(localanim.localdata.AnimationLayer);

		if (!i::ClientState->nChokedCommands)
			pEnt->SetupBonesFix(localanim.localdata.Matrix);

		memcpy(pEnt->GetCachedBoneData().Base(), localanim.localdata.Matrix, pEnt->GetCachedBoneData().Count() * sizeof(matrix3x4_t));

		return;
		
	}

	original(entityPointer, edx);
}