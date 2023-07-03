#include "../hooks.h"
#include "../../Features/Visuals/ESP.h"
#include "../../Features/Rage/Animations/LocalAnimation.h"
#include "../../Features/Rage/ragebot.h"
#include "../../Features/Misc/enginepred.h"
#include "../../Features/Misc/misc.h"
#include "../../Features/Misc/serversounds.h"

void __fastcall h::hkPaintTraverse(uintptr_t pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce) {

	static auto original = detour::paintTraverse.GetOriginal<decltype(&h::hkPaintTraverse)>();

	if (cfg::misc::removals[3] && !strcmp("HudZoom", i::Panel->GetName(vguiPanel)))
		return;

	if (vguiPanel == i::EngineVGui->GetPanel(PANEL_TOOLS)) {

#if NDEBUG
		//misc::Print("[USER] Built date: " __DATE__ " at " __TIME__ "\n");
		if (g::bStartWelcome)
			visual::WelcomeUser("[USER] Built date: " __DATE__ " at " __TIME__ "\n");
#endif
#if _DEBUG
		//misc::Print("[DEVELOPER] Built date: " __DATE__ " at " __TIME__ "\n");
		if (g::bStartWelcome)
			visual::WelcomeUser("[DEVELOPER] Built date: " __DATE__ " at " __TIME__ "\n");
#endif
#if ALPHA
		//misc::Print("[ALPHA] Built date: " __DATE__ " at " __TIME__ "\n");
		if (g::bStartWelcome)
			visual::WelcomeUser("[ALPHA] Built date: " __DATE__ " at " __TIME__ "\n");
#endif
		serversound.Start();
		visual::VisualRender();
		serversound.Finish();

		visual::WorldEsp();
		visual::CoolHackKeyBindList();
		visual::DrawRadioInformation();

		for (Vector& drawPos : g::drawList)
		{
			Vector screenPosition;
			if (i::DebugOverlay->ScreenPosition(drawPos, screenPosition))
				continue;

			i::Surface->DrawSetColor(Color(255, 255, 255, 255));
			i::Surface->DrawFilledRect(screenPosition.x - 2, screenPosition.y - 2, screenPosition.x + 2, screenPosition.y + 2);
		}

		if (g::drawList.size() > 12)
			g::drawList.clear();

		if (g::pLocal) {

			if (cfg::misc::removals[3] && g::pLocal->IsScoped() && i::EngineClient->IsInGame()) {

				int iHeight = 0;
				int iWidth = 0;
				i::EngineClient->GetScreenSize(iWidth, iHeight);

				float flSize = iHeight * (cfg::misc::scopeLength / 100.f);

				i::Surface->DrawSetColor(cfg::misc::scopeColor);
				i::Surface->DrawFilledRectFade(
					iWidth / 2 - flSize - 15,
					iHeight / 2 - 1,
					iWidth / 2 - 15,
					iHeight / 2,
					0, cfg::misc::scopeColor[3] * 255.f, true);

				i::Surface->DrawSetColor(Color(cfg::misc::scopeColorEnd));
				i::Surface->DrawFilledRectFade(
					iWidth / 2 - flSize - 15,
					iHeight / 2 - 1,
					iWidth / 2 - 15,
					iHeight / 2,
					cfg::misc::scopeColorEnd[3] * 255.f, 0, true);

				i::Surface->DrawSetColor(cfg::misc::scopeColor);
				i::Surface->DrawFilledRectFade(
					iWidth / 2 + 15,
					iHeight / 2 - 1,
					iWidth / 2 + flSize + 15,
					iHeight / 2,
					cfg::misc::scopeColor[3] * 255.f, 0, true);

				i::Surface->DrawSetColor(Color(cfg::misc::scopeColorEnd));
				i::Surface->DrawFilledRectFade(
					iWidth / 2 + 15,
					iHeight / 2 - 1,
					iWidth / 2 + flSize + 15,
					iHeight / 2,
					0, cfg::misc::scopeColorEnd[3] * 255.f, true);

				i::Surface->DrawSetColor(cfg::misc::scopeColor);
				i::Surface->DrawFilledRectFade(
					iWidth / 2 - 1,
					iHeight / 2 - flSize - 15,
					iWidth / 2,
					iHeight / 2 - 15,
					0, cfg::misc::scopeColor[3] * 255.f, false);

				i::Surface->DrawSetColor(Color(cfg::misc::scopeColorEnd));
				i::Surface->DrawFilledRectFade(
					iWidth / 2 - 1,
					iHeight / 2 - flSize - 15,
					iWidth / 2,
					iHeight / 2 - 15,
					cfg::misc::scopeColorEnd[3] * 255.f, 0, false);

				i::Surface->DrawSetColor(cfg::misc::scopeColor);
				i::Surface->DrawFilledRectFade(
					iWidth / 2 - 1,
					iHeight / 2 + 15,
					iWidth / 2,
					iHeight / 2 + flSize + 15,
					cfg::misc::scopeColor[3] * 255.f, 0, false);

				i::Surface->DrawSetColor(Color(cfg::misc::scopeColorEnd));
				i::Surface->DrawFilledRectFade(
					iWidth / 2 - 1,
					iHeight / 2 + 15,
					iWidth / 2,
					iHeight / 2 + flSize + 15,
					0, cfg::misc::scopeColorEnd[3] * 255.f, false);

				//i::Surface->DrawLine( // vertical
				//	iWidth / 2, 
				//	iHeight / 2 - flSize,
				//	iWidth / 2, 
				//	iHeight / 2 + flSize);
				//i::Surface->DrawLine( // horizontal
				//	iWidth / 2 - flSize,
				//	iHeight / 2, 
				//	iWidth / 2 + flSize,
				//	iHeight / 2);
			}
		}
	}

	
	original(pPanels, edx, vguiPanel, forceRepaint, allowForce);
}