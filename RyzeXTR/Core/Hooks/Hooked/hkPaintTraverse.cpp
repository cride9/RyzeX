#include "../hooks.h"
#include "../../Features/Visuals/ESP.h"
#include "../../Features/Rage/Animations/LocalAnimation.h"
#include "../../Features/Rage/ragebot.h"

void __fastcall h::hkPaintTraverse(uintptr_t pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce) {

	static auto original = detour::paintTraverse.GetOriginal<decltype(&h::hkPaintTraverse)>();

	if (cfg::misc::removals[3] && !strcmp("HudZoom", i::Panel->GetName(vguiPanel)))
		return;

	if (i::ClientState->iSignonState != SIGNONSTATE_FULL) {
		g::pLocal = nullptr;
		if (g::bSendPacket)
			*g::bSendPacket = true;
	}

	if (vguiPanel == i::EngineVGui->GetPanel(PANEL_TOOLS)) {

		visual::VisualRender();

		if (g::pLocal) {

			if (cfg::misc::removals[3] && g::pLocal->IsScoped() && i::EngineClient->IsInGame()) {

				int iHeight = 0;
				int iWidth = 0;
				i::EngineClient->GetScreenSize(iWidth, iHeight);

				i::Surface->DrawSetColor(cfg::misc::scopeColor);
				
				float flSize = iHeight * (cfg::misc::scopeLength / 100.f);

				i::Surface->DrawLine( // vertical
					iWidth / 2, 
					iHeight / 2 - flSize,
					iWidth / 2, 
					iHeight / 2 + flSize);
				i::Surface->DrawLine( // horizontal
					iWidth / 2 - flSize,
					iHeight / 2, 
					iWidth / 2 + flSize,
					iHeight / 2);
			}
		}
	}

	
	original(pPanels, edx, vguiPanel, forceRepaint, allowForce);
}