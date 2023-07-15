#include "../../hooks.h"
#include "../../../SDK/Entity.h"
#include "../../../Features/Rage/Animations/EnemyAnimations.h"

void __fastcall h::hkInterpolateServerEntites(void* ecx, void* edx) {

	static auto original = detour::interpolateEntites.GetOriginal<decltype(&h::hkInterpolateServerEntites)>();

	if (!g::pLocal || i::ClientState->iSignonState != SIGNONSTATE_FULL || g::bUpdatingSkins)
		return original(ecx, edx);

	lagcomp.SetInterpolationFlags();
	invokeFastcall<void>(adr(ecx), adr(edx), adr(original), ROP::ClientGadget_t::uReturnGadget);
	//original(ecx, edx);

	anims.InterpolateMatricies();
	g_LocalAnimations->InterpolateMatricies();
}