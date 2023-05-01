#include "chams.h"
#include "../../Interface/Classes/CKeyValues.h"
#include "../Rage/Animations/LocalAnimation.h"
#include "../../SDK/math.h"
#include "../Misc/misc.h"
#include "../Rage/Animations/Lagcompensation.h"
#include "../Rage/ragebot.h"

constexpr std::string_view szScrollProxies = R"#(
	"texturescroll"
    {
        "texturescrollvar" "$basetexturetransform"
        "texturescrollrate" "0.4"
        "texturescrollangle" "90"
    }
)#";

IMaterial* RyzeCreateMaterial(std::string_view szName, std::string_view szShader, const char* szMaterial) {
	

	CKeyValues* pKeyValues = new CKeyValues(szShader.data());
	pKeyValues->LoadFromBuffer(szName.data(), szMaterial);

	IMaterial* pReturnValue = i::MaterialSystem->CreateMaterial(szName.data(), pKeyValues);

	return pReturnValue;
}

IMaterial* chams::CreateMaterial(std::string_view szName, std::string_view szShader, std::string_view szBaseTexture, std::string_view szEnvMap, bool bIgnorez, bool bWireframe, std::string_view szProxies)
{
	/*
	 * @note: materials info:
	 1	shaders: https://developer.valvesoftware.com/wiki/Category:Shaders
	 -		parameters: https://developer.valvesoftware.com/wiki/Category:List_of_Shader_Parameters
	 2	proxies: https://developer.valvesoftware.com/wiki/Material_proxies
	 -		list: https://developer.valvesoftware.com/wiki/List_Of_Material_Proxies
	 *
	 * use "mat_texture_list 1" command to see full materials list
	 */

	const std::string szMaterial = std::vformat(R"#("{0}"
	{{
		"$basetexture"		"{1}"
		"$envmap"			"{2}"
		"$envmapfresnel"	"0"
		"$model"			"1"
		"$translucent"		"0"
		"$ignorez"			"{3}"
		"$selfillum"		"1"
		"$halflambert"		"1"
		"$wireframe"		"{4}"
		"$nofog"			"1"
		"proxies"
		{{
			{5}
		}}
	}})#", std::make_format_args(szShader, szBaseTexture, szEnvMap, bIgnorez ? 1 : 0, bWireframe ? 1 : 0, szProxies));



	// load to memory
	CKeyValues* pKeyValues = new CKeyValues(szShader.data());
	pKeyValues->LoadFromBuffer(szName.data(), szMaterial.c_str());

	// create from buffer
	IMaterial* pReturn = i::MaterialSystem->CreateMaterial(szName.data(), pKeyValues);

	return pReturn;
}

void MatrixSetOrigin( Vector pos, matrix3x4_t& matrix )
{
	matrix[ 0 ][ 3 ] = pos.x;
	matrix[ 1 ][ 3 ] = pos.y;
	matrix[ 2 ][ 3 ] = pos.z;
}

bool GenerateLerpedMatrix(CBaseEntity* pEntity, matrix3x4_t* out) 
{
	if (!pEntity)
		return false;

	auto& pLog = lagcomp.GetLog(pEntity->EntIndex());
	if (!&pLog || !pLog.pRecord.size() || pLog.iLastValid + 1 >= pLog.pRecord.size())
		return false;

	const auto& FirstInvalid = &pLog.pRecord[pLog.iLastValid];
	const auto& LastInvalid = &pLog.pRecord[pLog.iLastValid + 1];

	if (FirstInvalid->bDormant)
		return false;

	if (LastInvalid->flSimulationTime - FirstInvalid->flSimulationTime > 0.5f)
		return false;

	if (!LastInvalid->bDidShot && (!LastInvalid->vecOrigin.DistTo(FirstInvalid->vecOrigin) || LastInvalid->flDuck == FirstInvalid->flDuck && LastInvalid->vecEyeAngles == FirstInvalid->vecEyeAngles && LastInvalid->vecOrigin == FirstInvalid->vecOrigin))
		return false;

	const auto NextOrigin = LastInvalid->vecOrigin;
	const auto curtime = i::GlobalVars->flCurrentTime;

	auto flDelta = 1.f - (curtime - LastInvalid->flInterpTime) / (LastInvalid->flSimulationTime - FirstInvalid->flSimulationTime);
	if (flDelta < 0.f || flDelta > 1.f)
		LastInvalid->flInterpTime = curtime;

	flDelta = 1.f - (curtime - LastInvalid->flInterpTime) / (LastInvalid->flSimulationTime - FirstInvalid->flSimulationTime);

	flDelta = std::clamp( flDelta, 0.f, 1.f );

	const auto lerp = M::Lerp(NextOrigin, FirstInvalid->vecOrigin, flDelta );

	matrix3x4_t ret[128];
	memcpy(ret, FirstInvalid->pMatrix, sizeof(matrix3x4_t[128]));

	for (size_t i{ }; i < 128; ++i) {
		const auto matrix_delta = Vector( FirstInvalid->pMatrix[ i ][ 0 ][ 3 ], FirstInvalid->pMatrix[ i ][ 1 ][ 3 ], FirstInvalid->pMatrix[ i ][ 2 ][ 3 ] ) - FirstInvalid->vecOrigin;
		MatrixSetOrigin(matrix_delta + lerp, ret[i]);
	}

	memcpy(out, ret, sizeof(matrix3x4_t[128]));
	return true;
}

static void BeginChams( IMaterial* pMaterial, float const* flColor, bool bIgnoreZ, bool bWireFrame ) {

	if (pMaterial == nullptr)
		return;

	i::StudioRender->SetColorModulation( flColor, pMaterial );
	i::StudioRender->SetAlphaModulation( flColor[ 3 ] );

	pMaterial->SetMaterialVarFlag( MATERIAL_VAR_IGNOREZ, bIgnoreZ );
	pMaterial->SetMaterialVarFlag( MATERIAL_VAR_WIREFRAME, bWireFrame );

	i::StudioRender->ForcedMaterialOverride( pMaterial );
}

static void EndChams( ) {

	static float reset[ 3 ] = { 1, 1, 1 };

	i::StudioRender->SetColorModulation( reset );
	i::StudioRender->SetAlphaModulation( 1.f );
	i::StudioRender->ForcedMaterialOverride( nullptr );
}

bool chams::DrawChams(CBaseEntity* pLocal, DrawModelResults_t* pResults, const DrawModelInfo_t& info, matrix3x4_t* pBoneToWorld, float* flFlexWeights, float* flFlexDelayedWeights, const Vector& vecModelOrigin, int nFlags) {

	static auto original = detour::drawModel.GetOriginal<decltype(&h::hkDrawModel)>();

	using namespace cfg::model;

	IClientRenderable* pRenderable = info.pClientEntity;

	if (!pRenderable)
		return false;

	CBaseEntity* pEnt = pRenderable->GetIClientUnknown()->GetBaseEntity();

	if (!pEnt || !g::pLocal)
		return false;

	const std::string_view& szModelName = info.pStudioHdr->szName;

	if (!materials[DEFAULT])
		materials[DEFAULT] = CreateMaterial("ryzextr_players", "VertexLitGeneric");
	if (!materials[FLAT])
		materials[FLAT] = CreateMaterial("ryzextr_playersflat", "UnlitGeneric");
	if (!materials[GLOW])
		materials[GLOW] = RyzeCreateMaterial("ryzextr_glow", "VertexLitGeneric", GlowChams);
	if (!materials[THINGLOW])
		materials[THINGLOW] = i::MaterialSystem->FindMaterial("dev/glow_armsrace", nullptr, true, nullptr);
	if (!materials[ANIMATED])
		materials[ANIMATED] = RyzeCreateMaterial("ryzextr_animated", "VertexLitGeneric", AnimatedChams);

	if (pEnt->IsPlayer() && pEnt->IsAlive()) {

		if (nFlags & (STUDIO_RENDER | STUDIO_SKIP_FLEXES | STUDIO_DONOTMODIFYSTENCILSTATE | STUDIO_NOLIGHTING_OR_CUBEMAP | STUDIO_SKIP_DECALS))
			return false;

		if (pEnt == g::pLocal) {

			matrix3x4_t* desyncMatrix = g_LocalAnimations->GetDesyncMatrix().data();

			if (localDesync) {

				if (!local) {
					EndChams();
					original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
				}

				BeginChams(materials[localDesyncType], localDesyncColor, false, localDesyncXhair);
				original(i::StudioRender, 0, pResults, info, desyncMatrix, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
			}
			if (cfg::antiaim::idealTick && GetAsyncKeyState(cfg::antiaim::idealTickBind)) {

				if (!local) {
					EndChams();
					original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
				}

				BeginChams(materials[ANIMATED], localIdealTickColor, false, true);
				original(i::StudioRender, 0, pResults, info, misc::matrixRecord, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
			}
			if (local) {

				if (localXQZ) {
					BeginChams(materials[localType], localXQZColor, true, localXQZXhair);
					original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
				}
				BeginChams(materials[localType], localColor, false, localXhair);
				original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
			}
			else {

				EndChams();
				original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
			}
			if (localOverlay) {

				if (localOverlayXQZ) {
					BeginChams(materials[GLOW], localOverlayXQZColor, true, localOverlayXQZXhair);
					original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
				}
				BeginChams(materials[GLOW], localOverlayColor, false, localOverlayXhair);
				original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
			}
			if (localThinOverlay) {

				if (localThinOverlayXQZ) {
					BeginChams(materials[THINGLOW], localThinOverlayXQZColor, true, localThinOverlayXQZXhair);
					original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
				}
				BeginChams(materials[THINGLOW], localThinOverlayColor, false, localThinOverlayXhair);
				original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
			}
			if (localAnimOverlay) {

				if (localAnimOverlayXQZ) {
					BeginChams(materials[ANIMATED], localAnimOverlayXQZColor, true, localAnimOverlayXQZXhair);
					original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
				}
				BeginChams(materials[ANIMATED], localAnimOverlayColor, false, localAnimOverlayXhair);
				original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
			}
			return true;
		}
		else if (pEnt->GetTeam() == g::pLocal->GetTeam()) {

			if (team) {

				if (teamXQZ) {
					BeginChams(materials[teamType], teamXQZColor, true, teamXQZXhair);
					original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
				}
				BeginChams(materials[teamType], teamColor, false, teamXhair);
				original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
			}
			else {

				EndChams();
				original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
			}
			if (teamOverlay) {

				if (teamOverlayXQZ) {
					BeginChams(materials[GLOW], teamOverlayXQZColor, true, teamOverlayXQZXhair);
					original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
				}
				BeginChams(materials[GLOW], teamOverlayColor, false, teamOverlayXhair);
				original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
			}
			if (teamThinOverlay) {

				if (teamThinOverlayXQZ) {
					BeginChams(materials[THINGLOW], teamThinOverlayXQZColor, true, teamThinOverlayXQZXhair);
					original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
				}
				BeginChams(materials[THINGLOW], teamThinOverlayColor, false, teamThinOverlayXhair);
				original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
			}
			if (teamAnimOverlay) {

				if (teamAnimOverlayXQZ) {
					BeginChams(materials[ANIMATED], teamAnimOverlayXQZColor, true, teamAnimOverlayXQZXhair);
					original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
				}
				BeginChams(materials[ANIMATED], teamAnimOverlayColor, false, teamAnimOverlayXhair);
				original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
			}
			return true;
		}
		else {

#if NO
			if (lagcomp.GetLog(pEnt->EntIndex()).pEntity) {

				if (lagcomp.GetLog(pEnt->EntIndex()).pRecord.size() >= 2) {

					auto lastrecord = lagcomp.GetLog(pEnt->EntIndex()).pRecord.front();

					BeginChams(materials[FLAT], teamAnimOverlayXQZColor, true, teamAnimOverlayXQZXhair);
					original(i::StudioRender, 0, pResults, info, lastrecord.pLeftMatrix, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);

					BeginChams(materials[FLAT], teamAnimOverlayColor, true, teamAnimOverlayXQZXhair);
					original(i::StudioRender, 0, pResults, info, lastrecord.pCenterMatrix, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);

					BeginChams(materials[FLAT], teamThinOverlayXQZColor, true, teamAnimOverlayXQZXhair);
					original(i::StudioRender, 0, pResults, info, lastrecord.pRightMatrix, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
				}
			}
#endif

			if (enemyBTEnable) {

				if (matrix3x4_t pMatrix[128];  GenerateLerpedMatrix(pEnt, pMatrix)) {

					BeginChams(materials[enemyBTType], enemyBTColor, true, enemyBTXhair);
					original(i::StudioRender, 0, pResults, info, pMatrix, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
				}
			}

			if (enemy) {

				if (enemyXQZ) {
					BeginChams(materials[enemyType], enemyXQZColor, true, enemyXQZXhair);
					original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
				}
				BeginChams(materials[enemyType], enemyColor, false, enemyXhair);
				original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
			}
			else {

				EndChams();
				original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
			}
			if (enemyOverlay) {

				if (enemyOverlayXQZ) {
					BeginChams(materials[GLOW], enemyOverlayXQZColor, true, enemyOverlayXQZXhair);
					original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
				}
				BeginChams(materials[GLOW], enemyOverlayColor, false, enemyOverlayXhair);
				original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
			}
			if (enemyThinOverlay) {

				if (enemyThinOverlayXQZ) {
					BeginChams(materials[THINGLOW], enemyThinOverlayXQZColor, true, enemyThinOverlayXQZXhair);
					original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
				}
				BeginChams(materials[THINGLOW], enemyThinOverlayColor, false, enemyThinOverlayXhair);
				original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
			}
			if (enemyAnimOverlay) {

				if (enemyAnimOverlayXQZ) {
					BeginChams(materials[ANIMATED], enemyAnimOverlayXQZColor, true, enemyAnimOverlayXQZXhair);
					original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
				}
				BeginChams(materials[ANIMATED], enemyAnimOverlayColor, false, enemyAnimOverlayXhair);
				original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
			}
			return true;
		}
	}
	//else if (szModelName.find("sleeve") != std::string_view::npos) {

	//	// get original sleeves material
	//	IMaterial* pSleeveMaterial = i::MaterialSystem->FindMaterial(szModelName.data(), TEXTURE_GROUP_MODEL);

	//	// check is valid material
	//	if (pSleeveMaterial == nullptr)
	//		return false;

	//}
	else if (szModelName.find("arms") != std::string_view::npos) {

		if (viewmodel) {
			BeginChams(materials[viewmodelType], viewmodelColor, false, viewmodelXhair);
			original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
		}
		else {
			EndChams();
			original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
		}
		if (viewmodelOverlay) {
			BeginChams(materials[GLOW], viewmodelOverlayColor, false, viewmodelOverlayXhair);
			original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
		}
		if (viewmodelThinOverlay) {
			BeginChams(materials[THINGLOW], viewmodelThinOverlayColor, false, viewmodelThinOverlayXhair);
			original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
		}
		if (viewmodelAnimOverlay) {
			BeginChams(materials[ANIMATED], viewmodelAnimOverlayColor, false, viewmodelAnimOverlayXhair);
			original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
		}
		return true;
	}
	else if ((szModelName.find("weapons\\v_") != std::string_view::npos)) {

		IMaterial* pViewModelMaterial = i::MaterialSystem->FindMaterial(szModelName.data(), TEXTURE_GROUP_MODEL);

		if (pViewModelMaterial == nullptr)
			return false;

		if (weapon) {
			BeginChams(materials[weaponType], weaponColor, false, weaponXhair);
			original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
		}
		else {
			if (i::StudioRender->IsForcedMaterialOverride()) {
				original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
			}
			else {
				EndChams();
				original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
			}
		}
		if (weaponOverlay) {
			BeginChams(materials[GLOW], weaponOverlayColor, false, weaponOverlayXhair);
			original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
		}
		if (weaponThinOverlay) {
			BeginChams(materials[THINGLOW], weaponThinOverlayColor, false, weaponThinOverlayXhair);
			original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
		}
		if (weaponAnimOverlay) {
			BeginChams(materials[ANIMATED], weaponAnimOverlayColor, false, weaponAnimOverlayXhair);
			original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
		}
		return true;
	}
	return false;
}