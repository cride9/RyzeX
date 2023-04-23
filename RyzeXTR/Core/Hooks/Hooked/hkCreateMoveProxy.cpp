#include "../hooks.h"
#include "../../SDK/Entity.h"
#include "../../globals.h"
#include "../../SDK/math.h"
#include "../../SDK/Menu/gui.h"
#include "../../Features/Misc/enginepred.h"
#include "../../Features/Misc/misc.h"
#include "../../Features/Rage/antiaim.h"
#include "../../Features/Rage/Animations/LocalAnimation.h"
#include "../../Features/Rage/exploits.h"
#include "../../Features/Networking/networking.h"
#include "../../Features/Rage/ragebot.h"
#include "../../Features/Rage/Animations/Lagcompensation.h"

static void __stdcall CreateMove(int nSequenceNumber, float flInputSampleFrametime, bool bIsActive, bool& bSendPacket) {

	static auto original = detour::createMove.GetOriginal<decltype(&h::hkCreateMoveProxy)>();

	// call original first so our movement and other stuff will be sent normally
	original(i::ClientDll, 0, nSequenceNumber, flInputSampleFrametime, bIsActive);

	if (i::ClientState->iSignonState != SIGNONSTATE_FULL)
		return;

	CUserCmd* pCmd = i::Input->GetUserCmd(nSequenceNumber);
	CVerifiedUserCmd* pVerifiedCmd = i::Input->GetVerifiedCmd(nSequenceNumber);

	if (!pCmd || !pVerifiedCmd || !bIsActive)
		return;

	bSendPacket = true;
	CBaseEntity* pLocal = g::pLocal = reinterpret_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i::EngineClient->GetLocalPlayer()));
	g::pCmd = pCmd;

	Vector oldViewAngle = g::vecOriginalViewAngle = pCmd->angViewPoint;

	// for now that is the fix for the menu xddxdx
	if (pCmd->iButtons & IN_ATTACK && menu::open)
		pCmd->iButtons &= ~(IN_ATTACK | IN_SECOND_ATTACK | IN_MIDDLE_ATTACK);

	if (cfg::misc::infiniteDuck)
		pCmd->iButtons |= IN_BULLRUSH;

	misc::CreateMove(pCmd, oldViewAngle, bSendPacket);

	if ( exploits::bIsShiftingTicks )
		bSendPacket = pLocal->GetWeapon() ? pLocal->GetWeapon()->GetItemDefinitionIndex() == WEAPON_SSG08 ? true : exploits::iShiftAmount == 1 ? true : false : exploits::iShiftAmount == 1 ? true : false; // Only send on the last shifted

	prediction.SaveNetvars(pCmd->iCommandNumber, pLocal);

	prediction.Start(pCmd, pLocal, nSequenceNumber);
	{
		g_LocalAnimations->CopyPlayerAnimationData(false);
		g_LocalAnimations->SetupShootPosition( );

		antiaim::AntiAim(pCmd, bSendPacket);

		ragebot.CreateMove(pCmd, pLocal, bSendPacket);

		exploits::HandleDoubleTap( bSendPacket, pCmd );
		exploits::HandleBreakLagcomp(pCmd);
		misc::IdealTick(pCmd);
		misc::AutoPistol(pCmd, pLocal);
		if (pCmd->iButtons & IN_ATTACK)
			misc::vecEyePosition = pLocal->GetEyePosition();
		if (ragebot.rageBotData.iTickCount + 3 >= i::GlobalVars->iTickCount)
			bSendPacket = true;
	}
	prediction.End(pCmd, pLocal);

	prediction.RestoreNetvars( pCmd->iCommandNumber, pLocal);

	misc::MovementFix(pCmd, g::vecOriginalViewAngle);

	INetChannel* pNetChannel = i::ClientState->pNetChannel;

	//h::HookNetChannel(pNetChannel);

	if ( cfg::misc::fakePing && pNetChannel)
		lagcomp.UpdateIncomingSequences( pNetChannel );
	else
		lagcomp.ClearIncomingSequences( );

	h::HookClientState();

	static auto maxusercmd = i::ConVar->FindVar("sv_maxusrcmdprocessticks");
	if (i::ClientState->nChokedCommands >= maxusercmd->GetInt() - 1)
		bSendPacket = true;

	if (bSendPacket)
		packetManager.pCommandList.emplace_back(pCmd->iCommandNumber);

	pCmd->angViewPoint.Normalize();
	pCmd->angViewPoint.Clamp();

	pVerifiedCmd->userCmd = *pCmd;
	pVerifiedCmd->uHashCRC = pCmd->GetChecksum();

	if (bSendPacket)
		localanim.localdata.vecViewAngle = pVerifiedCmd->userCmd.angViewPoint;

	g_LocalAnimations->OnCreateMove(bSendPacket);
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