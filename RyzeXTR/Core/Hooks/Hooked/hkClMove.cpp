#include "../hooks.h"
#include "../../Interface/Classes/CConvar.h"
#include "../../SDK/Menu/config.h"
#include "../../Features/Rage/doubletap.h"
using namespace doubletap;

//https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/engine/cl_main.cpp#L2736
void __cdecl h::hkClMove(float flSamples, bool bFinalTick) {

	static auto original = detour::clMove.GetOriginal<decltype(&h::hkClMove)>();

	if (cfg::rage::doubletap && GetKeyState(cfg::rage::doubletapkey)) {

		if (GetAsyncKeyState(VK_END))
			return;

		if (bCharged && shiftAmount) {

			while (shiftAmount) {

				g::bShifting = true;
				shiftAmount--;
				original(flSamples, bFinalTick);
			}
			g::bShifting = false;
			return;
		}
		else if (!bCharged && rechargeAmount > 0 && !bCharged && !shiftAmount) {

			g::pCmd->iButtons &= ~IN_ATTACK;
			if (i::GlobalVars->iTickCount % 2 == 0) {

				g::bWaiting = false;
				g::bSendPacket = true;
				rechargeAmount--;

				if (rechargeAmount == 0) {
					bCharged = true;
				}

				return;
			}
			else {

				rechargeCommandNumber = g::pCmd->iCommandNumber;
				rechargeTickbase = prediction.GetTickBase(g::pCmd, g::pLocal);
				rechargeCurtime = i::GlobalVars->flCurrentTime;

				return original(flSamples, bFinalTick);
			}
		}

		return original(flSamples, bFinalTick);
	}

	return original(flSamples, bFinalTick);

	// this hook calls g_ClientDLL->CreateMove (that is hooked lmao)
	// g_ClientDLL->CreateMove( cl.lastoutgoingcommand + cl.chokedcommands + 1, host_state.interval_per_tick - flSamples, !cl.IsPaused() );

	// changing flSamples while doubletapping can cause some issues like freezing when doubletapping while our ticks are getting back to sync
	// and gl getting back ticks to synced values while we freeze :D (takes like 20-30sec to unfreeze the game)
}