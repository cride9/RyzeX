#include "../hooks.h"
#include "../../globals.h"
#include "../../Features/Visuals/chams.h"

void __fastcall h::hkDrawModel(IStudioRender* thisptr, int edx, DrawModelResults_t* pResults, const DrawModelInfo_t& info, matrix3x4_t* pBoneToWorld, float* flFlexWeights, float* flFlexDelayedWeights, const Vector& vecModelOrigin, int nFlags) {

	static auto original = detour::drawModel.GetOriginal<decltype(&h::hkDrawModel)>();

	if (!i::EngineClient->IsInGame())
		return original(thisptr, edx, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);

	bool bClearOverride = false;

	if (g::pLocal) {

		bClearOverride = chams::DrawChams(g::pLocal, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
	
		// need menu element
		if (cfg::misc::removals[0]) {

			static auto linegoesthrusmoke = util::FindSignature("client.dll", "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0");

			static std::vector<const char*> vistasmoke_wireframe =
			{
				"particle/vistasmokev1/vistasmokev1_smokegrenade",
			};

			static std::vector<const char*> vistasmoke_nodraw =
			{
				"particle/vistasmokev1/vistasmokev1_fire",
				"particle/vistasmokev1/vistasmokev1_emods",
				"particle/vistasmokev1/vistasmokev1_emods_impactdust",
			};

			for (auto mat_s : vistasmoke_wireframe) {

				IMaterial* mat = i::MaterialSystem->FindMaterial(mat_s, "Other textures");
				mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true); //wireframe
			}

			for (auto mat_n : vistasmoke_nodraw) {

				IMaterial* mat = i::MaterialSystem->FindMaterial(mat_n, "Other textures");
				mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
			}

			static auto smokecout = *(DWORD*)(linegoesthrusmoke + 0x8);
			*(int*)(smokecout) = 0;
		}
	}

	original(thisptr, edx, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);

	if (bClearOverride)
		i::StudioRender->ForcedMaterialOverride(nullptr);
}