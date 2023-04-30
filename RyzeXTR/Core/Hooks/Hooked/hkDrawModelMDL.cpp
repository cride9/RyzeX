#include "../hooks.h"
#include "../../Features/Visuals/chams.h"
using namespace cfg::model;

static void BeginChams(IMaterial* pMaterial, float const* flColor, bool bIgnoreZ, bool bWireFrame) {

	if (pMaterial == nullptr)
		return;

	i::StudioRender->SetColorModulation(flColor, pMaterial);
	i::StudioRender->SetAlphaModulation(flColor[3]);

	pMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, bIgnoreZ);
	pMaterial->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, bWireFrame);

	i::ModelRender->ForcedMaterialOverride(pMaterial);
}

static void EndChams() {

	float reset[3] = { 1, 1, 1 };

	i::StudioRender->SetColorModulation(reset);
	i::StudioRender->SetAlphaModulation(1.f);
	i::ModelRender->ForcedMaterialOverride(nullptr);
}

void __fastcall h::hkDrawModelMDL(void* ecx, int edx, IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bonetoworld) {

	static auto original = detour::drawModelMdl.GetOriginal<decltype(&h::hkDrawModelMDL)>();

	if (i::ModelRender->IsForcedMaterialOverride())
		return original(ecx, edx, ctx, state, info, bonetoworld);

	for (size_t i = 0; i < MATERIAL_MAX; i++) 
		if (!chams::materials[i])
			return original(ecx, edx, ctx, state, info, bonetoworld);

	const std::string_view& szModelName = info.pModel->szName;

	// attachment chams on local LMDLMDXL
	if ((szModelName.find("weapons") != std::string_view::npos) && (info.vecOrigin - g::pLocal->GetVecOrigin()).Length2D() < 20.f) {

		if (weapon) {
			BeginChams(chams::materials[weaponType], weaponColor, false, weaponXhair);
			original(ecx, edx, ctx, state, info, bonetoworld);
		}
		else {
			EndChams();
			original(ecx, edx, ctx, state, info, bonetoworld);
		}
		if (weaponOverlay) {
			BeginChams(chams::materials[GLOW], weaponOverlayColor, false, weaponOverlayXhair);
			original(ecx, edx, ctx, state, info, bonetoworld);
		}
		if (weaponThinOverlay) {
			BeginChams(chams::materials[THINGLOW], weaponThinOverlayColor, false, weaponThinOverlayXhair);
			original(ecx, edx, ctx, state, info, bonetoworld);
		}
		if (weaponAnimOverlay) {
			BeginChams(chams::materials[ANIMATED], weaponAnimOverlayColor, false, weaponAnimOverlayXhair);
			original(ecx, edx, ctx, state, info, bonetoworld);
		}
	}
	else
		return original(ecx, edx, ctx, state, info, bonetoworld);
	
	i::ModelRender->ForcedMaterialOverride(nullptr);
}