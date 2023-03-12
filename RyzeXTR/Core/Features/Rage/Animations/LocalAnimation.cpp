#include "LocalAnimation.h"
#include "../../Misc/enginepred.h"
#include "../../../SDK/math.h"

enum nSequenceStages {


};

void localanimation::AnimlayerFix(CUserCmd* pCmd, CAnimState* pState) {

	if (!g::pLocal || !g::pLocal->GetHealth())
		return;

	CAnimationLayer* pLandOrClimbLayer = &g::pLocal->GetAnimationOverlays()[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB];
	if (!pLandOrClimbLayer)
		return;

	CAnimationLayer* pJumpOrFallLayer = &g::pLocal->GetAnimationOverlays()[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL];
	if (!pJumpOrFallLayer)
		return;

	if (localanim.localdata.oldMoveType != MOVETYPE_LADDER && g::pLocal->GetMoveType() == MOVETYPE_LADDER)
		SetLayerSequence(pLandOrClimbLayer, ACT_CSGO_CLIMB_LADDER);
	else if (localanim.localdata.oldMoveType == MOVETYPE_LADDER && g::pLocal->GetMoveType() != MOVETYPE_LADDER)
		SetLayerSequence(pJumpOrFallLayer, ACT_CSGO_FALL);
	else
	{
		if (g::pLocal->GetFlags() & FL_ONGROUND)
		{
			if (!(localanim.localdata.oldFlags & FL_ONGROUND))
				SetLayerSequence
				(
					pLandOrClimbLayer,
					g::pLocal->AnimState()->flDurationInAir > 1.0f ? ACT_CSGO_LAND_HEAVY : ACT_CSGO_LAND_LIGHT
				);
		}
		else if (localanim.localdata.oldFlags & FL_ONGROUND)
		{
			if (g::pLocal->GetVelocity().z > 0.0f)
				SetLayerSequence(pJumpOrFallLayer, ACT_CSGO_JUMP);
			else
				SetLayerSequence(pJumpOrFallLayer, ACT_CSGO_FALL);
		}
	}

	localanim.localdata.oldFlags = g::pLocal->GetFlags();
	localanim.localdata.oldMoveType = g::pLocal->GetMoveType();

	//auto pLocal = g::pLocal;

	//if (pLocal->GetAnimationOverlays()) {

	//	CAnimationLayer& pLand = pLocal->GetAnimationOverlays()[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB];
	//	CAnimationLayer& pJumpFall = pLocal->GetAnimationOverlays()[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL];
	//	CAnimationLayer& pMoving = pLocal->GetAnimationOverlays()[ANIMATION_LAYER_MOVEMENT_MOVE];

	//	if (!(pLocal->GetFlags() & FL_FROZEN)) {

	//		const int iPrevious = g::localprediction::before::nFlags;

	//		const bool bCrouched = pLocal->GetDuckAmount() > .55f;
	//		const bool bMoving = pLocal->GetVelocity().Length2D() >= 0.25f;
	//		const bool bJumped = (!(pLocal->GetFlags() & FL_ONGROUND) && iPrevious & FL_ONGROUND);
	//		
	//		if (bJumped && (pLocal->GetMoveType() != MOVETYPE_LADDER)) {


	//		}
	//	}
	//}
}

void localanimation::SetLayerSequence(CAnimationLayer* layer, int sequence) {

	if (sequence >= 2) {

		if (layer) {
			SetSequence(layer, sequence);
			layer->flPlaybackRate = g::pLocal->GetLayerSequenceCycleRate(layer, sequence);
			SetCycle(layer, 0.f);
			SetWeight(layer, 0.f);
		}
	}
}

void localanimation::FixVelocityModifer(){

	static int iLastCmdAck = 0;
	static float flNextCmdTime = 0.f;

	if (g::pCmd && (iLastCmdAck != i::ClientState->iLastCommandAck || flNextCmdTime != i::ClientState->flNextCmdTime)) {

		if (localanim.localdata.flVelocityModifier != g::pLocal->GetOffset<float>(0xA38C)) {
			*reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(i::Prediction + 0x24)) = 1;
			localanim.localdata.flVelocityModifier = g::pLocal->GetOffset<float>(0xA38C);
		}
	}
}

void localanimation::UpdateLocal() {

	if (!g::pLocal || !g::pLocal->IsAlive() || !g::pLocal->AnimState() || i::ClientState->iDeltaTick < 0)
		return;

	/* Update only each tick */
	if (update) {

		//AnimlayerFix(g::pCmd, g::pLocal->AnimState());

		/* Store current animationlayers */
		g::pLocal->GetAnimationLayers(localdata.AnimationLayer);

		/* Allow client to animate local player */
		g::bAllowAnimations[g::pLocal->EntIndex()] = g::pLocal->IsClientSideAnimation() = true;

		/* Update animstate with current viewangles */
		g::pLocal->AnimState()->Update(g::pCmd->angViewPoint);
		g::pLocal->UpdateClientSideAnimations();

		/* Disallow client to animate local player */
		g::bAllowAnimations[g::pLocal->EntIndex()] = g::pLocal->IsClientSideAnimation() = false;

		/* Store networked data */
		if (!i::ClientState->nChokedCommands) {

			g::pLocal->GetPoseParameters(localdata.flPoseParameters);
			localdata.flGoalFeetYaw = g::pLocal->AnimState()->flGoalFeetYaw;
		}
		update = false;
	}
	/* Restore every frame with the updated ticks animation */
	g::pLocal->SetAnimationLayers(localdata.AnimationLayer);
	g::pLocal->SetPoseParameters(localdata.flPoseParameters);
	g::pLocal->SetAbsAngles(Vector(0.f, localdata.flGoalFeetYaw, 0.f));

	/* Only allow bone setup when we send packet */
	if (!i::ClientState->nChokedCommands)
		g::pLocal->SetupBonesFix(localdata.Matrix);

	/* Use latest setupbones */
	g::pLocal->SetBoneCache(localdata.Matrix);
}

void localanimation::SetSequence(CAnimationLayer* pLayer, int iSequence) {

	if (!pLayer->pOwner)
		return;

	if (pLayer->nSequence != iSequence)
		pLayer->pOwner->InvalidatePhysicsRecursive(BOUNDS_CHANGED);

	pLayer->nSequence = iSequence;
}

void localanimation::SetCycle(CAnimationLayer* pLayer, int iCycle) {

	if (!pLayer->pOwner)
		return;

	if (pLayer->flCycle != iCycle)
		pLayer->pOwner->InvalidatePhysicsRecursive(ANIMATION_CHANGED);

	pLayer->flCycle = iCycle;
}

void localanimation::SetOrder(CAnimationLayer* pLayer, int iOrder) {

	if (!pLayer->pOwner)
		return;

	if (pLayer->iOrder != iOrder)
		if (pLayer->iOrder == 13 || iOrder == 13)
		pLayer->pOwner->InvalidatePhysicsRecursive(BOUNDS_CHANGED);

	pLayer->iOrder = iOrder;
}

void localanimation::SetWeight(CAnimationLayer* pLayer, float flWeight) {

	if (!pLayer->pOwner)
		return;

	if (pLayer->flWeight == 0.f || flWeight == 0.f)
		pLayer->pOwner->InvalidatePhysicsRecursive(BOUNDS_CHANGED);

	pLayer->flWeight = flWeight;
}