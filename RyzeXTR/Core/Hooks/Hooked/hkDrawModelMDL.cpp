#include "../hooks.h"
#include "../../Features/Visuals/chams.h"
using namespace cfg::model;

void __fastcall h::hkDrawModelMDL(void* ecx, int edx, IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bonetoworld) {

	static auto original = detour::drawModelMdl.GetOriginal<decltype(&h::hkDrawModelMDL)>();

	return original(ecx, edx, ctx, state, info, bonetoworld);

	if (!g::pLocal)
		return original(ecx, edx, ctx, state, info, bonetoworld);

	bool bClearOverride = chams::DrawChamsMDL(ctx, state, info, bonetoworld);

	original(ecx, edx, ctx, state, info, bonetoworld);
	
	if (bClearOverride)
		i::ModelRender->ForcedMaterialOverride(nullptr);
	
}