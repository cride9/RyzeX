#include "chams.h"
#include "../../Interface/Classes/CKeyValues.h"
#include "../Rage/Animations/LocalAnimation.h"
#include "../../SDK/math.h"
#include "../Misc/misc.h"
#include "../Rage/Animations/Lagcompensation.h"

enum EMATERIAL : int {

	DEFAULT,
	FLAT,
	GLOW,
	THINGLOW,
	ANIMATED
};

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

	return i::MaterialSystem->CreateMaterial(szName.data(), pKeyValues);
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
	return i::MaterialSystem->CreateMaterial(szName.data(), pKeyValues);
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

bool chams::DrawChams(CBaseEntity* pLocal, DrawModelResults_t* pResults, const DrawModelInfo_t& info, matrix3x4_t* pBoneToWorld, float* flFlexWeights, float* flFlexDelayedWeights, const Vector& vecModelOrigin, int nFlags) {

	static auto original = detour::drawModel.GetOriginal<decltype(&h::hkDrawModel)>();

	IClientRenderable* pRenderable = info.pClientEntity;

	if (!pRenderable)
		return false;

	CBaseEntity* pEnt = pRenderable->GetIClientUnknown()->GetBaseEntity();

	if (!pEnt)
		return false;

	const std::string_view szModelName = info.pStudioHdr->szName;

	static auto MakeChams = [](IMaterial* pMaterial, float const* flColor, bool bIgnoreZ, bool bWireFrame) {

		i::StudioRender->SetColorModulation(flColor, pMaterial);
		i::StudioRender->SetAlphaModulation(flColor[3]);

		pMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, bIgnoreZ);
		pMaterial->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, bWireFrame);

		i::StudioRender->ForcedMaterialOverride(pMaterial);
	};

	if (pEnt->IsPlayer() && pEnt->IsAlive()) {

		if (nFlags & (STUDIO_RENDER | STUDIO_SKIP_FLEXES | STUDIO_DONOTMODIFYSTENCILSTATE | STUDIO_NOLIGHTING_OR_CUBEMAP | STUDIO_SKIP_DECALS))
			return false;

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

			if (pEnt->GetTeam() == pLocal->GetTeam()) {

				if (pEnt == pLocal) {

					if (misc::vecRecord != Vector(0.f, 0.f, 0.f) && cfg::model::localIdealTick) {

						if (misc::bRetreat)
							MakeChams(materials[ANIMATED], cfg::model::localIdealTickColor2, false, true);
						else
							MakeChams(materials[ANIMATED], cfg::model::localIdealTickColor, false, true);
						original(i::StudioRender, 0, pResults, info, misc::matrixRecord, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
					}

					if (!cfg::visual::localEsp)
						return false;

					bool justOverlay = true;

					matrix3x4_t* localmatrix = /*pBoneToWorld*/localanim.localdata.Matrix;

					//float curangle = g::oldViewAngle.y;
					//int angle = 0;

					//if (curangle <= 180 && curangle >= 145)
					//	angle = 0;
					//else if (curangle >= -180 && curangle <= -145)
					//	angle = 0;
					//else if (curangle >= 0 && curangle <= 45)
					//	angle = 0;
					//else if (curangle <= 0 && curangle >= -45)
					//	angle = 0;
					//else
					//	angle = 1;

					//for (int i = 0; i < 128; i++) {

					//	localmatrix[i][angle][0] = 0.1;
					//	localmatrix[i][angle][1] = 0.1;
					//	localmatrix[i][angle][2] = 0.1;
					//	localmatrix[i][angle][3] = (angle == 0? pEnt->GetAbsOrigin().x : pEnt->GetAbsOrigin().y);
					//}

					if (cfg::visual::localGlow) {

						MakeChams(materials[1], new float[] {cfg::visual::localGlowColor[0], cfg::visual::localGlowColor[1], cfg::visual::localGlowColor[2], min(254.f / 255.f, cfg::visual::localGlowColor[3])}, false, true);
						original(i::StudioRender, 0, pResults, info, localmatrix, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);

						i::StudioRender->SetAlphaModulation(1.f);
						i::StudioRender->SetColorModulation(new float[] {1.f, 1.f, 1.f});
						i::StudioRender->ForcedMaterialOverride(nullptr);
						original(i::StudioRender, 0, pResults, info, localmatrix, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
					}

					if (cfg::model::local) {

						if (cfg::model::localXQZ) {

							MakeChams(materials[cfg::model::localType], cfg::model::localXQZColor, true, cfg::model::localXQZXhair);

							original(i::StudioRender, 0, pResults, info, localmatrix, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
						}

						MakeChams(materials[cfg::model::localType], cfg::model::localColor, false, cfg::model::localXhair);

						original(i::StudioRender, 0, pResults, info, localmatrix, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
						justOverlay = false;
					}
					if (cfg::model::localOverlay || cfg::model::localThinOverlay || cfg::model::localAnimOverlay) {

						if (justOverlay)
							original(i::StudioRender, 0, pResults, info, localmatrix, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);

						if (cfg::model::localOverlay) {
							if (cfg::model::localOverlayXQZ) {

								MakeChams(materials[GLOW], cfg::model::localOverlayXQZColor, false, cfg::model::localOverlayXQZXhair);
								original(i::StudioRender, 0, pResults, info, localmatrix, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
							}
							MakeChams(materials[GLOW], cfg::model::localOverlayColor, false, cfg::model::localOverlayXhair);
							original(i::StudioRender, 0, pResults, info, localmatrix, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
						}
						if (cfg::model::localThinOverlay) {
							if (cfg::model::localThinOverlayXQZ) {

								MakeChams(materials[THINGLOW], cfg::model::localThinOverlayXQZColor, false, cfg::model::localThinOverlayXQZXhair);
								original(i::StudioRender, 0, pResults, info, localmatrix, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
							}
							MakeChams(materials[THINGLOW], cfg::model::localThinOverlayColor, false, cfg::model::localThinOverlayXhair);
							original(i::StudioRender, 0, pResults, info, localmatrix, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
						}
						if (cfg::model::localAnimOverlay) {
							if (cfg::model::localAnimOverlayXQZ) {

								MakeChams(materials[ANIMATED], cfg::model::localAnimOverlayXQZColor, false, cfg::model::localAnimOverlayXQZXhair);
								original(i::StudioRender, 0, pResults, info, localmatrix, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
							}
							MakeChams(materials[ANIMATED], cfg::model::localAnimOverlayColor, false, cfg::model::localAnimOverlayXhair);
							original(i::StudioRender, 0, pResults, info, localmatrix, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
						}
					}

					return true;
				}

				if (!cfg::visual::teamEsp)
					return false;

				bool justOverlay = true;

				if (cfg::visual::teamGlow) {

					MakeChams(materials[1], new float[] {cfg::visual::teamGlowColor[0], cfg::visual::teamGlowColor[1], cfg::visual::teamGlowColor[2], min(254.f / 255.f, cfg::visual::teamGlowColor[3])}, true, true);
					original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);

					i::StudioRender->SetAlphaModulation(1.f);
					i::StudioRender->SetColorModulation(new float[] {1.f, 1.f, 1.f});
					i::StudioRender->ForcedMaterialOverride(nullptr);
					original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
				}

				if (cfg::model::team) {

					if (cfg::model::teamXQZ) {

						MakeChams(materials[cfg::model::teamType], cfg::model::teamXQZColor, true, cfg::model::teamXQZXhair);

						original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
					}

					MakeChams(materials[cfg::model::teamType], cfg::model::teamColor, false, cfg::model::teamXhair);

					original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
					justOverlay = false;
				}
				if (cfg::model::teamOverlay || cfg::model::teamThinOverlay || cfg::model::teamAnimOverlay) {

					if (justOverlay)
						original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);

					if (cfg::model::teamOverlay) {
						if (cfg::model::teamOverlayXQZ) {

							MakeChams(materials[GLOW], cfg::model::teamOverlayXQZColor, true, cfg::model::teamOverlayXQZXhair);
							original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
						}
						MakeChams(materials[GLOW], cfg::model::teamOverlayColor, false, cfg::model::teamOverlayXhair);
						original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
					}
					if (cfg::model::teamThinOverlay) {
						if (cfg::model::teamThinOverlayXQZ) {

							MakeChams(materials[THINGLOW], cfg::model::teamThinOverlayXQZColor, true, cfg::model::teamThinOverlayXQZXhair);
							original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
						}
						MakeChams(materials[THINGLOW], cfg::model::teamThinOverlayColor, false, cfg::model::teamThinOverlayXhair);
						original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
					}
					if (cfg::model::teamAnimOverlay) {
						if (cfg::model::teamAnimOverlayXQZ) {

							MakeChams(materials[ANIMATED], cfg::model::teamAnimOverlayXQZColor, true, cfg::model::teamAnimOverlayXQZXhair);
							original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
						}
						MakeChams(materials[ANIMATED], cfg::model::teamAnimOverlayColor, false, cfg::model::teamAnimOverlayXhair);
						original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
					}
				}
				return true;
			}
			else {

				//if (lagcomp.deqRecords[pEnt->EntIndex()].size() >= 2) {

				//	MakeChams(materials[1], new float[] {1.f, 1.f, 1.f, 0.5f}, true, false);
				//	original(i::StudioRender, 0, pResults, info, lagcomp.deqRecords[pEnt->EntIndex()].front().pMatrix, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
				//}

				if (!cfg::visual::enemyEsp)
					return false;

				if (cfg::visual::enemyGlow) {

					MakeChams(materials[1], new float[] {cfg::visual::enemyGlowColor[0], cfg::visual::enemyGlowColor[1], cfg::visual::enemyGlowColor[2], min(254.f / 255.f, cfg::visual::enemyGlowColor[3])}, true, true);
					original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);

					if (!cfg::model::enemy) {

						i::StudioRender->SetAlphaModulation(1.f);
						i::StudioRender->SetColorModulation(new float[] {1.f, 1.f, 1.f});
						i::StudioRender->ForcedMaterialOverride(nullptr);
						original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
					}
				}

				bool justOverlay = true;
				if (cfg::model::enemy) {

					if (cfg::model::enemyXQZ) {

						MakeChams(materials[cfg::model::enemyType], cfg::model::enemyXQZColor, true, cfg::model::enemyXQZXhair);

						original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
					}

					MakeChams(materials[cfg::model::enemyType], cfg::model::enemyColor, false, cfg::model::enemyXhair);
					original( i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags );	
					justOverlay = false;
				}
				if (cfg::model::enemyOverlay || cfg::model::enemyThinOverlay || cfg::model::enemyAnimOverlay) {

					if (justOverlay)
						original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);

					if (cfg::model::enemyOverlay) {
						if (cfg::model::enemyOverlayXQZ) {
							
							MakeChams(materials[GLOW], cfg::model::enemyOverlayXQZColor, true, cfg::model::enemyOverlayXQZXhair);
							original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
						}
						MakeChams(materials[GLOW], cfg::model::enemyOverlayColor, false, cfg::model::enemyOverlayXhair);
						original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
					}
					if (cfg::model::enemyThinOverlay) {
						if (cfg::model::enemyThinOverlayXQZ) {

							MakeChams(materials[THINGLOW], cfg::model::enemyThinOverlayXQZColor, true, cfg::model::enemyThinOverlayXQZXhair);
							original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
						}
						MakeChams(materials[THINGLOW], cfg::model::enemyThinOverlayColor, false, cfg::model::enemyThinOverlayXhair);
						original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
					}
					if (cfg::model::enemyAnimOverlay) {
						if (cfg::model::enemyAnimOverlayXQZ) {

							MakeChams(materials[ANIMATED], cfg::model::enemyAnimOverlayXQZColor, true, cfg::model::enemyAnimOverlayXQZXhair);
							original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
						}
						MakeChams(materials[ANIMATED], cfg::model::enemyAnimOverlayColor, false, cfg::model::enemyAnimOverlayXhair);
						original(i::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
					}
				
				}
				return true;
			}
		}
	}
	return false;
}