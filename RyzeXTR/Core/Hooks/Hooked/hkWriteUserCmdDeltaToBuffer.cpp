#include "../hooks.h"
#include "../../Features/Rage/exploits.h"
#include "../../SDK/Entity.h"
#include "../../globals.h"
#include "../../Features/Rage/antiaim.h"
#include "../../Features/Rage/ragebot.h"

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
		return detour::writeUserCmd.CallOriginal<bool>(ROP::ClientGadget_t::uReturnGadget, ecx, edx, nSlot, buf, nFrom, nTo, bNewCommand);

	int* pNumBackupCommands = (int*)((uintptr_t)(buf)-0x30);
	int* pNumNewCommands = (int*)((uintptr_t)(buf)-0x2C);

	int iExtraCommands = exploits::iDefensive;
	exploits::iDefensive = 0;

	int iNewCommands = *pNumNewCommands;
	int iNextCommand = i::ClientState->iLastOutgoingCommand + i::ClientState->nChokedCommands + 1;

	*pNumBackupCommands = 0;
	for (nTo = iNextCommand - iNewCommands + 1; nTo <= iNextCommand; nTo++)
	{
		if (!detour::writeUserCmd.CallOriginal<bool>(ROP::ClientGadget_t::uReturnGadget, ecx, edx, nSlot, buf, nFrom, nTo, true))
			return false;

		nFrom = nTo;
	}

	*pNumNewCommands = iNewCommands + iExtraCommands;

	CUserCmd* pCmd = i::Input->GetUserCmd(nFrom);
	if (!pCmd)
		return true;

	CUserCmd ToCmd = *pCmd;
	CUserCmd FromCmd = *pCmd;

	ToCmd.iCommandNumber++;
	ToCmd.iTickCount += ((int)(1.0f / i::GlobalVars->flIntervalPerTick));

	for (int i = 0; i < iExtraCommands; i++)
	{
		WriteUsercmd(buf, &ToCmd, &FromCmd);

		FromCmd = ToCmd;

		ToCmd.iCommandNumber++;
		ToCmd.iTickCount++;
	}

	return true;

	/*if (!exploits::iDefensive)
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

	return true;*/
}