#include "../hooks.h"
#include "../../Features/Visuals/ESP.h"
#include "../../Features/Rage/Animations/LocalAnimation.h"
#include "../../Features/Rage/ragebot.h"
#include "../../Features/Misc/enginepred.h"
#include "../../Features/Misc/misc.h"
#include "../../Features/Misc/serversounds.h"
#include "../../SDK/Draw.h"

void __fastcall h::hkPaintTraverse(uintptr_t pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce) {

	static auto original = detour::paintTraverse.GetOriginal<decltype(&h::hkPaintTraverse)>();

	if (cfg::misc::removals[5] && !strcmp("HudZoom", i::Panel->GetName(vguiPanel)))
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

		D::ClearDrawData( );

		serversound.Start();
		visual::VisualRender();
		serversound.Finish();

		visual::WorldEsp();
		visual::CoolHackKeyBindList();
		visual::DrawRadioInformation();

		D::SwapDrawData( );

		if (g::pLocal && i::Input->bCameraInThirdPerson) {

			Vector vecHeadPosition = g::pLocal->GetBonePosition(BONE_HEAD).value();

			float flStep = (2 * M_PI) / 18.f;
			float flRadius = 9;

			for (float rotation = 0; rotation < (M_PI * 2.0); rotation += flStep) {

				Vector topRim(flRadius * cos(rotation) + vecHeadPosition.x, flRadius * sin(rotation) + vecHeadPosition.y, vecHeadPosition.z + 2);
				Vector bottomRim(flRadius / 2 * cos(rotation) + vecHeadPosition.x, flRadius / 2.f * sin(rotation) + vecHeadPosition.y, vecHeadPosition.z + 8);

				Vector topRimPredict(flRadius * cos(rotation + flStep) + vecHeadPosition.x, flRadius * sin(rotation + flStep) + vecHeadPosition.y, vecHeadPosition.z + 2);
				Vector bottomRimPredict(flRadius / 2 * cos(rotation + flStep) + vecHeadPosition.x, flRadius / 2.f * sin(rotation + flStep) + vecHeadPosition.y, vecHeadPosition.z + 8);

				Vector topOnScreen, botOnScreen, topOnScreenPredict, botOnScreenPredict;
				if (!i::DebugOverlay->ScreenPosition(topRim, topOnScreen))
				{
					if (!i::DebugOverlay->ScreenPosition(bottomRim, botOnScreen)) {

						i::Surface->DrawSetColor(255, 255, 255, 255);

						i::Surface->DrawLine(topOnScreen.x, topOnScreen.y, botOnScreen.x, botOnScreen.y);

						if (!i::DebugOverlay->ScreenPosition(topRimPredict, topOnScreenPredict))
							i::Surface->DrawLine(topOnScreen.x, topOnScreen.y, topOnScreenPredict.x, topOnScreenPredict.y);

						if (!i::DebugOverlay->ScreenPosition(bottomRimPredict, botOnScreenPredict))
							i::Surface->DrawLine(botOnScreen.x, botOnScreen.y, botOnScreenPredict.x, botOnScreenPredict.y);

					}
				}
			}
		}

		for (Vector& drawPos : g::drawList)
		{
			Vector screenPosition;
			if (i::DebugOverlay->ScreenPosition(drawPos, screenPosition))
				continue;

			i::Surface->DrawSetColor(Color(255, 255, 255, 255));
			i::Surface->DrawFilledRect(screenPosition.x - 2, screenPosition.y - 2, screenPosition.x + 2, screenPosition.y + 2);
		}

		while (g::drawList.size() > 24)
			g::drawList.pop_front();

		if (g::pLocal) {

			if (cfg::misc::removals[5] && g::pLocal->IsScoped() && i::EngineClient->IsInGame()) {

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