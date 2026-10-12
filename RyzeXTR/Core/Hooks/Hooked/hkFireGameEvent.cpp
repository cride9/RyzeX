#include "../hooks.h"
#include "../../globals.h"
#include "../../Features/Misc/misc.h"
#include "../../Features/Rage/Animations/Lagcompensation.h"
#include "../../Features/Rage/Animations/EnemyAnimations.h"
#include "../../Features/Rage/Animations/ExperimentalResolver.h"
#include "../../Lua/Lua.h"

void Event::FireGameEvent(IGameEvent* pEvent) {

	if (i::ClientState->iSignonState != SIGNONSTATE_FULL)
		return;

	if (!pEvent)
		return;

	misc::EventHandler(pEvent);
	experimentalResolver.HandleGameEvent(pEvent);
	anims.ResolverHandler(pEvent);

	LuaImplementation::RunCallbacks( LuaImplementation::vecCallbackList[ LuaImplementation::CALLBACK_FIRE_GAME_EVENT ], pEvent );
	
	LuaImplementation::RunCallbacks( pEvent->GetName( ), pEvent );

	//auto pEventInfo = i::ClientState->pEvents; //0x4E6C
	//CEventInfo* pNextEvent = nullptr;

	//if ( !pEventInfo )
	//	return;

	//do
	//{
	//	pNextEvent = pEventInfo->pNext;

	//	const uint16_t class_id = pEventInfo->iClassID - 1;

	//	const auto m_p_create_event_fn = pEventInfo->pClientClass->pCreateEventFn;
	//	if ( !m_p_create_event_fn )
	//		continue;

	//	const auto p_ce = m_p_create_event_fn( );
	//	if ( !p_ce )
	//		continue;

	//	pEventInfo->flFireDelay = 0.f;

	//	pEventInfo = pNextEvent;
	//} while ( pNextEvent != nullptr );
}

bool __fastcall h::hkFireEvent( void* ecx, void* edx, IGameEvent* pEvent )
{
	static auto oFireEvent = detour::fireEvent.GetOriginal<decltype( &hkFireEvent )>( );

	if ( !g::pLocal || !g::pLocal->IsAlive( ) )
		return oFireEvent( ecx, edx, pEvent );

	auto pEventInfo = i::ClientState->pEvents; //0x4E6C

	if ( !pEventInfo )
		return oFireEvent( ecx, edx, pEvent );

	for (; pEventInfo != nullptr; pEventInfo = pEventInfo->pNext)
	{
		if (!pEventInfo->pClientClass)
			continue;

		const auto m_p_create_event_fn = pEventInfo->pClientClass->pCreateEventFn;
		if ( !m_p_create_event_fn )
			continue;

		const auto p_ce = m_p_create_event_fn( );
		if ( !p_ce )
			continue;

		pEventInfo->flFireDelay = 0.f;

	}
	return oFireEvent( ecx, edx, pEvent );
}
