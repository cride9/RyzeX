#include "../hooks.h"

bool __fastcall h::hkWriteUserCmdDeltaToBuffer(void* ecx, void* edx, int slot, bf_write* buf, int from, int to, bool isNew) {
	static auto original = detour::writeUserCmd.GetOriginal<decltype(&h::hkWriteUserCmdDeltaToBuffer)>();
	return original(ecx, edx, slot, buf, from, to, isNew);
}
