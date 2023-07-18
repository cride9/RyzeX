#include "../../Features/Rage/exploits.h"
#include "../../SDK/Entity.h"
#include "../../globals.h"
#include "../hooks.h"
#include "../../Features/Misc/enginepred.h"
#include "../../Features/Networking/networking.h"
#undef max

/* CBasePlayer::PhysicsSimulate */
void __fastcall h::hkPhysicsSimulate(CBaseEntity* ecx, void* edx) {

	static auto original = detour::physicsSimulate.GetOriginal<decltype(&h::hkPhysicsSimulate)>();

	/*
		m_nSimulationTick = gpGlobals->tickcount;
	
		if ( simulation_ticks > 0 )
		{
			AdjustPlayerTimeBase( simulation_ticks );
		}
	*/

	CBaseEntity* pEntity = reinterpret_cast<CBaseEntity*>(ecx);

	if (pEntity != g::pLocal)
		return detour::physicsSimulate.CallOriginal<void>(ROP::ClientGadget_t::uReturnGadget, ecx, edx);

	if (!pEntity->IsAlive())
		return detour::physicsSimulate.CallOriginal<void>(ROP::ClientGadget_t::uReturnGadget, ecx, edx);

	int& SimulationTick = *pEntity->GetOffsetPointer<int>( 0x2ACU );
	CCommandContext* pCommandContext = pEntity->GetOffsetPointer<CCommandContext>(0x350C);

	if ( SimulationTick == i::GlobalVars->iTickCount )
		return detour::physicsSimulate.CallOriginal<void>(ROP::ClientGadget_t::uReturnGadget, ecx, edx);

	if ( !pCommandContext || !pCommandContext->bNeedsProcessing )
		return detour::physicsSimulate.CallOriginal<void>(ROP::ClientGadget_t::uReturnGadget, ecx, edx);

	if ( pCommandContext->pCmd.iTickCount >= std::numeric_limits< int >::max( ) )
	{
		SimulationTick = i::GlobalVars->iTickCount;
		return networking.SaveNetvarData( pCommandContext->nCommandNumber );
	}
		
	//if ( pCommandContext->pCmd.iCommandNumber == ( i::ClientState->iCommandAck + 1 ) )
	//	pEntity->GetVelocityModifier( ) = localanim.localdata.flVelocityModifier;

	networking.RestoreNetvarData( pCommandContext->nCommandNumber - 1 );

	const int iTickBaseBackup = pEntity->GetTickBase( );
	pEntity->GetTickBase() = exploits::GetNetworkTickbase( pCommandContext->pCmd.iCommandNumber );

	detour::physicsSimulate.CallOriginal<void>(ROP::ClientGadget_t::uReturnGadget, ecx, edx);

	prediction.SaveViewmodelData(pEntity);
	return networking.SaveNetvarData( pCommandContext->nCommandNumber );
}