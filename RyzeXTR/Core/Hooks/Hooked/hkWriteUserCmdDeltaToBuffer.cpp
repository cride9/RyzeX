#include "../hooks.h"
#include "../../Features/Rage/exploits.h"
#include "../../SDK/Entity.h"
#include "../../globals.h"
#include "../../Features/Rage/antiaim.h"

static void WriteUsercmd(void* buf, CUserCmd* Cin, CUserCmd* Cout)
{
	using WriteUserCmd_t = void(__fastcall*)(void*, CUserCmd*, CUserCmd*);
	static auto Fn = (WriteUserCmd_t)MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 83 E4 F8 51 53 56 8B D9"));

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

	if (!exploits::iDefensive)
		return original(ecx, edx, nSlot, buf, nFrom, nTo, bNewCommand);

	if (nFrom != -1)
		return true;

	auto pNewCommands = (int*)((DWORD)buf - 0x2C);
	auto pBackupCommands = (int*)((DWORD)buf - 0x30);
	auto iNewCommands = *pNewCommands;

	auto iNextCmd = i::ClientState->nChokedCommands + i::ClientState->iLastOutgoingCommand + 1;

	auto iTotalNewCommands = std::clamp(exploits::iDefensive, 0, 16);
	exploits::iDefensive -= iTotalNewCommands;

	nFrom = -1;

	*pNewCommands = iTotalNewCommands;
	*pBackupCommands = 0;

	for (nTo = iNextCmd - iNewCommands + 1; nTo <= iNextCmd; nTo++)
	{
		if (!original(ecx, edx, nSlot, buf, nFrom, nTo, true))
			return false;

		nFrom = nTo;
	}

	CUserCmd* pRealCommand = i::Input->GetUserCmd(nFrom);
	CUserCmd pFromCmd;

	if (pRealCommand)
		memcpy(&pFromCmd, pRealCommand, sizeof(CUserCmd));

	CUserCmd pToCmd;
	memcpy(&pToCmd, &pFromCmd, sizeof(CUserCmd));

	pToCmd.iCommandNumber++;
	pToCmd.iTickCount += 200;

	for (int i = iNewCommands; i <= iTotalNewCommands; i++)
	{
		WriteUsercmd(buf, &pToCmd, &pFromCmd);
		memcpy(&pFromCmd, &pToCmd, sizeof(CUserCmd));
		pToCmd.iCommandNumber++;
		pToCmd.iTickCount++;
	}

	return true;
}