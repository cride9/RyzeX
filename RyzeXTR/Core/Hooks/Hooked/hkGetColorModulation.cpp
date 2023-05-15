#include "../hooks.h"
#include "../../Features/Misc/misc.h"

void __fastcall h::hkGetColorModulation(void* ecx, void* edx, float* r, float* g, float* b) {

	static auto original = detour::getColorModulation.GetOriginal<decltype(&h::hkGetColorModulation)>();

	original(ecx, edx, r, g, b);

	const IMaterial* pMaterial = reinterpret_cast<IMaterial*>(ecx);
	const char* szName = pMaterial->GetName();
	const char* szGroup = pMaterial->GetTextureGroupName();

	if (strstr(szName, "skybox/")) {
		misc::SkyboxChanger(szName);
		return;
	}

	if (cfg::misc::nightmode) {

		if (strstr(szGroup, "Other") || strstr(szName, "player") || strstr(szName, "chams") ||
			strstr(szName, "weapon") || strstr(szName, "glow") || strstr(szName, "skybox") || 
			strstr(szName, "dev/scope") || strstr(szName, "effects/"))
			return;

		bool bProp = strstr(szGroup, "StaticProp");

		*r *= bProp ? (cfg::misc::nightmodeColor[0] * 255.f) / 561.f : cfg::misc::nightmodeColor[0];
		*g *= bProp ? (cfg::misc::nightmodeColor[1] * 255.f) / 561.f : cfg::misc::nightmodeColor[1];
		*b *= bProp ? (cfg::misc::nightmodeColor[2] * 255.f) / 561.f : cfg::misc::nightmodeColor[2];
	}
}