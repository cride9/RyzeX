#include "hooks.h"
#include <stdexcept>
#include "../globals.h"
#include "../SDK/Menu/gui.h"
#include "../SDK/DataTyes/Color.h"
#include "../SDK/Menu/gui.h"
#include "../../Dependecies/ImGui/imgui_impl_win32.h"

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
	//HookTable(detour::fireEvent, i::GameEvent, table::fireEvent, &hkFireEvent);
	HookTable(detour::doPostScreenEffects, i::ClientMode, table::doPostScreenEffects, &hkDoPostScreenEffect);
	HookTable(detour::drawModelMdl, i::ModelRender, table::drawModelMdl, &hkDrawModelMDL);
	HookTable(detour::emitSound, i::EngineSoundClient, table::emitSound, &hkEmitSound);
	//HookTable(detour::installStringTableCallback, i::ClientDll, table::installStringTableCallback, &hkInstallStringTableCallback);

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
	HookSignature(detour::drawViewmodel, "client.dll", "55 8B EC 51 57 E8", hkShouldDrawViewmodel);
	//HookSignature(detour::emitSound, "client.dll", "8D 8F ? ? ? ? F3 0F 10 84 24 ? ? ? ? 50", &hkEmitSound, 0x2);
	HookSignature(detour::isFollowingEntity, "client.dll", "F6 ? ? ? ? ? ? 74 31 80", &hkIsFollowingEntity);
	HookSignature(detour::playerMove, "client.dll", "55 8B EC 83 EC 08 56 8B F1 8B 8E 54 0E 00 00 E8", &hkPlayerMove);
	HookSignature(detour::clampBonesInBBox, "client.dll", "55 8B EC 83 E4 F8 83 EC 70 56 57 8B F9 89 7C 24 38", &hkClampBonesInBBox);
	HookSignature(detour::getColorModulation, "materialsystem.dll", "55 8B EC 83 EC ? 56 8B F1 8A 46", &hkGetColorModulation);
	//HookSignature(detour::startSound, "engine.dll", "55 8B EC 81 ? ? ? ? ? 53 56 8B F1 57 8B 7E", &hkStartSound);

	menu::DestroyDirectX();

	//util::Print("Hooks initialized!");

	//i::EngineClient->ExecuteClientCmd("toggleconsole");
	//i::EngineClient->ExecuteClientCmd("clear");
	//util::LogConsole("Hooks Initialized!\n", Color(255, 255, 255));
}


void __fastcall h::hkInstallStringTableCallback(const char* tableName) {

	return;
}

void h::HookNetChannel(INetChannel* pNetChannel) {

	// netchannel pointer
	if (!pNetChannel)
		return;

	// @note: doesnt need rehook cuz detours here
	if (pNetChannel != nullptr)
	{
		if (!detour::processPacket.IsHooked())
			h::HookTable(detour::processPacket, pNetChannel, table::processPacket, &h::hkProcessPacket);

		if (!detour::sendNetMsg.IsHooked())
			h::HookTable(detour::sendNetMsg, pNetChannel, table::sendNetMsg, &h::hkSendNetMsg);

		if ( !detour::setChoked.IsHooked( ) )
			h::HookTable(detour::setChoked, pNetChannel, table::setChoked, &h::hkSetChoked);

		if (!detour::sendDatagram.IsHooked())
			h::HookTable(detour::sendDatagram, pNetChannel, table::sendDatagram, &h::hkSendDatagram);
	}
}

void h::HookClientState() {

	static const auto clientStateHookable = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(i::ClientState) + 0x8);

	if (clientStateHookable != nullptr)
	{
		oldpacketStart = util::GetVFunc(clientStateHookable, table::packetStart);
		oldpacketEnd = util::GetVFunc(clientStateHookable, table::packetEnd);
		oldtemptEntities = util::GetVFunc(clientStateHookable, table::temptEntities);

		// PacketStart Detour
		if (!detour::packetStart.IsHooked())
			h::HookTable(detour::packetStart, clientStateHookable, table::packetStart, &h::hkPacketStart);

		// PacketEnd Detour
		if (!detour::packetEnd.IsHooked())
			h::HookTable(detour::packetEnd, clientStateHookable, table::packetEnd, &h::hkPacketEnd);

		if (!detour::temptEntities.IsHooked())
			h::HookTable(detour::temptEntities, clientStateHookable, table::temptEntities, &h::hkTemptEntities);
	}
}

void h::UnHookClientState() {

	static const auto clientStateHookable = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(i::ClientState) + 0x8);

	if (clientStateHookable != nullptr)
	{
		// PacketStart Detour
		if (detour::packetStart.IsHooked()) {
			MH_DisableHook(oldpacketStart);
			MH_RemoveHook(oldpacketStart);
		}
			
		// PacketEnd Detour
		if (detour::packetEnd.IsHooked()) {
			MH_DisableHook(oldpacketEnd);
			MH_RemoveHook(oldpacketEnd);
		}

		if (detour::temptEntities.IsHooked()) {
			MH_DisableHook(oldtemptEntities);
			MH_RemoveHook(oldtemptEntities);
		}
	}
}

void h::DestroyHooks() {

	gameevent.Destroy();

	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);
	MH_Uninit();
}