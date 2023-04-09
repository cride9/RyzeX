#include "../../hooks.h"
#include "../../../SDK/Entity.h"
#include "../../../globals.h"
#include "../../../Features/Rage/Animations/Lagcompensation.h"
#include "../../../Features/Rage/Animations/EnemyAnimations.h"

bool __fastcall h::hkSetupBones(void* ecx, void* edx, matrix3x4_t* matrix, int maxbones, int bonemask, float curtime) {

	static auto original = detour::setupBones.GetOriginal<decltype(&h::hkSetupBones)>();

	//static auto retAddress = util::FindSignature("client.dll", "FF 75 08 E8 ? ? ? ? 5F 5E 5D C2 10 00") + 8;

	//if (_ReturnAddress() == (void*)retAddress || _AddressOfReturnAddress() == (void*)retAddress)
	//	return false;

	const auto pEnt = reinterpret_cast<CBaseEntity*>((uintptr_t)ecx - 4);

	bool bResult = true;
	if ( /*pEnt != g::pLocal ||*/ !pEnt || !g::pLocal || !pEnt->IsAlive() || (pEnt->GetTeam() == g::pLocal->GetTeam() && pEnt != g::pLocal))
		return original(ecx, edx, matrix, maxbones, bonemask, curtime);

	if (g::bSettingUpBones[pEnt->EntIndex()]) {

		if (pEnt == g::pLocal)
			return original(ecx, edx, matrix, maxbones, bonemask, curtime);

		if (lagcomp.GetLog(pEnt->EntIndex()).pRecord.empty())
			return original(ecx, edx, matrix, maxbones, bonemask, curtime);

		auto m_Record = &lagcomp.GetLog(pEnt->EntIndex()).pRecord.front();

		static int nFlags = 4;

		/* Reset layers */
		std::memcpy(pEnt->GetAnimationOverlays(), m_Record->pLayers, sizeof(CAnimationLayer) * ANIMATION_LAYER_COUNT);

		// save globals
		std::tuple < float, float, float, float, float, int, int > m_Globals = std::make_tuple
		(
			// backup globals
			i::GlobalVars->flCurrentTime,
			i::GlobalVars->flRealTime,
			i::GlobalVars->flFrameTime,
			i::GlobalVars->flAbsFrameTime,
			i::GlobalVars->flInterpolationAmount,

			// backup frame count and tick count
			i::GlobalVars->iFrameCount,
			i::GlobalVars->iTickCount
		);

		/* Store player's data */
		std::tuple < int, int, int, int, int, Vector > m_PlayerData = std::make_tuple
		(
			pEnt->GetLastSkipFrameCount(),
			pEnt->GetEffects(),
			pEnt->GetClientEffects(),
			pEnt->GetOcclusionFrameCount(),
			pEnt->m_nOcclusionMask(),
			pEnt->GetAbsOrigin()
		);

		/* Force game's globals */
		int nSimulationTick = TIME_TO_TICKS(m_Record->flSimulationTime);
		i::GlobalVars->flCurrentTime = m_Record->flSimulationTime;
		i::GlobalVars->flRealTime = m_Record->flSimulationTime;
		i::GlobalVars->flFrameTime = i::GlobalVars->flIntervalPerTick;
		i::GlobalVars->flAbsFrameTime = i::GlobalVars->flIntervalPerTick;
		i::GlobalVars->iTickCount = nSimulationTick;
		i::GlobalVars->iFrameCount = INT_MAX; /* ShouldSkipAnimationFrame fix */
		i::GlobalVars->flInterpolationAmount = 0.0f;

		/* Force it https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/game/client/c_baseanimating.cpp#L3102 */
		pEnt->InvalidateBoneCache();

		/* Force the owner of animation layers */
		for (int iLayer = 0; iLayer < ANIMATION_LAYER_COUNT; iLayer++)
		{
			CAnimationLayer* m_Layer = &pEnt->GetAnimationOverlays()[iLayer];
			if (!m_Layer)
				continue;

			m_Layer->pOwner = pEnt;
		}

		/* Disable ACT_CSGO_IDLE_TURN_BALANCEADJUST animation */
		if (nFlags & 8)
		{
			pEnt->GetAnimationOverlays()[ANIMATION_LAYER_LEAN].flWeight = 0.0f;
			if (pEnt->GetSequenceActivity(pEnt->GetAnimationOverlays()[ANIMATION_LAYER_ADJUST].nSequence) == ACT_CSGO_IDLE_TURN_BALANCEADJUST)
			{
				pEnt->GetAnimationOverlays()[ANIMATION_LAYER_ADJUST].flCycle = 0.0f;
				pEnt->GetAnimationOverlays()[ANIMATION_LAYER_ADJUST].flWeight = 0.0f;
			}
		}

		/* Remove interpolation if required */
		if (!(nFlags & 2))
			pEnt->SetAbsOrigin(m_Record->vecOrigin);

		/* Compute bone mask */
		int nBoneMask = BONE_USED_BY_ANYTHING;
		//if (nFlags & 4)
		//	nBoneMask = BONE_USED_BY_HITBOX;

		/* Fix player's data */
		pEnt->GetClientEffects() |= 2;
		pEnt->GetEffects() |= EF_NOINTERP;
		pEnt->GetOcclusionFrameCount() = -1;
		pEnt->m_nOcclusionMask() &= ~2;
		pEnt->GetLastSkipFrameCount() = 0;

		/* Setup bones */
		bResult = original(ecx, edx, matrix, maxbones, nBoneMask, curtime);
		pEnt->SetupBones_AttachmentHelper();

		/* Restore player's data */
		pEnt->GetLastSkipFrameCount() = std::get < 0 >(m_PlayerData);
		pEnt->GetEffects() = std::get < 1 >(m_PlayerData);
		pEnt->GetClientEffects() = std::get < 2 >(m_PlayerData);
		pEnt->GetOcclusionFrameCount() = std::get < 3 >(m_PlayerData);
		pEnt->m_nOcclusionMask() = std::get < 4 >(m_PlayerData);
		pEnt->SetAbsOrigin(std::get < 5 >(m_PlayerData));

		/* Reset layers */
		std::memcpy(pEnt->GetAnimationOverlays(), m_Record->pLayers, sizeof(CAnimationLayer) * ANIMATION_LAYER_COUNT);

		// restore globals
		i::GlobalVars->flCurrentTime = std::get < 0 >(m_Globals);
		i::GlobalVars->flRealTime = std::get < 1 >(m_Globals);
		i::GlobalVars->flFrameTime = std::get < 2 >(m_Globals);
		i::GlobalVars->flAbsFrameTime = std::get < 3 >(m_Globals);
		i::GlobalVars->flInterpolationAmount = std::get < 4 >(m_Globals);

		// restore frame count and tick count
		i::GlobalVars->iFrameCount = std::get < 5 >(m_Globals);
		i::GlobalVars->iTickCount = std::get < 6 >(m_Globals);

		return bResult;
	}
	else if (matrix) {
		if (pEnt == g::pLocal) {
			bResult = g_LocalAnimations->CopyCachedMatrix(matrix, maxbones);
		}
		else {
			bResult = anims.CopyCachedMatrix(pEnt, matrix, maxbones);
		}
	}

	return bResult;
}