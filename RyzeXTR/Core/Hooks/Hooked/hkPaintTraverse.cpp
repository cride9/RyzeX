#include "../hooks.h"
#include "../../Features/Visuals/ESP.h"
#include "../../Features/Rage/Animations/LocalAnimation.h"
#include "../../Features/Misc/enginepred.h"
#include "../../Features/Misc/misc.h"
#include "../../Features/Misc/serversounds.h"
#include "../../SDK/Draw.h"

void __fastcall h::hkPaintTraverse(void* pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce) {

	static auto original = detour::paintTraverse.GetOriginal<decltype(&h::hkPaintTraverse)>();

	if (cfg::misc::bRemovals[5] && !strcmp("HudZoom", i::Panel->GetName(vguiPanel)))
		return;

	if (vguiPanel == i::EngineVGui->GetPanel(PANEL_TOOLS)) {

		//if (g::pLocal) {

		//	i::Surface->DrawT(1200, 500, Color(255, 255, 255), g::fonts::DebugFont, true, std::to_string(reinterpret_cast<uintptr_t>(g::pLocal)).c_str());
		//	i::Surface->DrawT(1200, 700, Color(255, 255, 255), g::fonts::DebugFont, true, std::to_string(reinterpret_cast<uintptr_t>(g::pLocal->GetClientRenderable())).c_str());
		//}

#if RELEASE
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

		//D::ClearDrawData( );
		serversound.Start();
		visual::VisualRender();
		serversound.Finish();

		visual::WorldEsp();
		visual::CoolHackKeyBindList();
		visual::DrawRadioInformation();
		visual::CustomHud();
		visual::Hat();
		visual::CrossHair();
		visual::AutoPeekCircle();
		visual::HandleDamageIndicator();
#ifdef _DEBUG
		visual::DrawList();
#endif
		//D::SwapDrawData( );
	}

	original(pPanels, edx, vguiPanel, forceRepaint, allowForce);
}