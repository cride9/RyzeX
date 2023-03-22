#include "hooks.h"
#include <stdexcept>
#include "../globals.h"
#include "../SDK/Menu/gui.h"
#include "../SDK/DataTyes/Color.h"

void h::SetupHooks() {

	if (MH_init() != MH_OK)
		throw std::runtime_error("Failed to initialize minhook");

	gameevent.Init();

	// ImGui Hooks
	HookTable(detour::D3DReset, menu::device, table::D3DReset, &Reset);
	HookTable(detour::D3DEndScene, menu::device, table::D3DEndScene, &EndScene);

	// CSGO Hooks
	HookTable(detour::allocKeyValues, i::KeyValuesSystem, table::allocKeyValues, &hkAllocKeyValuesMemory);
	HookTable(detour::createMove, i::ClientDll, table::createMove, &hkCreateMoveProxy);

	HookTable(detour::frameStageNotify, i::ClientDll, table::frameStageNotify, &hkFrameStageNotify);
	HookTable(detour::overrideView, i::ClientMode, table::overrideView, &hkOverrideView);
	HookTable(detour::listLeaves, i::EngineClient->GetBSPTreeQuery(), table::listLeaves, &hkListLeavesInBox);
	HookTable(detour::paintTraverse, i::Panel, table::paintTraverse, &hkPaintTraverse);
	HookTable(detour::drawModel, i::StudioRender, table::drawModel, &hkDrawModel);
	HookTable(detour::lockCursor, i::Surface, table::lockCursor, &hkLockCursor);
	HookTable(detour::runCommand, i::Prediction, table::runCommand, &hkRunCommand);
	HookTable(detour::getViewmodelFov, i::ClientMode, table::getViewmodelFov, &hkGetViewModelFov);
	//HookTable(detour::isPaused, i::EngineClient, table::isPaused, &hkIsPaused);
	HookTable(detour::writeUserCmd, i::ClientDll, table::writeUserCmd, &hkWriteUserCmdDeltaToBuffer);
	HookTable(detour::fireEvent, i::GameEvent, table::fireEvent, &hkFireEvent);

	// Signature hooks
	HookSignature(detour::clMove, "engine.dll", "55 8B EC 81 EC ? ? ? ? 53 56 8A F9", &hkClMove);
	HookSignature(detour::buildTransform, "client.dll", "55 8B EC 83 E4 F0 81 ? ? ? ? ? 56 57 8B F9 8B ? ? ? ? ? 89 7C 24 28", &hkBuildTransformation);
	HookSignature(detour::calculateView, "client.dll", "55 8B EC 83 EC 14 53 56 57 FF 75 18", &hkCalculateView);
	HookSignature(detour::sequenceChange, "client.dll", "55 8B EC 51 53 8B 5D 08 56 8B F1 57 85", &hkCheckForSequenceChange);
	//HookSignature(detour::procedrualFoot, "client.dll", "55 8B EC 83 E4 F0 83 EC 78 56 8B F1 57 8B", &hkDoProceduralFootPlant);
	HookSignature(detour::isHltv, "engine.dll", "A1 ? ? ? ? 80 ? ? ? ? ? ? 75 0C", &hkIsHltv);
	HookSignature(detour::modifyEyePosition, "client.dll", "55 8B EC 83 E4 F8 83 EC 70 56 57 8B F9 89 7C 24 14", &hkModifyEyePosition);
	HookSignature(detour::skipAnimation, "client.dll", "57 8B F9 8B 07 8B 80 ? ? ? ? FF D0 84 C0 75 02", &hkShouldSkipAnimationFrame);
	HookSignature(detour::blendingRules, "client.dll", "55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 8B 75 08 57 8B F9 85 F6", &hkStandardBlendingRules);
	//HookSignature(detour::animationState, "client.dll", "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3", &hkUpdateAnimationState);
	HookSignature(detour::clientAnimation, "client.dll", "55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74", &hkUpdateClientSideAnimations);
	HookSignature(detour::setupBones, "client.dll", "55 8B EC 83 E4 F0 B8 D8", &hkSetupBones);
	HookSignature(detour::extraBoneProcessing, "client.dll", "55 8B EC 83 E4 F8 81 ? ? ? ? ? 53 56 8B F1 57 89 74 24 1C", &hkDoExtraBoneProcessing);
	HookSignature(detour::physicsSimulate, "client.dll", "56 8B F1 8B 8E ? ? ? ? 83 F9 FF 74 23", &hkPhysicsSimulate);
	HookSignature(detour::interpolateEntites, "client.dll", "55 8B EC 83 EC 1C 8B 0D ? ? ? ? 53 56 57", &hkInterpolateServerEntites);
	//HookSignature(detour::isFollowingEntity, "client.dll", "F6 ? ? ? ? ? ? 74 31 80", &hkIsFollowingEntity);

	menu::DestroyDirectX();

	util::LogConsole("Hooks Initialized!\n", Color(255, 255, 255));

	util::Print("Hooks initialized!");
}

void h::DestroyHooks() {

	gameevent.Destroy();

	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);
	MH_Uninit();
}