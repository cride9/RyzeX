#include "../hooks.h"
#include "../../SDK/Entity.h"
#include "../../globals.h"
#include "../../SDK/math.h"
#include "../../SDK/Menu/gui.h"
#include "../../Features/Misc/enginepred.h"
#include "../../Features/Misc/misc.h"
#include "../../Features/Rage/antiaim.h"
#include "../../Features/Rage/ragebot.h"
#include "../../Features/Rage/Animations/LocalAnimation.h"
#include "../../Features/Rage/doubletap.h"

static void __stdcall CreateMove(int nSequenceNumber, float flInputSampleFrametime, bool bIsActive, bool& bSendPacket) {

	static auto original = detour::createMove.GetOriginal<decltype(&h::hkCreateMoveProxy)>();

	// call original first so our movement and other stuff will be sent normally
	original(i::ClientDll, 0, nSequenceNumber, flInputSampleFrametime, bIsActive);

	CUserCmd* pCmd = i::Input->GetUserCmd(nSequenceNumber);
	CVerifiedUserCmd* pVerifiedCmd = i::Input->GetVerifiedCmd(nSequenceNumber);

	if (!pCmd || !pVerifiedCmd || !bIsActive)
		return;

	if (g::bShifting) {

		//pCmd->flSideMove = std::clamp((pCmd->flSideMove * -1) * 500, -450.f, 450.f);
		misc::IdealTick(pCmd);
		localanim.update = true;

		bSendPacket = doubletap::shiftAmount == 1; // Only send on the last shifted
		pCmd->iButtons &= ~(IN_ATTACK | IN_SECOND_ATTACK);

		pVerifiedCmd->userCmd = *pCmd;
		pVerifiedCmd->uHashCRC = pCmd->GetChecksum();
		return;
	}

	CBaseEntity* pLocal = g::pLocal = (CBaseEntity*)i::EntityList->GetClientEntity(i::EngineClient->GetLocalPlayer());
	g::pCmd = pCmd;

	auto oldViewAngle = g::oldViewAngle = pCmd->angViewPoint;

	// for now that is the fix for the menu xddxdx
	if (GetAsyncKeyState(VK_LBUTTON) && menu::open)
		pCmd->iButtons &= ~IN_ATTACK;

	misc::CreateMove(pCmd, oldViewAngle, bSendPacket);

	prediction.Start(pCmd, pLocal);
	{
		antiaim::AntiAim(pCmd, bSendPacket);

		ragebot.CreateMove(pCmd, pLocal, bSendPacket);
	}
	prediction.End(pCmd, pLocal);

	doubletap::Doubletap();

	misc::MovementFix(pCmd, oldViewAngle);

	// emergency bsendpacket to prevent server disconnecting
	if (i::ClientState->nChokedCommands > 15) {
		bSendPacket = true;
		util::Print("Emergency!");
	}

	if (bSendPacket)
		localanim.localdata.vecViewAngle = pCmd->angViewPoint;

	pCmd->angViewPoint.Normalize();
	pCmd->angViewPoint.Clamp();

	pVerifiedCmd->userCmd = *pCmd;
	pVerifiedCmd->uHashCRC = pCmd->GetChecksum();

	localanim.update = true;
}

__declspec(naked) void __fastcall h::hkCreateMoveProxy(IBaseClientDLL* thisptr, int edx, int nSequenceNumber, float flInputSampleFrametime, bool bIsActive)
{
	__asm
	{
		push	ebp
		mov		ebp, esp; // store the stack
		push	ebx; // bSendPacket
		push	esp; // restore the stack
		push	dword ptr[bIsActive]; // ebp + 16
		push	dword ptr[flInputSampleFrametime]; // ebp + 12
		push	dword ptr[nSequenceNumber]; // ebp + 8
		call	CreateMove // call our function
			pop		ebx
			pop		ebp
			retn	0Ch
	}
}