#include "../hooks.h"

//char ryzeXTR[] = {'R', 'y', 'z', 'e', 'X', 'T', 'R'};
void __fastcall h::hkSetText(void* ecx, void* edx, const char* tokenName) {

	static auto original = detour::setText.GetOriginal<decltype(&h::hkSetText)>();

	if (!g::pLocal || !g::pLocal->GetWeapon() || !g::pCmd)
		return invokeFastcall<void>(adr(ecx), adr(edx), adr(original), ROP::ClientGadget_t::uReturnGadget, tokenName);
		//return original(ecx, edx, tokenName);

	CBaseCombatWeapon* pWeapon = g::pLocal->GetWeapon();

	static float timerLmao = 0.f;
	if (g::pLocal->GetWeapon()->GetItemDefinitionIndex() == WEAPON_C4 && g::pCmd->iButtons & IN_ATTACK) {

		if (timerLmao == 0.f)
			timerLmao = i::GlobalVars->flCurrentTime + 0.5f;

		float flTransition = i::GlobalVars->flCurrentTime - timerLmao;

		std::string thisText;
		for (size_t i = 0; i < cfg::misc::bombBuffer.length( ); i++)
			if (flTransition >= (i * 0.18f))
				thisText += cfg::misc::bombBuffer[i];

		return invokeFastcall<void>(adr(ecx), adr(edx), adr(original), ROP::ClientGadget_t::uReturnGadget, thisText.c_str());
	}
	else
		timerLmao = 0.f;

	return invokeFastcall<void>(adr(ecx), adr(edx), adr(original), ROP::ClientGadget_t::uReturnGadget, tokenName);
}