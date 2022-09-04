#include "LocalAnimation.h"
#include "../../../SDK/math.h"

void localanimation::AnimlayerFix(CUserCmd* pCmd, CAnimState* pState) {

	CAnimationLayer& landLayer = g::pLocal->GetAnimationOverlays()[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL];
	CAnimationLayer& jumpLayer = g::pLocal->GetAnimationOverlays()[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB];
	CAnimationLayer& moveLayer = g::pLocal->GetAnimationOverlays()[ANIMATION_LAYER_MOVEMENT_MOVE];

	if (!(g::pLocal->GetFlags() & 0x40)) {

		const auto bCrouched = g::pLocal->GetDuckAmount() > .55f;
		const auto bMoving = g::pLocal->GetVelocity().Length2D() >= 0.25f;
		const auto iPreviousFlags = g::predicted::nFlags;
		const auto bWasOnGround = iPreviousFlags & FL_ONGROUND;

		if (g::pLocal->GetMoveType() != MOVETYPE_LADDER) {

			const auto bOnGround = g::pLocal->GetFlags() & FL_ONGROUND;

			if (bWasOnGround && !bOnGround) {

				if (pCmd->iButtons & IN_JUMP) {

					int iSequence = bCrouched ? bMoving ? 18 : 17 : bMoving ? 16 : 15;

					landLayer.flPlaybackRate = g::pLocal->GetLayerSequenceCycleRate(&landLayer, iSequence);
					landLayer.nSequence = iSequence;
					landLayer.flCycle = landLayer.flWeight = 0.f;

					flLastLayerFix = i::GlobalVars->flRealTime;
					g::pLocal->GetAnimationOverlays()[4] = landLayer;
				}
				else {

					static int iSequence = 14;

					landLayer.flPlaybackRate = g::pLocal->GetLayerSequenceCycleRate(&landLayer, iSequence);
					landLayer.nSequence = iSequence;
					landLayer.flCycle = landLayer.flWeight = 0.f;

					flLastLayerFix = i::GlobalVars->flRealTime;
					g::pLocal->GetAnimationOverlays()[4] = landLayer;
				}
			}
			else if (bOnGround) {

				if (!bWasOnGround && !pState->bHitGroundAnimation) {

					auto iSequence = bMoving ? 22 : 20;

					if (bCrouched)
						iSequence = bMoving ? 19 : 21;

					if (pState->flDurationInAir > 1)
						iSequence = bCrouched ? 24 : 23;

					jumpLayer.flPlaybackRate = g::pLocal->GetLayerSequenceCycleRate(&jumpLayer, iSequence);
					jumpLayer.nSequence = iSequence;
					jumpLayer.flCycle = jumpLayer.flWeight = 0.f;

					flLastLayerFix = i::GlobalVars->flRealTime;
					g::pLocal->GetAnimationOverlays()[5] = jumpLayer;
				}
			}

			if (!(!bWasOnGround && bOnGround) && !(pCmd->iButtons & IN_JUMP)) {

				flLastLayerFix = i::GlobalVars->flRealTime;
				jumpLayer.flWeight = 0;
				g::pLocal->GetAnimationOverlays()[5] = jumpLayer;
			}
		}
		else {

			bool bWasOnLadder = false;
			if (g::predicted::nMoveType != MOVETYPE_LADDER || (bWasOnLadder = true, iPreviousFlags & FL_ONGROUND))
				bWasOnLadder = false;

			if (!bWasOnLadder && !(g::pLocal->GetFlags() & FL_ONGROUND)) {

				jumpLayer.flPlaybackRate = g::pLocal->GetLayerSequenceCycleRate(&jumpLayer, 13);
				jumpLayer.nSequence = 13;
				jumpLayer.flCycle = jumpLayer.flWeight = 0.f;

				flLastLayerFix = i::GlobalVars->flRealTime;
				g::pLocal->GetAnimationOverlays()[5] = jumpLayer;
			}
		}
	}
}

void localanimation::SetLayerSequence(int layer, int sequence) {

	if (sequence >= 2) {

		CAnimationLayer* pLayer = &g::pLocal->GetAnimationOverlays()[layer];

		if (pLayer) {
			SetSequence(pLayer, sequence);
			pLayer->flPlaybackRate = g::pLocal->GetLayerSequenceCycleRate(pLayer, sequence);
			SetCycle(pLayer, 0.f);
			SetWeight(pLayer, 0.f);
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