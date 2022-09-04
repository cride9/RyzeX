#include "../hooks.h"
#include "../../globals.h"
#include "../../Features/Misc/misc.h"
#include "../../Features/Rage/Animations/resolver.h"

void Event::FireGameEvent(IGameEvent* pEvent) {

	if (!pEvent)
		return;

	misc::PreserveKillfeed(pEvent);
	misc::BuyBot(pEvent);
	resolver::EventManager(pEvent);
}