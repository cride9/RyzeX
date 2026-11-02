#include "antiaim.h"
#include "AdaptiveAntiAim.h"
#include "Animations/LocalAnimation.h"
#include "StudioHitboxGeometry.h"
#include "../../Interface/Classes/CCSGameRulesProxy.h"
#include "../../SDK/InputSystem.h"
#include <chrono>
#include <string_view>

namespace {
struct AdaptiveRuntime {
    adaptiveaa::Controller controller;
    adaptiveaa::Statistics stats;
    adaptiveaa::Input input;
    adaptiveaa::LowerBodyClock lowerBody;
    CBaseHandle handle{};
    const void* model{};
    float spawn{};
    int server{}, lastCommand{}, pending{}, nextProbe{};
    Vector expected{}, localProbe{}, threatProbe{};
    float probeYaw{};
    int threatIndex{}, probeMovement = -1, mode = -1;
    CBaseHandle threatHandle{};
    bool micro{}, lastConfirmed{}, probeAutoYaw{};
    float microValue{};
    struct EnemyShot { Vector source{}; float time = -10.f; int command{}; bool valid{}; } shots[65];
} adaptive;

int MovementType() {
    if (!g::pLocal) return STANDING;
    // Match AntiAim's enabled-state fallback, including standing settings
    // being used while a moving/in-air override is disabled.
    if (!(g::pLocal->GetFlags() & FL_ONGROUND) && cfg::antiaim::bEnabled[INAIR]) return INAIR;
    if (g::pLocal->GetVelocity().Length2D() > 5.f && cfg::antiaim::bEnabled[MOVING]) return MOVING;
    return STANDING;
}
bool Enabled(int type) {
    return type >= STANDING && type <= INAIR && cfg::antiaim::bEnabled[type]
        && (antiaim::IsAdaptiveMode(cfg::antiaim::iDesyncType[type]) || cfg::antiaim::iDesyncType[type] == EXTENDED);
}
bool SupportsDefensive(int type) {
    return Enabled(type) && (cfg::antiaim::iDesyncType[type] == ADAPTIVE || cfg::antiaim::iDesyncType[type] == EXTENDED_HYBRID);
}
adaptiveaa::Profile ProfileFor(int mode) {
    switch (mode) {
    case EXTENDED: case EXTENDED_HOLD: return adaptiveaa::Profile::ExtendedHold;
    case EXTENDED_PULSE: return adaptiveaa::Profile::ExtendedPulse;
    case EXTENDED_COVER: return adaptiveaa::Profile::ExtendedCover;
    case EXTENDED_HYBRID: return adaptiveaa::Profile::ExtendedHybrid;
    default: return adaptiveaa::Profile::Adaptive;
    }
}
bool Protected(CUserCmd* cmd) {
    bool unused = true;
    return !cmd || !g::pLocal || !g::pLocal->IsAlive() || !i::ClientState || i::ClientState->iDeltaTick <= 0
        || (GameRules && *GameRules && (*GameRules)->m_bFreezePeriod())
        || (cmd->iButtons & IN_USE) || (g::pLocal->GetFlags() & FL_FROZEN)
        || g::pLocal->GetMoveType() == MOVETYPE_LADDER || g::pLocal->GetMoveType() == MOVETYPE_NOCLIP
        || antiaim::ShouldDisableAntiaim(cmd, unused);
}
void Identity(CUserCmd* cmd, int type) {
    auto* local = g::pLocal;
    const bool lifecycle = adaptive.handle != local->GetRefEHandle() || adaptive.spawn != local->GetSpawnTime()
        || adaptive.model != local->GetModel() || adaptive.server != i::ClientState->nServerCount
        || cmd->iCommandNumber < adaptive.lastCommand;
    if (lifecycle || adaptive.mode != cfg::antiaim::iDesyncType[type]) {
        const auto clock=adaptive.lowerBody;
        antiaim::ResetAdaptive();
        // Mode/stance changes reset the pose choice, but must retain the
        // movement-established LBY deadline from the preceding commands.
        if (!lifecycle) adaptive.lowerBody=clock;
        adaptive.handle = local->GetRefEHandle(); adaptive.spawn = local->GetSpawnTime();
        adaptive.model = local->GetModel(); adaptive.server = i::ClientState->nServerCount;
        adaptive.mode = cfg::antiaim::iDesyncType[type];
        const auto entropy = static_cast<std::uint32_t>(std::chrono::steady_clock::now().time_since_epoch().count());
        adaptive.controller.Reset(entropy ^ static_cast<std::uint32_t>(adaptive.handle) ^ static_cast<std::uint32_t>(cmd->iCommandNumber));
    }
    adaptive.lastCommand = cmd->iCommandNumber;
}
adaptiveaa::LowerBodyWindow PrepareLowerBody(CUserCmd* cmd) {
    const auto* anim=g::pLocal->AnimState();
    const float speed=anim && std::isfinite(anim->flVelocityLenght2D)
        ? (std::max)(g::pLocal->GetVelocity().Length2D(),anim->flVelocityLenght2D)
        : std::numeric_limits<float>::quiet_NaN();
    const int tickbase=g::pLocal->GetTickBase();
    return adaptive.lowerBody.Prepare(cmd->iCommandNumber,TICKS_TO_TIME(tickbase),i::GlobalVars->flIntervalPerTick,
        speed,(g::pLocal->GetFlags() & FL_ONGROUND) != 0);
}
CBaseEntity* Threat() {
    CBaseEntity* best = nullptr; float bestScore = (std::numeric_limits<float>::max)();
    const Vector origin = g::pLocal->GetVecOrigin();
    for (int index=1; index <= (std::min)(64, i::GlobalVars->nMaxClients); ++index) {
        auto* player = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(index));
        if (!player || player == g::pLocal || !player->IsAlive() || player->IsDormant() || !player->IsEnemy(g::pLocal)) continue;
        const float distance = (player->GetVecOrigin()-origin).LengthSqr();
        const float fov = std::fabs(adaptiveaa::Yaw(M::CalcAngle(origin,player->GetVecOrigin()).y-g::vecOriginalViewAngle.y));
        const float score = distance * (1.f + fov / 180.f);
        if (score < bestScore) { bestScore=score; best=player; }
    }
    return best;
}
float Exposure(CBaseEntity* threat, Vector point) {
    const Vector enemy = threat->GetEyePosition(false);
    CTraceFilter filter([local = g::pLocal, threat](IHandleEntity* entity, int) { return entity != local && entity != threat; });
    Trace_t enter{}; ++adaptive.stats.traces;
    i::EngineTrace->TraceRay(Ray_t(enemy, point), MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &enter);
    if (enter.bStartSolid || enter.flFraction >= .995f) return 1.f;
    if (enter.pHitEntity && enter.pHitEntity->IsPlayer()) return .9f;
    Trace_t exit{}; ++adaptive.stats.traces;
    i::EngineTrace->TraceRay(Ray_t(point, enemy), MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &exit);
    if (exit.flFraction >= .995f) return 1.f;
    const auto* surface = i::PhysicsProps->GetSurfaceData(enter.surface.nSurfaceProps);
    if (!surface) return .8f;
    const float modifier = surface->game.flPenetrationModifier;
    if (!std::isfinite(modifier) || modifier < 0.f) return .8f;
    const float thickness = (enter.vecEnd-exit.vecEnd).Length();
    if (!std::isfinite(thickness)) return 1.f;
    // Visibility/thickness heuristic only: this deliberately avoids GetDamage.
    return std::clamp(1.f / (1.f + thickness / (12.f * (std::max)(.1f, modifier))), .03f, 1.f);
}
float PrepareCover(CUserCmd* cmd, int type) {
    auto* threat = Threat();
    const float base = adaptiveaa::Yaw(cmd->angViewPoint.y);
    const bool confirmed = false;
    const Vector eye = localAnim->GetShootPosition();
    const Vector enemyEye = threat ? threat->GetEyePosition(false) : Vector{};
    const int index = threat ? threat->EntIndex() : 0;
    const auto handle = threat ? threat->GetRefEHandle() : 0;
    if (!adaptiveaa::UsesCover(adaptive.input.profile)) {
        adaptive.stats.threat=index; adaptive.stats.coreExposure=1.f;
        adaptive.stats.validHead=adaptive.stats.commonCore=false;
        adaptive.controller.SetCover({{{-62,1},{-29,1},{0,1},{34,1},{67,1}}});
        return base;
    }
    if (cmd->iCommandNumber < adaptive.nextProbe && adaptive.probeMovement == type && adaptive.threatIndex == index && adaptive.threatHandle == handle
        && (eye-adaptive.localProbe).LengthSqr() < 16.f && (enemyEye-adaptive.threatProbe).LengthSqr() < 256.f
        && std::fabs(adaptiveaa::Yaw(base-adaptive.probeYaw)) < 8.f && adaptive.lastConfirmed == confirmed
        && adaptive.probeAutoYaw == adaptive.input.autoYaw) return base;
    adaptive.nextProbe = cmd->iCommandNumber + 6;
    adaptive.threatIndex=index; adaptive.threatHandle=handle; adaptive.localProbe=eye; adaptive.threatProbe=enemyEye;
    adaptive.probeYaw=base; adaptive.lastConfirmed=confirmed; adaptive.probeMovement=type;
    adaptive.probeAutoYaw=adaptive.input.autoYaw;
    adaptive.stats.threat=index;
    std::array<adaptiveaa::Candidate,5> candidates{{{-62,1}, {-29,1}, {0,1}, {34,1}, {67,1}}};
    hitbox::Shape head; Vector origin; float feetYaw{};
    adaptive.stats.validHead = localAnim->GetAdaptiveHead(head, origin, feetYaw);
    const auto currentOrigin = hitbox::FromVector(g::pLocal->GetAbsOrigin());
    adaptiveaa::Core core;
    if (adaptive.stats.validHead) {
        const auto shift = currentOrigin-hitbox::FromVector(origin);
        head.a = head.a + shift; head.b = head.b + shift;
        const auto delta = head.Center()-currentOrigin;
        if (delta.z < 25. || delta.z > 95. || delta.x*delta.x+delta.y*delta.y > 900.) adaptive.stats.validHead = false;
        else core = adaptiveaa::EstimateCore(head,currentOrigin,adaptive.input.negative,adaptive.input.positive);
    }
    adaptive.stats.commonCore = core.valid && core.common;
    if (!core.valid) { core.point=hitbox::FromVector(eye); feetYaw=base; }
    if (threat) {
        // With auto yaw off, cover only selects a fake/feet side. Probe the
        // two sides once instead of silently moving the configured real yaw.
        std::array<float,2> exposure{}; std::array<bool,2> probed{};
        for (auto& candidate : candidates) {
            const int side=candidate.offset < -15.f ? -1 : candidate.offset > 15.f ? 1 : adaptive.controller.Side();
            const int slot=side<0 ? 0 : 1;
            if (adaptive.input.autoYaw || !probed[slot]) {
                const int effectiveSide=side*adaptive.input.inverter;
                const float angle=adaptive.input.autoYaw ? candidate.offset
                    : effectiveSide*(effectiveSide<0 ? adaptive.input.negative : adaptive.input.positive);
                const auto point=adaptiveaa::Rotate(core.point,currentOrigin,adaptiveaa::Yaw(base+angle-feetYaw));
                exposure[slot]=Exposure(threat,hitbox::ToVector(point)); probed[slot]=true;
            }
            candidate.exposure=exposure[slot];
        }
    }
    adaptive.stats.coreExposure=1.f;
    for (const auto& candidate : candidates) adaptive.stats.coreExposure=(std::min)(adaptive.stats.coreExposure,candidate.exposure);
    adaptive.controller.SetCover(candidates);
    return base;
}
void UndoMicro(CUserCmd* cmd) {
    if (!adaptive.micro) return;
    const float angle = (cmd->angViewPoint.y-g::vecOriginalViewAngle.y) * .01745329252f;
    const float forward = adaptive.microValue * std::cos(angle), side = adaptive.microValue * std::sin(angle);
    if (std::fabs(cmd->flForwardMove-forward)<.05f && std::fabs(cmd->flSideMove-side)<.05f)
        cmd->flForwardMove=cmd->flSideMove=0.f;
}
}

const adaptiveaa::Statistics& antiaim::GetAdaptiveStatistics() { return adaptive.stats; }
void antiaim::ResetAdaptive() { adaptive = {}; }
void antiaim::ObserveLowerBody(CUserCmd* cmd) {
    if (!cmd || cmd->iCommandNumber<=0 || !g::pLocal || !g::pLocal->IsAlive()) return;
    if (!i::ClientState || i::ClientState->iDeltaTick<=0) { ResetAdaptive(); return; }
    Identity(cmd,MovementType());
    PrepareLowerBody(cmd);
}
void antiaim::ApplyAdaptivePreset(int type) {
    if (type<STANDING || type>INAIR) return;
    cfg::antiaim::bEnabled[type]=true; cfg::antiaim::iDesyncType[type]=ADAPTIVE;
    ResetAdaptive();
}
void antiaim::ApplyExtendedPreset(int type) {
    if (type<STANDING || type>INAIR) return;
    cfg::antiaim::bEnabled[type]=true;
    cfg::antiaim::iDesyncType[type]=type==STANDING ? EXTENDED_HOLD : EXTENDED_HYBRID;
    ResetAdaptive();
}
bool antiaim::LBYUpdate(CBaseEntity* local) {
    if (!local || local != g::pLocal || !g::pCmd || !local->IsAlive() || !i::ClientState) return false;
    if (!Enabled(MovementType())) return false;
    Identity(g::pCmd,MovementType());
    return PrepareLowerBody(g::pCmd).due;
}
namespace {
void DoManaged(CUserCmd* cmd, bool& sendPacket, AATYPE type, bool legacy, int legacyInverter) {
    antiaim::needMicromovement=false; antiaim::bHideFlick=false;
    adaptive.pending=0; adaptive.micro=false; adaptive.stats.traces=0;
    if (Protected(cmd)) return;
    if (type<STANDING || type>INAIR || !Enabled(type)
        || (legacy ? cfg::antiaim::iDesyncType[type]!=EXTENDED : !antiaim::IsAdaptiveMode(cfg::antiaim::iDesyncType[type]))) return;
    Identity(cmd,type);
    adaptive.input={}; adaptive.input.command=cmd->iCommandNumber; adaptive.input.movement=type;
    adaptive.input.profile=ProfileFor(adaptive.mode); adaptive.input.legacyExtended=legacy;
    const auto window=PrepareLowerBody(cmd);
    adaptive.input.stationary=window.stationary; adaptive.input.lowerBodyDue=window.due;
    adaptive.input.lowerBodyPre=window.preBreak; adaptive.input.lowerBodyCycle=window.cycle;
    const auto* anim=g::pLocal->AnimState();
    if (anim) {
        adaptive.input.negative=adaptiveaa::Bound(anim->flMinBodyYaw,anim->flMaxBodyYaw,anim->flRunningSpeed,
            anim->flWalkToRunTransition,anim->flDuckAmount,anim->flDuckingSpeed,-1);
        adaptive.input.positive=adaptiveaa::Bound(anim->flMinBodyYaw,anim->flMaxBodyYaw,anim->flRunningSpeed,
            anim->flWalkToRunTransition,anim->flDuckAmount,anim->flDuckingSpeed,1);
    } else adaptive.input.negative=adaptive.input.positive=0.f;
    adaptive.input.inverter=cfg::antiaim::bInverter && IPT::HandleInput(cfg::antiaim::iInverterBind) ? -1 : 1;
    if (cfg::antiaim::bInvertOnShoot[type] && antiaim::shotInvert) adaptive.input.inverter *= -1;
    if (legacy) adaptive.input.inverter=legacyInverter;
    adaptive.input.autoYaw=!legacy && cfg::antiaim::bAdaptiveYaw[type];
    adaptive.input.defensivePitch=!legacy && cfg::antiaim::bDefensivePitch[type];
    adaptive.input.baseYaw=legacy ? cmd->angViewPoint.y : PrepareCover(cmd,type);
    adaptive.input.pitch=cmd->angViewPoint.x;
    adaptive.input.confirmedDefensive=false;
    // A one-command choke is only a request. The final shot packet decision
    // remains authoritative and is reconciled in FinalizeAdaptive.
    const auto pose=adaptive.controller.Evaluate(adaptive.input,true);
    const float breakBound=pose.side<0 ? adaptive.input.negative : adaptive.input.positive;
    const bool lowerBodyRequest=adaptive.input.profile!=adaptiveaa::Profile::Adaptive && window.stationary && breakBound>35.f
        && (window.due || (adaptive.input.profile==adaptiveaa::Profile::ExtendedPulse && window.preBreak));
    if (((!legacy && i::ClientState->nChokedCommands==0) || lowerBodyRequest)
        && i::ClientState->nChokedCommands<14
        && !(cfg::antiaim::bFakeDuck && IPT::HandleInput(cfg::antiaim::iFakeDuckKey))) sendPacket=false;
    const auto out=adaptive.controller.Evaluate(adaptive.input,sendPacket);
    cmd->angViewPoint={out.pitch,out.yaw,0.f};
    adaptive.expected=cmd->angViewPoint; adaptive.pending=cmd->iCommandNumber;
    antiaim::bHideFlick=out.hideFlick;
    if (!legacy && adaptive.input.profile==adaptiveaa::Profile::Adaptive && type==STANDING
        && g::pLocal->GetVelocity().Length2D()<3.f && std::fabs(cmd->flForwardMove)<.001f
        && std::fabs(cmd->flSideMove)<.001f && !(cmd->iButtons & (IN_ATTACK|IN_SECOND_ATTACK|IN_DUCK))) {
        adaptive.micro=true; adaptive.microValue=cmd->iCommandNumber&1 ? 1.1f : -1.1f;
        cmd->flForwardMove=adaptive.microValue;
    }
}
}
void antiaim::DoAdaptive(CUserCmd* cmd, bool& sendPacket, AATYPE type) { DoManaged(cmd,sendPacket,type,false,1); }
void antiaim::DoExtended(CUserCmd* cmd, bool& sendPacket, AATYPE type, int inverter) { DoManaged(cmd,sendPacket,type,true,inverter); }
bool antiaim::FinalizeAdaptive(CUserCmd* cmd, bool sendPacket) {
    if (!cmd || adaptive.pending!=cmd->iCommandNumber) return false;
    adaptive.pending=0;
    bHideFlick=false;
    if (Protected(cmd) || !Enabled(adaptive.input.movement)
        || cfg::antiaim::iDesyncType[adaptive.input.movement]!=adaptive.mode) { UndoMicro(cmd); return false; }
    // Aimbot and Lua can own this command after AntiAim. Never overwrite
    // their resulting angles or any attack timestamp.
    if (std::fabs(adaptiveaa::Yaw(cmd->angViewPoint.y-adaptive.expected.y))>.01f
        || std::fabs(cmd->angViewPoint.x-adaptive.expected.x)>.01f) { UndoMicro(cmd); return false; }
    const auto out=adaptive.controller.Evaluate(adaptive.input,sendPacket);
    cmd->angViewPoint={out.pitch,out.yaw,0.f};
    bHideFlick=out.hideFlick;
    const int tickbase=g::pLocal->GetTickBase();
    adaptive.lowerBody.CommitBreak(cmd->iCommandNumber,TICKS_TO_TIME(tickbase),out.hiddenBreak);
    adaptive.controller.Commit(cmd->iCommandNumber,sendPacket);
    adaptive.stats.side=out.side; adaptive.stats.phase=adaptive.controller.Phase(); adaptive.stats.packets=adaptive.controller.Packets();
    adaptive.stats.offset=adaptive.input.autoYaw ? adaptive.controller.Offset() : 0.f;
    adaptive.stats.chosenExposure=adaptive.controller.Exposure();
    adaptive.stats.defensiveConfirmed=adaptive.input.confirmedDefensive;
    adaptive.stats.mode=adaptive.mode; adaptive.stats.lowerBodyCycle=adaptive.input.lowerBodyCycle;
    adaptive.stats.lowerBodyDue=adaptive.input.lowerBodyDue; adaptive.stats.hiddenBreak=out.hiddenBreak;
    return true;
}
void antiaim::HandleAdaptiveEvent(IGameEvent* event) {
    if (!event || !g::pLocal || !g::pLocal->IsAlive() || !cfg::antiaim::bAdaptiveReact || !Enabled(MovementType())) return;
    if (cfg::antiaim::iDesyncType[MovementType()]==EXTENDED || cfg::antiaim::iDesyncType[MovementType()]==EXTENDED_HOLD) return;
    if (!event->GetName()) return;
    const std::string_view name=event->GetName();
    const int user=i::EngineClient->GetPlayerForUserID(event->GetInt("userid"));
    const int local=g::pLocal->EntIndex();
    const int command=g::pCmd ? g::pCmd->iCommandNumber : adaptive.lastCommand;
    if (name=="player_hurt" && user==local) {
        const int attacker=i::EngineClient->GetPlayerForUserID(event->GetInt("attacker"));
        if (attacker<1 || attacker>=65) return;
        auto* player=static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(attacker));
        if (player && player->IsPlayer() && player->IsEnemy(g::pLocal) && adaptive.controller.React(command)) ++adaptive.stats.reactions;
        return;
    }
    if (user<1 || user>=65 || user==local) return;
    auto* player=static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(user));
    if (!player || !player->IsAlive() || !player->IsEnemy(g::pLocal)) return;
    auto& shot=adaptive.shots[user];
    if (name=="weapon_fire") { shot={player->GetEyePosition(false),i::GlobalVars->flRealTime,command,true}; return; }
    if (name!="bullet_impact" || !shot.valid || i::GlobalVars->flRealTime-shot.time<0.f || i::GlobalVars->flRealTime-shot.time>.35f) return;
    const hitbox::Vec impact{event->GetFloat("x"),event->GetFloat("y"),event->GetFloat("z")};
    const auto head=hitbox::FromVector(g::pLocal->GetEyePosition(false));
    const auto source=hitbox::FromVector(shot.source);
    // Use the real impact segment: never extend it through an intervening wall.
    if (impact.Valid() && (head-hitbox::ClosestPoint(head,source,impact)).LengthSq()<32.*32.) {
        if (adaptive.controller.React(command)) ++adaptive.stats.reactions;
        shot.valid=false;
    }
}
