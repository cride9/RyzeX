#include "../hooks.h"
#include "../../Features/Rage/exploits.h"
#include "../../SDK/Entity.h"
#include "../../globals.h"
#include <limits>

using WriteUserCmd_t = void(__fastcall*)(void*, CUserCmd*, CUserCmd*);
static WriteUserCmd_t SyntheticWriter() {
    static auto fn = reinterpret_cast<WriteUserCmd_t>(MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 83 E4 F8 51 53 56 8B D9")));
    return fn;
}
static void WriteUsercmd(void* buf, CUserCmd* to, CUserCmd* from) {
    auto fn = SyntheticWriter();
    __asm {
        mov ecx, buf
        mov edx, to
        push from
        call fn
        add esp, 4
    }
}

bool __fastcall h::hkWriteUserCmdDeltaToBuffer(void* ecx, void* edx, int slot, bf_write* buf, int from, int to, bool isNew) {
    static auto original = detour::writeUserCmd.GetOriginal<decltype(&h::hkWriteUserCmdDeltaToBuffer)>();
    if (!i::ClientState || !buf) return original(ecx, edx, slot, buf, from, to, isNew);
    const int sequence = i::ClientState->pNetChannel ? i::ClientState->pNetChannel->iOutSequenceNr : 0;
    const int next = i::ClientState->iLastOutgoingCommand + i::ClientState->nChokedCommands + 1;
    auto& cycle = exploits::serialization;
    if (from == -1) cycle.Begin(buf, sequence);
    else if (cycle.Owns(buf, sequence)) return true;
    const int extra = exploits::iDefensive + exploits::iHideShiftAmount;
    if (!extra || from != -1) {
        const bool ok = original(ecx, edx, slot, buf, from, to, isNew);
        if (!ok) exploits::OnSerializationFailed();
        else if (isNew && to == next && !extra) {
            const int real = std::clamp(i::ClientState->nChokedCommands + 1, 1, tickshift::MaxNewCommands);
            return exploits::OnSerialized(next - real + 1, next, real, 0);
        }
        return ok;
    }

    // Legacy target-build stack contract. Plausibility checks fail closed; these
    // offsets still require verification against the running client DLL.
    MEMORY_BASIC_INFORMATION memory{};
    auto* header = reinterpret_cast<unsigned char*>(buf) - 0x30;
    if (!VirtualQuery(header, &memory, sizeof(memory)) || memory.State != MEM_COMMIT
        || (memory.Protect & (PAGE_GUARD | PAGE_NOACCESS))
        || reinterpret_cast<uintptr_t>(buf) + sizeof(bf_write) > reinterpret_cast<uintptr_t>(memory.BaseAddress) + memory.RegionSize
        || !((memory.Protect & 0xff) == PAGE_READWRITE || (memory.Protect & 0xff) == PAGE_WRITECOPY
            || (memory.Protect & 0xff) == PAGE_EXECUTE_READWRITE || (memory.Protect & 0xff) == PAGE_EXECUTE_WRITECOPY)) {
        exploits::OnSerializationFailed(); return false;
    }
    auto* backup = reinterpret_cast<int*>(header);
    auto* count = reinterpret_cast<int*>(header + 4);
    const int oldBackup = *backup, oldCount = *count;
    const int real = i::ClientState->nChokedCommands + 1;
    const int oldBit = buf->iCurrentBit;
    const bool oldOverflow = buf->bOverflow;
    if (slot != 0 || !g::pLocal || !g::pLocal->IsAlive() || !SyntheticWriter()
        || oldBackup < 0 || oldBackup > 7 || oldCount != real || real < 1
        || real + extra > tickshift::MaxNewCommands || oldBit != 0 || oldOverflow
        || !buf->pData || buf->nDataBytes <= 0 || buf->nDataBytes > 65536 || buf->nDataBits <= 0
        || buf->nDataBits > buf->nDataBytes * 8) {
        exploits::OnSerializationFailed(); return false;
    }
    CUserCmd* base = i::Input->GetUserCmd(next);
    if (!base || base->iCommandNumber != next) {
        exploits::OnSerializationFailed(); return false;
    }
    const int first = next - real + 1;
    for (int command = first; command <= next; ++command) {
        auto* cmd = i::Input->GetUserCmd(command);
        if (!cmd || cmd->iCommandNumber != command) {
            exploits::OnSerializationFailed(); return false;
        }
    }
    auto rollback = [&]() {
        *backup = oldBackup; *count = oldCount;
        buf->iCurrentBit = oldBit; buf->bOverflow = oldOverflow;
        exploits::OnSerializationFailed();
        return false; // CL_SendMove discards the entire payload.
    };
    int delta = -1;
    for (int command = first; command <= next; ++command) {
        if (!original(ecx, edx, slot, buf, delta, command, true) || buf->IsOverflowed()) return rollback();
        delta = command;
    }
    CUserCmd previous = *base, synthetic = *base;
    static auto* futureLimit = i::ConVar->FindVar("sv_max_usercmd_future_ticks");
    const int future = futureLimit ? (std::max)(0, futureLimit->GetInt()) : 8;
    const long long futureTick = (std::max)(static_cast<long long>(base->iTickCount),
        static_cast<long long>(exploits::packetPlan.serverTick) + future + exploits::packetPlan.uncertainty)
        + TIME_TO_TICKS(1.f);
    if (futureTick + extra >= (std::numeric_limits<int>::max)() || next > (std::numeric_limits<int>::max)() - extra)
        return rollback();
    synthetic.iCommandNumber = next + 1;
    synthetic.iTickCount = static_cast<int>(futureTick);
    // Rejected-count commands must never repeat weapon input if a different
    // server build executes them. They are intentionally absent from the ring.
    synthetic.iButtons &= ~(IN_ATTACK | IN_SECOND_ATTACK | IN_USE | IN_RELOAD);
    synthetic.iWeaponSelect = synthetic.iWeaponSubType = synthetic.uImpulse = 0;
    synthetic.flForwardMove = synthetic.flSideMove = synthetic.flUpMove = 0.f;
    for (int i = 0; i < extra; ++i) {
        WriteUsercmd(buf, &synthetic, &previous);
        if (buf->IsOverflowed()) return rollback();
        previous = synthetic; ++synthetic.iCommandNumber; ++synthetic.iTickCount;
    }
    *backup = 0; *count = real + extra;
    cycle.replaced = true;
    if (!exploits::OnSerialized(first, next, real, extra)) return rollback();
    return true;
}
