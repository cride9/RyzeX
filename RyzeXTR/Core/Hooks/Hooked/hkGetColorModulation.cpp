#include "../hooks.h"
#include "../../Features/Misc/misc.h"

void __fastcall h::hkGetColorModulation(void* ecx, void* edx, float* r, float* g, float* b) {

	static auto original = detour::getColorModulation.GetOriginal<decltype(&h::hkGetColorModulation)>();
	
	original(ecx, edx, r, g, b);

	static CConVar* r_DrawSpecificStaticProp = i::ConVar->FindVar("r_DrawSpecificStaticProp");
	if (r_DrawSpecificStaticProp->GetInt() != 0)
		r_DrawSpecificStaticProp->SetValue(0);

	IMaterial* pMaterial = reinterpret_cast<IMaterial*>(ecx);
	const char* szName = pMaterial->GetName();
	const char* szGroup = pMaterial->GetTextureGroupName();

	if (strstr(szGroup, XorStr("SkyBox textures"))) {

		*r *= cfg::misc::flSkyboxColor[0];
		*g *= cfg::misc::flSkyboxColor[1];
		*b *= cfg::misc::flSkyboxColor[2];

		misc::SkyboxChanger();
		return;
	}

	if (cfg::misc::nightmode) {

		if (strstr(szGroup, XorStr("World textures"))) {

			*r *= cfg::misc::nightmodeColor[0];
			*g *= cfg::misc::nightmodeColor[1];
			*b *= cfg::misc::nightmodeColor[2];
		}
		else if (strstr(szGroup, XorStr("StaticProp textures"))) {

			*r *= cfg::misc::propsColor[0] * 1.2f;
			*g *= cfg::misc::propsColor[1] * 1.2f;
			*b *= cfg::misc::propsColor[2] * 1.2f;
		}
	}
}