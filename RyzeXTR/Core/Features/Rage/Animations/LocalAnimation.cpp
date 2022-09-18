#include "LocalAnimation.h"
#include "../../../SDK/math.h"

void localanimation::AnimlayerFix(CUserCmd* pCmd, CAnimState* pState) {

	if (!g::pLocal || !g::pLocal->GetHealth())
		return;

	auto pLocal = g::pLocal;

	if (pLocal->GetAnimationOverlays()) {

		CAnimationLayer& pLand = pLocal->GetAnimationOverlays()[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB];
		CAnimationLayer& pJumpFall = pLocal->GetAnimationOverlays()[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL];
		CAnimationLayer& pMoving = pLocal->GetAnimationOverlays()[ANIMATION_LAYER_MOVEMENT_MOVE];

		if (!(pLocal->GetFlags() & FL_FROZEN)) {

			const int iPrevious = g::localprediction::before::nFlags;

			const bool bCrouched = pLocal->GetDuckAmount() > .55f;
			const bool bMoving = pLocal->GetVelocity().Length2D() >= 0.25f;

			if (pLocal->GetMoveType() != MOVETYPE_LADDER) {

				const bool bOnGround = (pLocal->GetFlags() & FL_ONGROUND);
				const bool bWasOnGround = (iPrevious & FL_ONGROUND);

				if (bWasOnGround && !bOnGround) {

					if (pCmd->iButtons & IN_JUMP) {

						int iSeq = bMoving ? 16 : 15;

						if (bCrouched)
							iSeq = bMoving ? 18 : 17;

						pLand.flPlaybackRate = pLocal->GetLayerSequenceCycleRate(&pLand, iSeq);
						pLand.nSequence = iSeq;
						pLand.flCycle = pLand.flWeight = 0.f;
						localanim.localdata.AnimationLayer[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB] = pLand;
					}
					else {

						int iSeq = 14;

						pLand.flPlaybackRate = pLocal->GetLayerSequenceCycleRate(&pLand, iSeq);
						pLand.nSequence = iSeq;
						pLand.flCycle = pLand.flWeight = 0.f;
						localanim.localdata.AnimationLayer[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB] = pLand;
					}
				}
				else if (bOnGround) {

					if (!bWasOnGround && !pState->bHitGroundAnimation) {

						int iSeq = bMoving ? 22 : 20;

						if (bCrouched)
							iSeq = bMoving ? 19 : 21;

						if (pState->flDurationInAir > 1.f)
							iSeq = bMoving ? 14 : 23;

						pJumpFall.flPlaybackRate = pLocal->GetLayerSequenceCycleRate(&pJumpFall, iSeq);
						pJumpFall.nSequence = iSeq;
						pJumpFall.flCycle = pLand.flWeight = 0.f;
						localanim.localdata.AnimationLayer[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL] = pJumpFall;
					}
				}

				if (!(!bWasOnGround && bOnGround) && !(pCmd->iButtons & IN_JUMP)) {

					pJumpFall.flWeight = 0;
					localanim.localdata.AnimationLayer[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL] = pJumpFall;
				}
			}
			else {

				bool bWasOnLadder = false;
				if (g::localprediction::before::nMoveType != MOVETYPE_LADDER || (bWasOnLadder = true, iPrevious & FL_ONGROUND))
					bWasOnLadder = false;

				if (!bWasOnLadder && !(pLocal->GetFlags() & FL_ONGROUND))
				{
					pJumpFall.flPlaybackRate = pLocal->GetLayerSequenceCycleRate(&pJumpFall, 13);
					pJumpFall.nSequence = 13;
					pJumpFall.flCycle = pLand.flWeight = 0.f;
					localanim.localdata.AnimationLayer[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL] = pJumpFall;
				}
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