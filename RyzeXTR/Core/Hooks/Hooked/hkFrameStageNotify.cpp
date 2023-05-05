#include "../hooks.h"
#include "../../Features/Misc/misc.h"
#include "../../Features/Rage/Animations/LocalAnimation.h"
#include "../../SDK/Menu/config.h"
#include "../../Features/Networking/networking.h"
#include "../../Features/Rage/Animations/Lagcompensation.h"

void hkPreFrameStageNotify(EStage curStage) {

	switch (curStage) {

	case FRAME_START:

		break;

	case FRAME_RENDER_END:

		break;

	case FRAME_RENDER_START:

		misc::ServerHitboxes();

		if (cfg::misc::removals[1])
			g::pLocal->GetFlashMaxAlpha() = 0.f;
		
		misc::BulletImpact(nullptr, curStage, true);
		break;

	case FRAME_NET_UPDATE_END:
		break;

	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:

		lagcomp.RemoveInterpolation();
		break;

	case FRAME_NET_UPDATE_POSTDATAUPDATE_START:

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
		if (!Config2->bSaving)
			lagcomp.FrameStageNotify();
		break;

	case FRAME_RENDER_START:

		break;
	}
}

void __fastcall h::hkFrameStageNotify(void* ecx, void* edx, EStage curStage) {

	static auto original = detour::frameStageNotify.GetOriginal<decltype(&h::hkFrameStageNotify)>();

	if (i::ClientState->iSignonState != SIGNONSTATE_FULL || !g::pLocal)
		return original(ecx, edx, curStage);

	hkPreFrameStageNotify(curStage);

	original(ecx, edx, curStage);

	hkPostFrameStageNotify(curStage);
}