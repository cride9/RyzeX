#include "../hooks.h"
#include "../../globals.h"
#include "../../Features/Misc/misc.h"
#include "../../Features/Rage/Animations/Lagcompensation.h"

void Event::FireGameEvent(IGameEvent* pEvent) {

	if (!pEvent)
		return;

	misc::PreserveKillfeed(pEvent);
	misc::BuyBot(pEvent);
	lagcomp.ResolverHandler(pEvent);

	if ( !g::pLocal || !g::pLocal->IsAlive( ) )
		return;

	auto pEventInfo = i::ClientState->pEvents; //0x4E6C
	CEventInfo* pNextEvent = nullptr;

	if ( !pEventInfo )
		return;

	do
	{
		pNextEvent = pEventInfo->pNext;

		const uint16_t class_id = pEventInfo->iClassID - 1;

		const auto m_p_create_event_fn = pEventInfo->pClientClass->pCreateEventFn;
		if ( !m_p_create_event_fn )
			continue;

		const auto p_ce = m_p_create_event_fn( );
		if ( !p_ce )
			continue;

		pEventInfo->flFireDelay = 0.f;

		pEventInfo = pNextEvent;
	} while ( pNextEvent != nullptr );
}

bool __fastcall h::hkFireEvent( void* ecx, void* edx, IGameEvent* m_pEvent )
{
	static auto oFireEvent = detour::fireEvent.GetOriginal<decltype( &hkFireEvent )>( );

	if ( !g::pLocal || !g::pLocal->IsAlive( ) )
		return oFireEvent( ecx, edx, m_pEvent );

	auto pEventInfo = i::ClientState->pEvents; //0x4E6C
	CEventInfo* pNextEvent = nullptr;

	if ( !pEventInfo )
		return oFireEvent( ecx, edx, m_pEvent );

	do
	{
		pNextEvent = pEventInfo->pNext;

		const uint16_t class_id = pEventInfo->iClassID - 1;

		const auto m_p_create_event_fn = pEventInfo->pClientClass->pCreateEventFn;
		if ( !m_p_create_event_fn )
			continue;

		const auto p_ce = m_p_create_event_fn( );
		if ( !p_ce )
			continue;

		pEventInfo->flFireDelay = 0.f;

		pEventInfo = pNextEvent;
	}
	
	while ( pNextEvent != nullptr );
		return oFireEvent( ecx, edx, m_pEvent );
}