#include "../hooks.h"
#include "../../Features/Visuals/ESP.h"
#include "../../Features/Rage/ragebot.h"
#include "../../Features/Rage/Animations/LocalAnimation.h"

void __fastcall h::hkPaintTraverse(uintptr_t pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce) {

	static auto original = detour::paintTraverse.GetOriginal<decltype(&h::hkPaintTraverse)>();

	// need menu element
	if (cfg::misc::removals[3] && !strcmp("HudZoom", i::Panel->GetName(vguiPanel)))
		return;

	if (vguiPanel == i::EngineVGui->GetPanel(PANEL_TOOLS)) {

		visual::VisualRender();

		if (g::pLocal) {

			if (cfg::misc::removals[3] && g::pLocal->IsScoped() && i::EngineClient->IsInGame()) {

				int iHeight = 0;
				int iWidth = 0;
				i::EngineClient->GetScreenSize(iWidth, iHeight);

				i::Surface->DrawSetColor(Color(0, 0, 0, 255));
				i::Surface->DrawLine(iWidth / 2, 0, iWidth / 2, iHeight);
				i::Surface->DrawLine(0, iHeight / 2, iWidth, iHeight / 2);
			}
		}
	}

	
	original(pPanels, edx, vguiPanel, forceRepaint, allowForce);
}