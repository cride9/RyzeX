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

	CUserCmd* pCmd = i::Input->GetUserCmd(nSequenceNumber);
	CVerifiedUserCmd* pVerifiedCmd = i::Input->GetVerifiedCmd(nSequenceNumber);

	if (!pCmd || !pVerifiedCmd || !bIsActive)
		return;

	if ( i::ClientState->iDeltaTick > 0 )
		i::Prediction->Update( i::ClientState->iDeltaTick, i::ClientState->iDeltaTick > 0, i::ClientState->iLastCommandAck, i::ClientState->iLastOutgoingCommand + i::ClientState->nChokedCommands );

	CBaseEntity* pLocal = g::pLocal = (CBaseEntity*)i::EntityList->GetClientEntity(i::EngineClient->GetLocalPlayer());
	g::pCmd = pCmd;

	Vector oldViewAngle = pCmd->angViewPoint;

	// for now that is the fix for the menu xddxdx
	if (GetAsyncKeyState(VK_LBUTTON) && menu::open)
		pCmd->iButtons &= ~IN_ATTACK;

	prediction.SaveNetvars( pCmd->iCommandNumber, pLocal);

	misc::CreateMove(pCmd, oldViewAngle, bSendPacket);

	if ( exploits::bIsShiftingTicks ) {

		bSendPacket = pLocal->GetWeapon() ? pLocal->GetWeapon()->GetItemDefinitionIndex() == WEAPON_SSG08 ? true : exploits::iShiftAmount == 1 ? true : false : exploits::iShiftAmount == 1 ? true : false; // Only send on the last shifted

		pVerifiedCmd->userCmd = *pCmd;
		pVerifiedCmd->uHashCRC = pCmd->GetChecksum( );
	}

	prediction.Start(pCmd, pLocal, nSequenceNumber);
	{
		g_LocalAnimations->CopyPlayerAnimationData(false);
		g_LocalAnimations->SetupShootPosition( );

		antiaim::AntiAim(pCmd, bSendPacket);

		ragebot.CreateMove(pCmd, pLocal, bSendPacket);

		exploits::HandleDoubleTap( bSendPacket, pCmd );
	}
	prediction.End(pCmd, pLocal);

	prediction.RestoreNetvars( pCmd->iCommandNumber, pLocal);

	misc::MovementFix(pCmd, oldViewAngle);

	// netchannel pointer
	INetChannel* pNetChannel = i::ClientState->pNetChannel;

	// @note: doesnt need rehook cuz detours here
	if ( pNetChannel != nullptr )
	{
		if ( !detour::processPacket.IsHooked( ) )
			detour::processPacket.Create( util::GetVFunc( pNetChannel, table::processPacket ), &h::hkProcessPacket );

		if ( !detour::sendNetMsg.IsHooked( ) )
			detour::sendNetMsg.Create( util::GetVFunc( pNetChannel, table::sendNetMsg ), &h::hkSendNetMsg );

		if ( !detour::setChoked.IsHooked( ) )
			detour::setChoked.Create( util::GetVFunc( pNetChannel, table::setChoked ), &h::hkSetChoked );

		if ( !detour::sendDatagram.IsHooked( ) )
			detour::sendDatagram.Create( util::GetVFunc( pNetChannel, table::sendDatagram ), &h::hkSendDatagram );
	}

	if ( cfg::misc::fakePing )
		lagcomp.UpdateIncomingSequences( pNetChannel );
	else
		lagcomp.ClearIncomingSequences( );

	static const auto clientStateHookable = (void*)(uintptr_t(i::ClientState) + 0x8); // ignore c-style casting

	if ( clientStateHookable != nullptr )
	{
		// PacketStart Detour
		if ( !detour::packetStart.IsHooked( ) )
			detour::packetStart.Create( util::GetVFunc( clientStateHookable, table::packetStart ), &h::hkPacketStart );

		// PacketEnd Detour
		if ( !detour::packetEnd.IsHooked( ) )
			detour::packetEnd.Create( util::GetVFunc( clientStateHookable, table::packetEnd ), &h::hkPacketEnd );

		if ( !detour::temptEntities.IsHooked( ) )
			detour::temptEntities.Create( util::GetVFunc( clientStateHookable, table::temptEntities ), &h::hkTemptEntities );
	}

	g::bSendPacket = &bSendPacket;

	if (bSendPacket)
		packetManager.pCommandList.emplace_back(pCmd->iCommandNumber);

	pCmd->angViewPoint.Normalize();
	pCmd->angViewPoint.Clamp();

	g_LocalAnimations->OnCreateMove( );

	pVerifiedCmd->userCmd = *pCmd;
	pVerifiedCmd->uHashCRC = pCmd->GetChecksum();

	if (bSendPacket)
		localanim.localdata.vecViewAngle = pVerifiedCmd->userCmd.angViewPoint;
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