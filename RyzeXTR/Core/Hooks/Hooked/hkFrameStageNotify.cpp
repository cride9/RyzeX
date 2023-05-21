#include "../hooks.h"
#include "../../Features/Misc/misc.h"
#include "../../Features/Rage/Animations/LocalAnimation.h"
#include "../../SDK/Menu/config.h"
#include "../../Features/Networking/networking.h"
#include "../../Features/Rage/Animations/Lagcompensation.h"
#include "../../Features/Changers/skinchanger.h"

void hkPreFrameStageNotify(EStage& curStage) {

	if (!g::pLocal)
		return;

	switch (curStage) {

	case FRAME_START:

		break;

	case FRAME_RENDER_END:
		//i::Prediction->SetLocalViewAngles();
		break;

	case FRAME_RENDER_START:

		misc::ServerHitboxes();

		if (cfg::misc::removals[1])
			g::pLocal->GetFlashMaxAlpha() = 0.f;
		else
			g::pLocal->GetFlashMaxAlpha() = 255.f;

		misc::BulletImpactFrameStage();
		break;

	case FRAME_NET_UPDATE_END:
		lagcomp.FrameStageNotify();
		break;

	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
		if (cfg::rage::enable)
			lagcomp.RemoveInterpolation();
		break;

	case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
		break;

	default:
		break;
	}
}

void hkPostFrameStageNotify(EStage& curStage) {

	if (!(i::ClientState->iDeltaTick > 0))
		return;

	if (!g::pLocal)
		return;

	switch (curStage) {

	case FRAME_NET_UPDATE_START:

		break;

	case FRAME_NET_UPDATE_POSTDATAUPDATE_START: // lmao

		break;

	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:

		break;

	case FRAME_NET_UPDATE_END:
		break;

	case FRAME_RENDER_START:

		break;

	default:
		break;
	}
}

void __fastcall h::hkFrameStageNotify(IBaseClientDLL* ecx, int edx, EStage curStage) {

	static auto original = detour::frameStageNotify.GetOriginal<decltype(&h::hkFrameStageNotify)>();

	hkPreFrameStageNotify(curStage);
	original(ecx, edx, curStage);
	hkPostFrameStageNotify(curStage);
}