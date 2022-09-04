#include "../hooks.h"
#include "../../Features/Misc/misc.h"
#include "../../Features/Rage/Animations/LocalAnimation.h"
#include "../../Features/Rage/Animations/Lagcompensation.h"
#include "../../Features/Rage/doubletap.h"
#include "../../SDK/Menu/config.h"

void hkPreFrameStageNotify(EStage curStage) {

	switch (curStage) {

	case FRAME_RENDER_START:

		if (cfg::misc::removals[1]) { // need menu element
			if (g::pLocal)
				*g::pLocal->GetFlashMaxAlpha() = 0.f;
		}

		misc::NightMode();

		break;

	case FRAME_NET_UPDATE_END:

		if (g::bRestoreGlobals) {

			g::pLocal->GetTickBase() = g::nRestoreTickbase;
			i::GlobalVars->flCurrentTime = g::flRestoreCurtime;

			g::bRestoreGlobals = false;
		}
		break;

	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:

		break;

	case FRAME_NET_UPDATE_POSTDATAUPDATE_START:

		localanim.FixVelocityModifer();
		break;
	}
}

void hkPostFrameStageNotify(EStage curStage) {

	switch (curStage) {

	case FRAME_NET_UPDATE_START:

		break;

	case FRAME_NET_UPDATE_POSTDATAUPDATE_START: // lmao

		break;

	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:

		break;

	case FRAME_NET_UPDATE_END:

		lagcomp.PostPlayerUpdate();
		break;

	case FRAME_RENDER_START:

		break;
	}
}

void __fastcall h::hkFrameStageNotify(void* ecx, void* edx, EStage curStage) {

	static auto original = detour::frameStageNotify.GetOriginal<decltype(&h::hkFrameStageNotify)>();

	hkPreFrameStageNotify(curStage);

	original(ecx, edx, curStage);

	hkPostFrameStageNotify(curStage);
}