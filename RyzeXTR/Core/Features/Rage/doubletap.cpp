#include "doubletap.h"

void doubletap::Doubletap() {

	if (!cfg::rage::doubletap || !GetKeyState(cfg::rage::doubletapkey)) {

		rechargeAmount = 17;
		bCharged = false;
		bReset = false;
		return;
	}

	if (cfg::antiaim::fakeduck && GetAsyncKeyState(cfg::antiaim::fakeduckbind))
		return;

	if (!g::pLocal || !g::pLocal->IsAlive())
		return;

	if (!g::pLocal->GetWeapon())
		return;

	static auto sv_maxusrcmdprocessticks = i::ConVar->FindVar("sv_maxusrcmdprocessticks");

	if (g::pCmd->iButtons & IN_ATTACK && !bReset && bCharged && !rechargeAmount) {

		cmdCommandNumber = g::pCmd->iCommandNumber;
		cmdTickbase = prediction.GetTickBase(g::pCmd, g::pLocal);
		cmdCurtime = i::GlobalVars->flCurrentTime;

		shiftAmount = 15;
		bReset = true;
	}

	if (g::pLocal->GetWeapon()->GetLastShotTime() + 0.24f < i::GlobalVars->flCurrentTime && bReset && !shiftAmount && !g::bShifting) {

		g::bWaiting = true;
		
		if (!(g::pCmd->iButtons & IN_ATTACK) && g::pLocal->GetWeapon()->GetLastShotTime() + 2.f < i::GlobalVars->flCurrentTime) {

			if (!(g::pCmd->iButtons & IN_ATTACK)) {

				rechargeAmount = 16;
				bCharged = false;
				bReset = false;
			}
		}
	}
}