#include "../hooks.h"
#include "../../Features/Misc/misc.h"
#include "../../Features/Rage/Animations/LocalAnimation.h"
#include "../../SDK/Menu/config.h"
#include "../../Features/Networking/networking.h"
#include "../../Features/Rage/Animations/Lagcompensation.h"
#include "../../Features/Changers/SkinChanger.h"
#include "../../Features/Changers/wtf.h"

void hkPreFrameStageNotify(EStage& curStage) {

	auto pLocal = CBaseEntity::GetLocalPlayer();
	if (!pLocal)
		return;

	//skinChanger.AgentChanger(curStage);
	switch (curStage) {

	case FRAME_START:

		break;

	case FRAME_RENDER_END:
		break;

	case FRAME_RENDER_START:

		misc::ServerHitboxes();

		if (cfg::misc::removals[1])
			pLocal->GetFlashMaxAlpha() = 0.f;
		else
			pLocal->GetFlashMaxAlpha() = 255.f;

		misc::BulletImpactFrameStage(pLocal);
		break;

	case FRAME_NET_UPDATE_END:
		lagcomp.FrameStageNotify();
		break;

	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
		break;

	case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
		beforeIfuckUpEverything::SetSkin(pLocal);
		break;

	default:
		break;
	}
}

void hkPostFrameStageNotify(EStage& curStage) {

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
	original(i::ClientDll, edx, curStage);
	hkPostFrameStageNotify(curStage);
}