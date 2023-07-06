#include "../hooks.h"
#include "../../SDK/Entity.h"
#include "../../globals.h"
#include "../../SDK/math.h"
#include "../../SDK/Menu/gui.h"
#include "../../Features/Misc/enginepred.h"
#include "../../Features/Misc/misc.h"
#include "../../Features/Misc/Playerlist.h"
#include "../../Features/Rage/antiaim.h"
#include "../../Features/Rage/Animations/LocalAnimation.h"
#include "../../Features/Rage/exploits.h"
#include "../../Features/Networking/networking.h"
#include "../../Features/Rage/ragebot.h"
#include "../../Features/Rage/Animations/Lagcompensation.h"
#include "../../Features/Rage/Animations/EnemyAnimations.h"
#include "../../SDK/InputSystem.h"

static void __stdcall CreateMove(int nSequenceNumber, float flInputSampleFrametime, bool bIsActive, bool& bSendPacket) {

	static auto original = detour::createMove.GetOriginal<decltype(&h::hkCreateMoveProxy)>();
	
	// call original first so our movement and other stuff will be sent normally
	CUserCmd* pCmd = i::Input->GetUserCmd(nSequenceNumber);
	CVerifiedUserCmd* pVerifiedCmd = i::Input->GetVerifiedCmd(nSequenceNumber);

	original(i::ClientDll, 0, nSequenceNumber, flInputSampleFrametime, exploits::bIsCurrentlyCharging ? bIsActive : !(exploits::bIsShiftingTicks && ((cfg::rage::doubletap && IPT::HandleInput(cfg::rage::doubletapkey)) || (cfg::rage::hideshot && IPT::HandleInput(cfg::rage::hideshotkey))) && !(cfg::antiaim::idealTick && IPT::HandleInput(cfg::antiaim::idealTickBind))));

	if (!pCmd || !pVerifiedCmd || !bIsActive)
		return;

	bSendPacket = true;
	CBaseEntity* pLocal = g::pLocal = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i::EngineClient->GetLocalPlayer()));
	g::pCmd = pCmd;
	g::vecEyePosition = pLocal->GetEyePosition(false);

	lagcomp.StartLagcompensation(pLocal);

	Vector oldViewAngle = g::vecOriginalViewAngle = pCmd->angViewPoint;

	if (cfg::misc::infiniteDuck)
		pCmd->iButtons |= IN_BULLRUSH;

	misc::CreateMove(pCmd, oldViewAngle, bSendPacket);
	playerList::InitializePlayerList(pLocal);

	//if (i::ClientState->iDeltaTick > 0)
	//	i::Prediction->Update(i::ClientState->iDeltaTick, i::ClientState->iDeltaTick > 0, i::ClientState->iLastCommandAck, i::ClientState->iLastOutgoingCommand + i::ClientState->nChokedCommands);

	prediction.SaveNetvars(pCmd->iCommandNumber, pLocal);

	prediction.Start(pCmd, pLocal, nSequenceNumber);
	{
		g_LocalAnimations->CopyPlayerAnimationData(false, pLocal);
		g_LocalAnimations->SetupShootPosition(pLocal );

		antiaim::AntiAim(pCmd, bSendPacket);

		ragebot.CreateMove(pCmd, pLocal, bSendPacket);

		exploits::HandleDoubleTap( bSendPacket, pCmd );
		exploits::HandleHideShots(bSendPacket, pCmd);
		exploits::HandleBreakLagcomp(pCmd);
		misc::IdealTick(pCmd, pLocal);
		antiaim::InvertOnShoot(pCmd);
	}
	prediction.End(pCmd, pLocal);

	prediction.RestoreNetvars( pCmd->iCommandNumber, pLocal);

	misc::AutoPistol(pCmd, pLocal);
	misc::MovementFix(pCmd, g::vecOriginalViewAngle);

	INetChannel* pNetChannel = i::ClientState->pNetChannel;

	h::HookNetChannel(pNetChannel);

	if ( cfg::misc::fakePing && pNetChannel)
		lagcomp.UpdateIncomingSequences( pNetChannel );
	else
		lagcomp.ClearIncomingSequences( );

	h::HookClientState();

	if (ragebot.bSendPacketThisTick) {
		bSendPacket = true;
		ragebot.bSendPacketThisTick = false;
	}

	if (i::ClientState->nChokedCommands >= 14)
		bSendPacket = true;

	if (exploits::bIsShiftingTicks) {
		if (exploits::iShiftAmount == 12)
			pCmd->iButtons &= ~IN_ATTACK;

		if ((cfg::rage::hideshot && IPT::HandleInput(cfg::rage::hideshotkey)))
			bSendPacket = false;
		else
			bSendPacket = pLocal->GetWeapon() ? pLocal->GetWeapon()->GetItemDefinitionIndex() == WEAPON_SSG08 ? true : exploits::iShiftAmount == 1 ? true : false : exploits::iShiftAmount == 1 ? true : false; // Only send on the last shifted
	}

	if (bSendPacket)
		packetManager.pCommandList.emplace_back(pCmd->iCommandNumber);

	pCmd->angViewPoint.Normalize();
	pCmd->angViewPoint.Clamp();

	pVerifiedCmd->userCmd = *pCmd;
	pVerifiedCmd->uHashCRC = pCmd->GetChecksum();

	if (bSendPacket)
		localanim.localdata.vecViewAngle = pVerifiedCmd->userCmd.angViewPoint;

	g_LocalAnimations->OnCreateMove(bSendPacket, pLocal);
	lagcomp.FinishLagcompensation(pLocal);
	anims.ResolverLogic();
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