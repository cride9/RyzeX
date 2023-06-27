#include "../hooks.h"
#include "../../Features/Misc/misc.h"

void __fastcall h::hkGetColorModulation(void* ecx, void* edx, float* r, float* g, float* b) {

	static auto original = detour::getColorModulation.GetOriginal<decltype(&h::hkGetColorModulation)>();
	
	original(ecx, edx, r, g, b);

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

			*r *= cfg::misc::nightmodeColor[0] /** 0.4544644448404919f*/;
			*g *= cfg::misc::nightmodeColor[1] /** 0.4544644448404919f*/;
			*b *= cfg::misc::nightmodeColor[2] /** 0.4544644448404919f*/;
		}
		else if (strstr(szGroup, XorStr("StaticProp textures"))) {

			*r *= cfg::misc::nightmodeColor[0];
			*g *= cfg::misc::nightmodeColor[1];
			*b *= cfg::misc::nightmodeColor[2];
		}
	}
}