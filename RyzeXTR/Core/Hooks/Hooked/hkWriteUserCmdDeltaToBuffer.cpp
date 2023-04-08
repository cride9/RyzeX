#include "../hooks.h"
#include "../../Features/Rage/exploits.h"
#include "../../SDK/Entity.h"
#include "../../globals.h"

static void WriteUsercmd(void* buf, CUserCmd* Cin, CUserCmd* Cout)
{
	using WriteUserCmd_t = void(__fastcall*)(void*, CUserCmd*, CUserCmd*);
	static auto Fn = (WriteUserCmd_t)util::FindSignature("client.dll", "55 8B EC 83 E4 F8 51 53 56 8B D9");

	__asm
	{
		mov     ecx, buf
		mov     edx, Cin
		push    Cout
		call    Fn
		add     esp, 4
	}
}

bool __fastcall h::hkWriteUserCmdDeltaToBuffer(void* ecx, void* edx, int nSlot, bf_write* buf, int nFrom, int nTo, bool bNewCommand) {

	static auto original = detour::writeUserCmd.GetOriginal<decltype(&h::hkWriteUserCmdDeltaToBuffer)>();

	if (!g::pLocal || !g::pLocal->IsAlive())
		return original(ecx, edx, nSlot, buf, nFrom, nTo, bNewCommand);

	if (!exploits::iDefensive)
		return original(ecx, edx, nSlot, buf, nFrom, nTo, bNewCommand);

	if (nFrom != -1)
		return true;

	auto p_new_commands = (int*)((DWORD)buf - 0x2C);
	auto p_backup_commands = (int*)((DWORD)buf - 0x30);
	auto new_commands = *p_new_commands;

	auto next_cmd_nr = i::ClientState->nChokedCommands + i::ClientState->iLastOutgoingCommand + 1;

	auto total_new_commands = std::clamp(exploits::iDefensive, 0, 16);
	exploits::iDefensive -= total_new_commands;

	nFrom = -1;

	*p_new_commands = total_new_commands;
	*p_backup_commands = 0;

	for (nTo = next_cmd_nr - new_commands + 1; nTo <= next_cmd_nr; nTo++)
	{
		if (!original(ecx, edx, nSlot, buf, nFrom, nTo, true))
			return false;

		nFrom = nTo;
	}

	CUserCmd* last_real_cmd = i::Input->GetUserCmd(nFrom);
	CUserCmd from_cmd;

	if (last_real_cmd)
		memcpy(&from_cmd, last_real_cmd, sizeof(CUserCmd));

	CUserCmd to_cmd;
	memcpy(&to_cmd, &from_cmd, sizeof(CUserCmd));

	to_cmd.iCommandNumber++;
	to_cmd.iTickCount++;

	for (int i = new_commands; i <= total_new_commands; i++)
	{
		WriteUsercmd(buf, &to_cmd, &from_cmd);
		memcpy(&from_cmd, &to_cmd, sizeof(CUserCmd));
		to_cmd.iCommandNumber++;
		to_cmd.iTickCount++;
	}

	return true;
}