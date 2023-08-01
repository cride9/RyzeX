#include "../hooks.h"
#include "../../globals.h"
#include "../../Features/Visuals/chams.h"
#include "../../Features/Misc/misc.h"

void __fastcall h::hkDrawModel(IStudioRender* thisptr, int edx, DrawModelResults_t* pResults, const DrawModelInfo_t& info, matrix3x4_t* pBoneToWorld, float* flFlexWeights, float* flFlexDelayedWeights, const Vector& vecModelOrigin, int nFlags) {

	static auto original = detour::drawModel.GetOriginal<decltype(&h::hkDrawModel)>();

	if (i::ClientState->iSignonState != SIGNONSTATE_FULL)
		return original(thisptr, edx, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);

	bool bClearOverride = false;

	if (g::pLocal) {

		misc::RemoveSmoke();
		bClearOverride = chams::DrawChams(g::pLocal, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
	}

	//detour::drawModel.CallOriginal<void>(ROP::ClientGadget_t::uReturnGadget, (void*)thisptr, edx, pResults, &info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, &vecModelOrigin, nFlags);
	original(thisptr, edx, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);

	if (bClearOverride)
		i::StudioRender->ForcedMaterialOverride(nullptr);
}