#include "misc.h"
#include <fstream>
#include "../../SDK/Menu/config.h"
#include "../../SDK/math.h"
#include "../Rage/exploits.h"
#include "../Rage/antiaim.h"
#include "../Rage/ragebot.h"
#include "../../SDK/WavParser.h"
#include "../Rage/autowall.h"
#include "../Visuals/ESP.h"
#pragma comment(lib, "winmm.lib")
#define CheckIfNonValidNumber(x) (fpclassify(x) == FP_INFINITE || fpclassify(x) == FP_NAN || fpclassify(x) == FP_SUBNORMAL)

void misc::CreateMove(CUserCmd* pCmd, Vector& vecViewAngle,bool& bSendPacket) {

	BunnyHop(pCmd);
	FakeLag(bSendPacket);
	AutoStrafe(vecViewAngle, pCmd);
	AspectRatio();
	Slowwalk(pCmd, cfg::antiaim::fakewalk); // need menu element && keybind
	FastStop(pCmd);
	FakeDuck(pCmd);
	SlideFix();
	OnlyCheatLogs();
	RemovePostProcessing();
	FixScopeSens();
	WalkBot(pCmd);
	BlockBot(pCmd);
	ClanTag();
#if NDEBUG
	Security();
#endif
	//ViewModel();
}

void misc::EventHandler(IGameEvent* pEvent) {

	PreserveKillfeed(pEvent);
	BuyBot(pEvent);
	BulletImpact(pEvent, FRAME_UNDEFINED, false );
	BulletTracer(pEvent);
	HandlePlayerHitEffects( pEvent );
	WalkBotHandler(pEvent);
	WorldCrosshairHandler(pEvent);
}

CBaseEntity* UTIL_PlayerByIndex(int index)
{
	typedef CBaseEntity* (__fastcall* PlayerByIndex)(int);
	static PlayerByIndex UTIL_PlayerByIndex = reinterpret_cast<PlayerByIndex>(util::FindSignature("server.dll", "85 C9 7E 32 A1"));

	if (!UTIL_PlayerByIndex)
		return nullptr;

	return UTIL_PlayerByIndex(index);
}

void misc::ServerHitboxes() {

	static uintptr_t* pCall = (uintptr_t*)(util::FindSignature("server.dll", "55 8B EC 81 EC ? ? ? ? 53 56 8B 35 ? ? ? ? 8B D9 57 8B CE"));
	float fDuration = i::GlobalVars->flIntervalPerTick * 1.0f;

	PVOID pTEntity = nullptr;

	if ( g::pLocal == nullptr )
		return;

	for (int i = 0; i < i::GlobalVars->nMaxClients; i++) {

		CBaseEntity* pEntity = static_cast< CBaseEntity* >( i::EntityList->GetClientEntity( i ) );
		if ( !pEntity || !pEntity->IsAlive( ) || pEntity->IsDormant( ) || !pEntity->IsPlayer( ) || !pEntity->EntIndex( ) )
			continue;

		if (!cfg::misc::m_bDrawServerHitboxOnAllEntities && !cfg::misc::m_bDrawServerHitbox)
			continue;

		if (!cfg::misc::m_bDrawServerHitboxOnAllEntities && pEntity != g::pLocal)
			continue;

		if (!cfg::misc::m_bDrawServerHitbox && pEntity == g::pLocal)
			continue;

		pTEntity = UTIL_PlayerByIndex(cfg::misc::m_bDrawServerHitboxOnAllEntities ? pEntity->EntIndex() : g::pLocal->EntIndex() );
		
		if (pTEntity)
		{
			__asm
			{
				pushad
				movss xmm1, fDuration
				push 1
				mov ecx, pTEntity
				call pCall
				popad
			}
		}
	}
}

void misc::RemovePostProcessing() {

	static CConVar* mat_postprocess_enable = i::ConVar->FindVar("mat_postprocess_enable");

	mat_postprocess_enable->SetValue(cfg::misc::removals[4] ? 0 : 1);
}

void misc::Security() {

	// dont even ask that
	static auto showerror = i::ConVar->FindVar("cl_showerror");

	if (showerror->GetInt() != 0)
		showerror->SetValue(0);
}

void gotoStart(CUserCmd* cmd, std::vector<CUserCmd>& recordedCmds) {

	static int commandCount = 0;

	if (recordedCmds.empty()) 
		return;

	if (misc::bRetreat) {
		if (commandCount > 0) {
			cmd->flUpMove = -std::clamp(recordedCmds.at(commandCount).flUpMove * 500.f, -450.f, 450.f);
			cmd->flSideMove = -std::clamp(recordedCmds.at(commandCount).flSideMove * 500.f, -450.f, 450.f);
			cmd->flForwardMove = -std::clamp(recordedCmds.at(commandCount).flForwardMove * 500.f, -450.f, 450.f);
			cmd->angViewPoint = recordedCmds.at(commandCount).angViewPoint;
			i::EngineClient->SetViewAngles(cmd->angViewPoint);
			commandCount--;
		}
	}
	else
		commandCount = recordedCmds.size();
}

void misc::IdealTick(CUserCmd* pCmd) {

	if (!pCmd || !g::pLocal)
		return;

	if (!cfg::antiaim::idealTick)
		return;

	static bool bPositionSet;

	static Vector vecOrigin;

	static Vector vecOriginDelta;

	if (GetAsyncKeyState(cfg::antiaim::idealTickBind)) {

		if (!bPositionSet) {

			bPositionSet = true;
			vecOrigin = g::pLocal->GetVecOrigin();
			vecRecord = vecOrigin;
			g::pLocal->SetupBones(matrixRecord, 128, BONE_USED_BY_ANYTHING, i::GlobalVars->flCurrentTime);
		}

		if (pCmd->iButtons & IN_ATTACK)
			bRetreat = true;
	}
	else {

		bPositionSet = false;
		vecOrigin = Vector(0, 0, 0);
		vecRecord = vecOrigin;
	}

	if (bPositionSet && vecOrigin != Vector(0, 0, 0) && GetAsyncKeyState(cfg::antiaim::idealTickBind) && bRetreat) {

		if ((vecOrigin - g::pLocal->GetVecOrigin()).Length2D() > 3.29217472f) {

			Vector vecAngle;
			M::VectorAngles(vecOrigin - g::pLocal->GetEyePosition(), vecAngle);

			g::vecOriginalViewAngle.y = vecAngle.y;
			g::pCmd->flForwardMove += 450.f;
			g::pCmd->flSideMove = 0.f;
		}

		if ((vecOrigin - g::pLocal->GetVecOrigin()).Length2D() < 2.f) {
			bRetreat = false;
		}
		/*vecOriginDelta = vecOrigin - g::pLocal->GetAbsOrigin();

		auto flSideMove = ((cos(M_DEG2RAD(pCmd->angViewPoint.y)) * -vecOriginDelta.y) + (sin(M_DEG2RAD(pCmd->angViewPoint.y)) * vecOriginDelta.x));
		auto flForwardMove = ((sin(M_DEG2RAD(pCmd->angViewPoint.y)) * vecOriginDelta.y) + (cos(M_DEG2RAD(pCmd->angViewPoint.y)) * vecOriginDelta.x));

		pCmd->flSideMove = std::clamp(flSideMove * 500.f, -450.f, 450.f);
		pCmd->flForwardMove = std::clamp(flForwardMove * 500.f, -450.f, 450.f);

		if ((vecOrigin - g::pLocal->GetAbsOrigin()).LengthSqr() < 1.f) {
			bRetreat = false;
		}*/
	}
}

void misc::OnlyCheatLogs() {

	static bool bDidSet = false;
	static bool bDidReset = false;

	if (cfg::misc::onlyCheatLogs) {

		if (!bDidSet) {

			i::ConVar->FindVar("developer")->SetValue(false);

			i::ConVar->FindVar("con_filter_enable")->SetValue(true);

			i::ConVar->FindVar("con_filter_text_out")->SetValue("Achievements disabled");
			
			bDidSet = true;
		}
		bDidReset = false;
	}
	else {
		if (!bDidReset) {
			i::ConVar->FindVar("con_filter_text")->SetValue("");
			i::ConVar->FindVar("con_filter_enable")->SetValue(false);

			bDidReset = true;
		}
		bDidSet = false;
	}
}

void misc::NightMode() {

	// sky_lunacy

	if (!i::EngineClient->IsConnected() || !i::EngineClient->IsInGame() || i::ClientState->iDeltaTick < 0 || !cfg::misc::nightmode) {
		bResetNightMode = true;
		return;
	}

	static float backupR = cfg::misc::nightmodeColor[0], backupG = cfg::misc::nightmodeColor[1], backupB = cfg::misc::nightmodeColor[3];

	if (bResetNightMode || backupR != cfg::misc::nightmodeColor[0] || backupG != cfg::misc::nightmodeColor[1] || backupB != cfg::misc::nightmodeColor[3]) {

		static CConVar* r_DrawSpecificStaticProp = i::ConVar->FindVar("r_DrawSpecificStaticProp");
		r_DrawSpecificStaticProp->SetValue(0);

		for (MaterialHandle_t i = i::MaterialSystem->FirstMaterial(); i != i::MaterialSystem->InvalidMaterial(); i = i::MaterialSystem->NextMaterial(i)) {

			IMaterial* pMat = i::MaterialSystem->GetMaterial(i);

			if (!pMat)
				return;

			auto groupName = pMat->GetTextureGroupName();

			if (strstr(groupName, "World") || strstr(groupName, "StaticProp")) {

				static auto LoadNamedSky = reinterpret_cast<void(__fastcall*)(const char*)>(util::FindSignature("engine.dll", "55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45"));

				if (LoadNamedSky != nullptr)
					LoadNamedSky("sky_lunacy");

				if (strstr(groupName, "StaticProp"))
					pMat->ColorModulate(cfg::misc::nightmodeColor[0], cfg::misc::nightmodeColor[1], cfg::misc::nightmodeColor[2]);
				else
					pMat->ColorModulate((cfg::misc::nightmodeColor[0] * 255.f) / 561.f, (cfg::misc::nightmodeColor[1] * 255.f) / 561.f, (cfg::misc::nightmodeColor[2] * 255.f) / 561.f);

			}
		}

		bResetNightMode = false;
		backupR = cfg::misc::nightmodeColor[0], backupG = cfg::misc::nightmodeColor[1], backupB = cfg::misc::nightmodeColor[3];
	}
}

void misc::SlideFix() {

	if (!g::pLocal || !g::pLocal->IsAlive() || g::pLocal->GetMoveType() == MOVETYPE_LADDER || cfg::antiaim::bSlideWalk)
		return;

	g::pCmd->iButtons &= ~(IN_FORWARD | IN_BACK | IN_MOVERIGHT | IN_MOVELEFT);
}

void misc::BulletImpact(IGameEvent* pEvent, EStage curStage, bool bFrameStage ) {

	if (!g::pLocal || !g::pLocal->IsAlive())
		return;

	auto& ClientImpactList = *( CUtlVector<ClientHitVerify_t>* )( ( uintptr_t )g::pLocal + 0x11C50 );

	if (!cfg::misc::bulletImpact) {
		ClientImpactList.RemoveAll();
		return;
	}

	if (pEvent != nullptr && bFrameStage == false ) {

		if (!strcmp(pEvent->GetName(), "bullet_impact")) {

			auto iUser = i::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));

			if (iUser != g::pLocal->EntIndex())
				return;

			Vector vecImpact = Vector(pEvent->GetInt("x"), pEvent->GetInt("y"), pEvent->GetInt("z"));

			i::DebugOverlay->AddBoxOverlay(
				vecImpact,
				Vector(-2.0f, -2.0f, -2.0f),
				Vector(2.0f, 2.0f, 2.0f),
				Vector(0.0f, 0.0f, 0.0f),
				0.f,
				0.f,
				255.f,
				127.f,
				4.f
			);
		}
	}

	if ( curStage == FRAME_RENDER_START && bFrameStage == true ) {

		static int iLastCount = 0;
		for (int i = ClientImpactList.Count(); i > iLastCount; --i) {

			i::DebugOverlay->AddBoxOverlay(
				Vector(ClientImpactList[i - 1].pos),
				Vector(-2.0f, -2.0f, -2.0f),
				Vector(2.0f, 2.0f, 2.0f),
				Vector(0.0f, 0.0f, 0.0f),
				255.f,
				0.f,
				0.f,
				127.f,
				4.f
			);
		}

		if (ClientImpactList.Count() != iLastCount)
			iLastCount = ClientImpactList.Count();
	}
}

void misc::BuyBot(IGameEvent* event) { // need menu element

	if (!cfg::misc::autobuyEnabled)
		return;

	if (!strcmp(event->GetName(), "round_start")) {

		std::string buy = "";

		switch (cfg::misc::pistols) {

		case 1: buy += "buy glock;buy usp_silencer;buy hkp2000;"; break;
		case 2: buy += "buy elite;"; break;
		case 3: buy += "buy p250;"; break;
		case 4: buy += "buy cz75a;buy fiveseven; buy tec9;"; break;
		case 5: buy += "buy deagle;buy revolver;"; break;
		}
		switch (cfg::misc::snipers) {

		case 1: buy += "buy ssg08;"; break;
		case 2: buy += "buy awp;"; break;
		case 3: buy += "buy scar20;buy g3sg1;"; break;
		}

		if (cfg::misc::equipments[0])
			buy += "buy vesthelm;";

		if (cfg::misc::equipments[1])
			buy += "buy taser;";

		if (cfg::misc::equipments[2])
			buy += "buy defuser;";

		if (cfg::misc::grenades[0])
			buy += "buy molotov;buy incgrenade;";

		if (cfg::misc::grenades[1])
			buy += "buy decoy;";

		if (cfg::misc::grenades[2])
			buy += "buy flashbang;";

		if (cfg::misc::grenades[3])
			buy += "buy hegrenade;";

		if (cfg::misc::grenades[4])
			buy += "buy smokegrenade;";

		i::EngineClient->ExecuteClientCmd(buy.c_str());

		bResetNightMode = true;
	}
}

void misc::HandlePlayerHitEffects( IGameEvent* pEvent ) {

	if ( !strcmp( pEvent->GetName( ), "player_hurt" ) ) {

		IClientEntity* pAttacker = i::EntityList->GetClientEntity( i::EngineClient->GetPlayerForUserID( pEvent->GetInt( "attacker" ) ) );

		if ( !pAttacker || pAttacker != g::pLocal )
			return;

		CBaseEntity* pEntity = reinterpret_cast<CBaseEntity*>(i::EntityList->GetClientEntity( i::EngineClient->GetPlayerForUserID( pEvent->GetInt( "userid" ) ) ) );

		if ( !pEntity || pEntity == g::pLocal )
			return;

		// TODO: Add effects interface for cool hit effects :D
		/*if ( C::Get<bool>( Vars.bScreenHitEffects ) )
		{
			std::optional<Vector> vecPosition;
			vecPosition = pEntity->GetHitGroupPosition( pEvent->GetInt( "hitgroup" ) );

			if ( !vecPosition.has_value( ) )
				return;

			switch ( C::Get<int>( Vars.iScreenHitEffects ) )
			{
			case 0:
				i::Effects->Sparks( vecPosition.value( ), 10, 10 );
				break;
			case 1:
				i::Effects->Smoke( vecPosition.value( ), 1, 10.f, 60.f );
				break;
			case 2:
				i::Effects->EnergySplash( vecPosition.value( ), vecPosition.value( ) + 50, true );
				break;
			}
		}*/

		// play hit sound
		if ( cfg::misc::m_iHitSound == 1 ) {
			i::EngineSoundClient->EmitAmbientSound( "buttons\\arena_switch_press_02.wav", cfg::misc::m_flHitSoundVolume / 100.f );
			// physics\\metal\\paintcan_impact_hard3.wav
		}
		else if ( cfg::misc::m_iHitSound == 2 && !cfg::misc::m_szWavPath.empty( ) ) {

			static bool m_bNeedsUpdate = true;
			static float m_flOldVolume = 0.f;
			static std::string m_szOldWavPath;
			// store the parsed hitsound bytes to a dummy byte.
			static BYTE* m_pParsedHitsound;

			// read the .wav file into memory.
			BYTE* m_pSoundBytes = util::ReadWavFileIntoMemory( cfg::misc::m_szWavPath );

			if ( cfg::misc::m_flHitSoundVolume != m_flOldVolume || m_szOldWavPath != cfg::misc::m_szWavPath )
				m_bNeedsUpdate = true;

			if ( m_bNeedsUpdate )
			{
				m_szOldWavPath = cfg::misc::m_szWavPath;
				m_flOldVolume = cfg::misc::m_flHitSoundVolume;
				m_bNeedsUpdate = false;

				// adjust the hitsound volume.
				m_pParsedHitsound = m_pSoundBytes;

				CWavParser::WavHeader_t header;
				header.ParseWavHeader( m_pParsedHitsound );
				wavparser.AdjustWavVolume( header, ( cfg::misc::m_flHitSoundVolume / 200.f ) );
			}
			// play the sound.
			if ( m_pParsedHitsound ) {

				PlaySoundA( LPCSTR( m_pParsedHitsound ), NULL, SND_MEMORY | SND_ASYNC );
			}
		}
	}
}

void misc::PreserveKillfeed(IGameEvent* event) { // need menu element

	static auto FindHudElement = [](const char* name) {

		static auto pThis = *reinterpret_cast<DWORD**>(util::FindSignature("client.dll", "B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08") + 1);

		static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(util::FindSignature("client.dll", "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));

		return find_hud_element(pThis, name);
	};

	if (!strcmp(event->GetName(), "player_death")) {

		auto pAttacker = i::EntityList->GetClientEntity(i::EngineClient->GetPlayerForUserID(event->GetInt("attacker")));

		if (!pAttacker || pAttacker != g::pLocal)
			return;

		int index = i::EngineClient->GetPlayerForUserID(event->GetInt("userid"));

		if (!index)
			return;

		static DWORD* _death_notice;

		if (i::EngineClient->IsConnected() && i::EngineClient->IsInGame())
			_death_notice = (DWORD*)FindHudElement("CCSGO_HudDeathNotice");

		static void(__thiscall * _clear_notices)(DWORD) = (void(__thiscall*)(DWORD))util::FindSignature("client.dll", "55 8B EC 83 EC 0C 53 56 8B 71 58");

		if (!_clear_notices)
			return;

		if (_death_notice)
			*(float*)((DWORD)_death_notice + 0x50) = cfg::misc::preserveKillfeed ? FLT_MAX : 1.5; // need menu element
	}
	else if (!strcmp(event->GetName(), "round_start")) {

		g_LocalAnimations->ResetData();

		static DWORD* _death_notice = (DWORD*)FindHudElement("CCSGO_HudDeathNotice");
		static void(__thiscall * _clear_notices)(DWORD) = (void(__thiscall*)(DWORD))util::FindSignature("client.dll", "55 8B EC 83 EC 0C 53 56 8B 71 58");

		if (_death_notice)
			_death_notice = (DWORD*)FindHudElement("CCSGO_HudDeathNotice");
		if (_clear_notices)
			_clear_notices(((DWORD)_death_notice - 20));
	}
}

void misc::FakeDuck(CUserCmd* pCmd) {

	if (GetAsyncKeyState(cfg::antiaim::fakeduckbind) && cfg::antiaim::fakeduck) {

		auto iChoke = i::ClientState->nChokedCommands;

		pCmd->iButtons |= IN_BULLRUSH;

		if (g::pLocal->GetFlags() & FL_ONGROUND) {

			if (iChoke <= 7)
				pCmd->iButtons &= ~IN_DUCK;
			else
				pCmd->iButtons |= IN_DUCK;
		}
	}
}

void misc::FastStop(CUserCmd* pCmd) {

	if (!cfg::misc::faststop)
		return;

	if (!g::pLocal || !g::pLocal->IsAlive() || !pCmd || !pCmd->iCommandNumber)
		return;

	if (!(g::pLocal->GetFlags() & FL_ONGROUND))
		return;

	auto bPressedKey = pCmd->iButtons & IN_FORWARD || pCmd->iButtons & IN_MOVELEFT || pCmd->iButtons & IN_BACK || pCmd->iButtons & IN_MOVERIGHT || pCmd->iButtons & IN_JUMP;

	if (bPressedKey)
		return;

	auto vecVelocity = g::pLocal->GetVelocity();

	if (vecVelocity.Length2D() > 20.0f)
	{
		Vector vecDirection;
		Vector vecView;

		M::VectorAngles(vecVelocity, vecDirection);
		i::EngineClient->GetViewAngles(vecView);

		vecDirection.y = vecView.y - vecDirection.y;

		Vector vecForward;
		M::AngleVectors(vecDirection, &vecForward);

		static auto cl_forwardspeed = i::ConVar->FindVar("cl_forwardspeed");
		static auto cl_sidespeed = i::ConVar->FindVar("cl_sidespeed");

		auto negative_forward_speed = -cl_forwardspeed->GetFloat();
		auto negative_side_speed = -cl_sidespeed->GetFloat();

		auto negative_forward_direction = vecForward * negative_forward_speed;
		auto negative_side_direction = vecForward * negative_side_speed;

		pCmd->flForwardMove = negative_forward_direction.x;
		pCmd->flSideMove = negative_side_direction.y;
	}
}

void misc::Slowwalk(CUserCmd* pCmd, float flSpeed) {

	if (!g::pLocal || !g::pLocal->IsAlive() || !pCmd || !pCmd->iCommandNumber)
		return;

	if (flSpeed <= 0.f)
		return;

	if (!GetAsyncKeyState(cfg::antiaim::fakewalkKey))
		return;

	float flMinSpeed = (float)(sqrt((pCmd->flForwardMove * pCmd->flForwardMove) + (pCmd->flSideMove * pCmd->flSideMove) + (pCmd->flUpMove * pCmd->flUpMove)));
	if (flMinSpeed <= 0.f)
		return;

	if (pCmd->iButtons & IN_DUCK)
		flSpeed *= 2.94117647f;

	if (flMinSpeed <= flSpeed)
		return;

	float flMove = flSpeed / flMinSpeed;

	pCmd->flForwardMove *= flMove;
	pCmd->flSideMove *= flMove;
	pCmd->flUpMove *= flMove;
}

void misc::AspectRatio() {

	static CConVar* r_aspectratio = i::ConVar->FindVar("r_aspectratio");

	if (!cfg::misc::aspectRatio) {
		r_aspectratio->SetValue(0);
		return;
	}

	float ratio = (cfg::misc::aspectRatioValue * 0.1) / 2;
	if (ratio > 0.001)
		r_aspectratio->SetValue(ratio); //ayyware hhhh
	else
		r_aspectratio->SetValue((35 * 0.1f) / 2);
}

void misc::AutoStrafe(Vector& vecView, CUserCmd* pCmd) {

	if (!cfg::misc::autoStrafe)
		return;

	// check if local player is valid
	if (!g::pLocal || !g::pLocal->IsAlive() || !pCmd || !pCmd->iCommandNumber)
		return;

	// check ladder and ground
	if (g::pLocal->GetMoveType() == MOVETYPE_LADDER || g::pLocal->GetFlags() & FL_ONGROUND)
		return;

	// check if we really want to strafe not just jumpscout
	if (g::pLocal->GetVelocity().Length2D() < 20)
		return;

	static auto cl_sidespeed = i::ConVar->FindVar("cl_sidespeed");
	auto side_speed = cl_sidespeed->GetFloat();

	static auto old_yaw = 0.0f;

	auto m_pcmd = pCmd;

	auto get_velocity_degree = [](float velocity)
	{
		auto tmp = M_RAD2DEG(atan(30.0f / velocity));

		if (CheckIfNonValidNumber(tmp) || tmp > 90.0f)
			return 90.0f;

		else if (tmp < 0.0f)
			return 0.0f;
		else
			return tmp;
	};

	auto velocity = g::pLocal->GetVelocity();
	//velocity.z = 0.0f;

	auto forwardmove = m_pcmd->flForwardMove;
	auto sidemove = m_pcmd->flSideMove;

	if (velocity.Length2D() < 5.0f && !forwardmove && !sidemove)
		return;

	static auto flip = false;
	flip = !flip;

	auto turn_direction_modifier = flip ? 1.0f : -1.0f;
	auto viewangles = vecView;

	if (forwardmove || sidemove)
	{
		m_pcmd->flForwardMove = 0.0f;
		m_pcmd->flSideMove = 0.0f;

		auto turn_angle = atan2(-sidemove, forwardmove);
		viewangles.y += turn_angle * M_RADPI;
	}
	else if (forwardmove) //-V550
		m_pcmd->flForwardMove = 0.0f;

	auto strafe_angle = M_RAD2DEG(atan(15.0f / velocity.Length2D()));

	if (strafe_angle > 90.0f)
		strafe_angle = 90.0f;
	else if (strafe_angle < 0.0f)
		strafe_angle = 0.0f;

	auto temp = Vector(0.0f, viewangles.y - old_yaw, 0.0f);
	temp.y = M::NormalizeYaw(temp.y);

	auto yaw_delta = temp.y;
	old_yaw = viewangles.y;

	auto abs_yaw_delta = fabs(yaw_delta);

	if (abs_yaw_delta <= strafe_angle || abs_yaw_delta >= 30.0f)
	{
		Vector velocity_angles;
		M::VectorAngles(velocity, velocity_angles);

		temp = Vector(0.0f, viewangles.y - velocity_angles.y, 0.0f);
		temp.y = M::NormalizeYaw(temp.y);

		auto velocityangle_yawdelta = temp.y;
		auto velocity_degree = get_velocity_degree(velocity.Length2D());

		if (velocityangle_yawdelta <= velocity_degree || velocity.Length2D() <= 15.0f)
		{
			if (-velocity_degree <= velocityangle_yawdelta || velocity.Length2D() <= 15.0f)
			{
				viewangles.y += strafe_angle * turn_direction_modifier;
				m_pcmd->flSideMove = side_speed * turn_direction_modifier;
			}
			else
			{
				viewangles.y = velocity_angles.y - velocity_degree;
				m_pcmd->flSideMove = side_speed;
			}
		}
		else
		{
			viewangles.y = velocity_angles.y + velocity_degree;
			m_pcmd->flSideMove = -side_speed;
		}
	}
	else if (yaw_delta > 0.0f)
		m_pcmd->flSideMove = -side_speed;
	else if (yaw_delta < 0.0f)
		m_pcmd->flSideMove = side_speed;

	auto move = Vector(m_pcmd->flForwardMove, m_pcmd->flSideMove, 0.0f);
	auto speed = move.Length();

	Vector angles_move;
	M::VectorAngles(move, angles_move);

	auto normalized_x = fmod(vecView.x + 180.0f, 360.0f) - 180.0f;
	auto normalized_y = fmod(vecView.y + 180.0f, 360.0f) - 180.0f;

	auto yaw = M_DEG2RAD(normalized_y - viewangles.y + angles_move.y);

	if (normalized_x >= 90.0f || normalized_x <= -90.0f || m_pcmd->angViewPoint.x >= 90.0f && m_pcmd->angViewPoint.x <= 200.0f || m_pcmd->angViewPoint.x <= -90.0f && m_pcmd->angViewPoint.x <= 200.0f)
		m_pcmd->flForwardMove = -cos(yaw) * speed;
	else
		m_pcmd->flForwardMove = cos(yaw) * speed;

	m_pcmd->flSideMove = sin(yaw) * speed;
}

void misc::MovementFix(CUserCmd* pCmd, Vector& oldang) {

	Vector vMovements(pCmd->flForwardMove, pCmd->flSideMove, 0.f);

	if (vMovements.Length2D() == 0)
		return;

	Vector vRealF, vRealR;
	Vector aRealDir = pCmd->angViewPoint;
	aRealDir.Normalize();
	aRealDir.Clamp();

	M::AngleVectors(aRealDir, &vRealF, &vRealR, nullptr);
	vRealF[2] = 0;
	vRealR[2] = 0;

	vRealF.VectorNormalize();
	vRealR.VectorNormalize();

	Vector aWishDir = oldang;
	aWishDir.Normalize();
	aWishDir.Clamp();

	Vector vWishF, vWishR;
	M::AngleVectors(aWishDir, &vWishF, &vWishR, nullptr);

	vWishF[2] = 0;
	vWishR[2] = 0;

	vWishF.VectorNormalize();
	vWishR.VectorNormalize();

	Vector vWishVel;
	vWishVel[0] = vWishF[0] * pCmd->flForwardMove + vWishR[0] * pCmd->flSideMove;
	vWishVel[1] = vWishF[1] * pCmd->flForwardMove + vWishR[1] * pCmd->flSideMove;
	vWishVel[2] = 0;

	float a = vRealF[0], b = vRealR[0], c = vRealF[1], d = vRealR[1];
	float v = vWishVel[0], w = vWishVel[1];

	float flDivide = (a * d - b * c);
	float x = (d * v - b * w) / flDivide;
	float y = (a * w - c * v) / flDivide;

	pCmd->flForwardMove = x;
	pCmd->flSideMove = y;

}

void misc::BunnyHop(CUserCmd* pCmd) {
	
	if (!g::pLocal || !g::pLocal->IsAlive())
		return;

	if (!cfg::misc::bunnyhop)
		return;

	static bool bLastJumped = false, bShouldFake = false;

	if (!bLastJumped && bShouldFake) {

		bShouldFake = false;
		pCmd->iButtons |= IN_JUMP;
	}
	else if (pCmd->iButtons & IN_JUMP) {

		if (g::pLocal->GetFlags() & FL_ONGROUND || g::pLocal->GetFlags() & FL_PARTIALGROUND) {

			bLastJumped = true;
			bShouldFake = true;
		}
		else {

			pCmd->iButtons &= ~IN_JUMP;
			bLastJumped = false;
		}
	}
	else {

		bLastJumped = false;
		bShouldFake = false;
	}
}

void misc::ThirdPerson() {

	if (!cfg::misc::thirdperson || !g::pLocal)
		return;

	static bool didSetThirdPerson = false;
	static CConVar* svcheats = i::ConVar->FindVar("sv_cheats");

	if (GetKeyState(cfg::misc::thirdpersonbind) && g::pLocal->IsAlive()) {

		if (!didSetThirdPerson) {

			*(int*)((DWORD)&svcheats->fnChangeCallbacks + 0xC) = 0; // ew
			svcheats->SetValue(1);

			static std::string tpfix = "cam_idealpitch 0;";
			tpfix += "cam_idealyaw 0;";
			tpfix += "cam_idealdist 140;";
			tpfix += "thirdperson;";

			i::EngineClient->ExecuteClientCmd(tpfix.c_str());
			didSetThirdPerson = true;
		}
	}
	else {
		i::EngineClient->ExecuteClientCmd("firstperson");
		didSetThirdPerson = false;
	}
}

void misc::FakeLag(bool& bSendPacket) {

	enum FAKELAGTYPE {

		MAXIMUM,
		ADAPTIVE,
		JITTER
	};

	if (!g::pLocal || !g::pLocal->IsAlive() || !cfg::antiaim::fakelag || !cfg::antiaim::enableFakelag) {
		bSendPacket = true;
		return;
	}

	if (GetAsyncKeyState(cfg::antiaim::fakeduckbind) && cfg::antiaim::fakeduck) {
		bSendPacket = i::ClientState->nChokedCommands >= 14;
		return;
	}

	if (exploits::bDoubleTapEnabled && exploits::iShiftAmount > 0 && cfg::rage::doubletap && GetKeyState(cfg::rage::doubletapkey))
		return;

	static int iCurrentChoke = 0;

	static CConVar* sv_maxspeed = i::ConVar->FindVar("sv_maxspeed");
	float flVelocity = g::pLocal->GetVelocity().Length2D();
	float flMaxVelocity = g::pLocal->GetWeapon() ? g::pLocal->IsScoped() ? g::pLocal->GetWeapon()->GetCSWpnData()->flMaxSpeed[1] : g::pLocal->GetWeapon()->GetCSWpnData()->flMaxSpeed[0] : sv_maxspeed->GetFloat();
	static bool bChokeCycleEnded = false;

	int iMin = cfg::antiaim::fakelagmin;
	int iMax = cfg::antiaim::fakelagmax;

	if (cfg::antiaim::fakelag) {

		if (!g::bWaiting && cfg::rage::doubletap && GetKeyState(cfg::rage::doubletapkey) && !exploits::bIsShiftingTicks) {
			iCurrentChoke = min(1, iCurrentChoke);
		}
		else {

			switch (cfg::antiaim::fakelagType) {

			case MAXIMUM:
				iCurrentChoke = cfg::antiaim::fakelag;
				break;

			case ADAPTIVE:

				iCurrentChoke = max(1, 15 * (flVelocity / flMaxVelocity));
				break;

			case JITTER:

				if (bChokeCycleEnded) {

					iCurrentChoke = iMin;
					bChokeCycleEnded = !(i::ClientState->nChokedCommands >= iCurrentChoke);
				}
				else {

					iCurrentChoke = cfg::antiaim::fakelag;
					bChokeCycleEnded = i::ClientState->nChokedCommands >= iCurrentChoke;
				}
				break;
			}
		}
	}
	else
		iCurrentChoke = cfg::antiaim::fakelag;


	bSendPacket = i::ClientState->nChokedCommands >= min(iMax, max(cfg::rage::doubletap && GetKeyState(cfg::rage::doubletapkey) ? 1 : iMin, iCurrentChoke));
}

void misc::DrawBream(Vector vecSource, Vector vecEnd, Color color) {

	BeamInfo_t info;
	info.m_nType = TE_BEAMPOINTS;
	//info.m_pszModelName = "sprites/purplelaser1.vmt";
	info.m_pszModelName = "sprites/white.vmt";
	info.m_nModelIndex = -1;
	info.m_flHaloScale = -1.0f;
	info.m_flLife = 3.0f;
	info.m_flWidth = 1.0f;
	info.m_flEndWidth = 1.0f;
	info.m_flFadeLength = 0.0f;
	info.m_flAmplitude = 2.0f;
	info.m_flBrightness = color[3];
	info.m_flSpeed = 0.5f;
	info.m_nStartFrame = 0.f;
	info.m_flFrameRate = 0.f;
	info.m_flRed = color[0];
	info.m_flGreen = color[1];
	info.m_flBlue = color[2];
	info.m_nSegments = 2;
	info.m_bRenderable = true;
	info.m_nFlags = FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM;
	info.m_vecStart = vecSource;
	info.m_vecEnd = vecEnd;

	Beam_t* pBeam = i::RenderBeam->CreateBeamPoints(info);
	if (pBeam)
		i::RenderBeam->DrawBeam(pBeam);
}

void misc::BulletTracer(IGameEvent* pEvent) {

	if (!g::pLocal || !g::pLocal->IsAlive() || !cfg::misc::bulletTracer)
		return;

	/* Get this once, so the beams won't deform bcs of multiple impact -> multiple position */
	if (!strcmp(pEvent->GetName(), "bullet_impact")) {

		auto iUser = i::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));

		if (iUser != i::EngineClient->GetLocalPlayer())
			return;

		Vector vecImpact = Vector(pEvent->GetInt("x"), pEvent->GetInt("y"), pEvent->GetInt("z"));

		DrawBream(vecEyePosition, vecImpact, cfg::misc::bulletTracerColor);
	}
}

void misc::WorldCrosshairHandler(IGameEvent* pEvent) {

	if (!g::pLocal || !g::pLocal->IsAlive() || !cfg::misc::bWorldCrosshair)
		return;

	if (!strcmp(pEvent->GetName(), "bullet_impact")) {

		auto iUser = i::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));

		if (iUser != g::pLocal->EntIndex())
			return;

		static int i = 0;

		cfg::misc::flWorldCrosshairColor[3] = 1.f;
		visual::flWorldCrosshairLength[i] = i::GlobalVars->flCurrentTime;
		visual::vecWorldCrosshair[i] = Vector(pEvent->GetInt("x"), pEvent->GetInt("y"), pEvent->GetInt("z"));
		i++;
		if (i >= 5)
			i = 0;
	}
}

void misc::FixScopeSens() {

	// zoom_sensitivity_ratio_mouse 
	static CConVar* zoom_sensitivity_ratio_mouse = i::ConVar->FindVar("zoom_sensitivity_ratio_mouse");

	static float backup = zoom_sensitivity_ratio_mouse->GetFloat();

	if (!g::pLocal || !g::pLocal->GetWeapon()) {
		zoom_sensitivity_ratio_mouse->SetValue(backup);
	}

	if (!g::pLocal->IsScoped() || i::EngineClient->IsInGame()) {
		zoom_sensitivity_ratio_mouse->SetValue(backup);
		return;
	}

	if (zoom_sensitivity_ratio_mouse->GetFloat() != 0 && backup != zoom_sensitivity_ratio_mouse->GetFloat())
		backup = zoom_sensitivity_ratio_mouse->GetFloat();

	zoom_sensitivity_ratio_mouse->SetValue(g::pLocal->IsScoped() ? cfg::misc::removals[3] ? 0 : backup : backup);
}

void misc::AutoPistol(CUserCmd* pCmd, CBaseEntity* pLocal) {

	if (!pLocal || !pLocal->IsAlive() || !pLocal->GetWeapon())
		return;

	if (pLocal->IsGrenade(pLocal->GetWeapon()))
		return;

	if (pLocal->GetWeapon()->GetItemDefinitionIndex() == EItemDefinitionIndex::WEAPON_C4 ||
		pLocal->GetWeapon()->GetItemDefinitionIndex() == EItemDefinitionIndex::WEAPON_REVOLVER)
		return;

		if ((pCmd->iButtons & IN_ATTACK) && (pLocal->GetWeapon()->GetNextPrimaryAttack() >= TICKS_TO_TIME(pLocal->GetTickBase())))
		pCmd->iButtons &= ~IN_ATTACK;
}

void misc::RemoveSmoke() {

	static int flCurrentTime = i::GlobalVars->flCurrentTime;

	if (!cfg::misc::removals[0] || !g::pLocal || flCurrentTime > (i::GlobalVars->flCurrentTime - TICKS_TO_TIME(64)))
		return;

	flCurrentTime = i::GlobalVars->flCurrentTime;
	static auto sigLineGoesThroughSmoke = util::FindSignature("client.dll", "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0");

	static std::vector<const char*> vecSmokeWireframe =
	{
		"particle/vistasmokev1/vistasmokev1_smokegrenade",
	};

	static std::vector<const char*> vecSmokeNoDraw =
	{
		"particle/vistasmokev1/vistasmokev1_fire",
		"particle/vistasmokev1/vistasmokev1_emods",
		"particle/vistasmokev1/vistasmokev1_emods_impactdust",
	};

	for (auto szCurrentMat : vecSmokeWireframe) {

		IMaterial* pMaterial = i::MaterialSystem->FindMaterial(szCurrentMat, "Other textures");
		pMaterial->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true); //wireframe
	}

	for (auto szCurrentMat : vecSmokeNoDraw) {

		IMaterial* pMaterial = i::MaterialSystem->FindMaterial(szCurrentMat, "Other textures");
		pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
	}

	static auto iSmokeCount = *reinterpret_cast<DWORD*>(sigLineGoesThroughSmoke + 0x8);
	*reinterpret_cast<int*>(iSmokeCount) = 0;
}

void misc::WalkBotHandler(IGameEvent* pEvent) {

	if (!strcmp(pEvent->GetName(), "round_start"))
		bNewRound = true;
}

void TraceRayBot(CUserCmd* pCmd);
void misc::WalkBot(CUserCmd* pCmd) {

	static int positionID = 0;
	static int randomLmao = 0;
	static std::vector<Vector> moveHere;
	static Vector vecLastAngle = g::vecOriginalViewAngle;

	if (!g::pLocal)
		return;

	if (!cfg::debugSwitch2 || !g::pLocal->IsAlive() || bNewRound) {
		AIVizualization.clear();
		moveHere.clear();
		positionID = 0;
		randomLmao = M::RandomInt(0, 6);
		bNewRound = false;

		int iLocalTeam = g::pLocal->GetTeam();
		switch (randomLmao)
		{
		case 1: moveHere = iLocalTeam == TEAM_CT ? walkbotPositions::MirageCTB : walkbotPositions::MirageTBRush;
			break;
		case 2: moveHere = iLocalTeam == TEAM_CT ? walkbotPositions::MirageCTNinja : walkbotPositions::MirageTMidRush;
			break;
		case 3: moveHere = iLocalTeam == TEAM_CT ? walkbotPositions::MirageCTNinja : walkbotPositions::MirageTPallaceRush;
			break;
		case 4: moveHere = iLocalTeam == TEAM_CT ? walkbotPositions::MirageCTStairs : walkbotPositions::MirageTStairs;
			break;
		case 5: moveHere = iLocalTeam == TEAM_CT ? walkbotPositions::MirageCTSpawnBox : walkbotPositions::MirageTUnderGroundRush;
			break;

		default: moveHere = iLocalTeam == TEAM_CT ? walkbotPositions::MirageCTSpawnBox : walkbotPositions::MirageTUnderGroundRush;
			break;
		}

		return;
	}

	if (moveHere.empty())
		return;

	//int iClosestIndex = antiaim::ClosestToLocal();
	//CBaseEntity* pClosestTarget = nullptr;

	//if (iClosestIndex != -1)
	//	pClosestTarget = reinterpret_cast<CBaseEntity*>(i::EntityList->GetClientEntity(iClosestIndex));

	//if (pClosestTarget != nullptr) {

	//	Vector vecInterpolatedEyePosition = ragebot.InterpolateLocalEyePosition(g::pLocal->GetEyePosition(), 3);

	//	FireBulletData_t data = { };
	//	data.vecPosition = vecInterpolatedEyePosition;
	//	data.vecDirection = (pClosestTarget->GetHitboxPosition(HITBOX_UPPER_CHEST).value() - vecInterpolatedEyePosition).Normalized();

	//	Vector vecCalcAngle;
	//	M::VectorAngles(pClosestTarget->GetHitboxPosition(HITBOX_STOMACH).value() - g::pLocal->GetEyePosition(), vecCalcAngle);
	//	Vector vecDistanceBetween = (vecLastAngle.NormalizeAngle() - vecCalcAngle.NormalizeAngle());

	//	float flFinalFov = vecDistanceBetween.y;

	//	vecLastAngle.y += std::clamp(flFinalFov, -30.f, 30.f);

	//	if (flFinalFov < 35)
	//		vecLastAngle.y = vecCalcAngle.y;

	//	M::NormalizeAngle(vecLastAngle.y);
	//	i::EngineClient->SetViewAngles(vecLastAngle);

	//	if (autowall.SimulateFireBullet(g::pLocal, g::pLocal->GetWeapon(), data))
	//		return;
	//}

	if (moveHere.size() > positionID) {

		MoveToPosition(moveHere.at(positionID));
		AIVizualization.push_front(moveHere.at(positionID));

		if ((moveHere.at(positionID) - g::pLocal->GetAbsOrigin()).Length2D() < 10.f) {
			positionID++;
		}

		while (AIVizualization.size() >= 32)
			AIVizualization.pop_back();
	}
	else {
		TraceRayBot(pCmd);
	}
}

void TraceRayBot(CUserCmd* pCmd) // wip
{
	using namespace misc;

	auto GRD_TO_BOG = [&](float GRD) -> float {
		return (M_PI / 180) * GRD;
	};
	auto ProjectPoint = [&](Vector vecOrigin, float flYaw, float flDistance) -> Vector {
		return (vecOrigin + Vector((flDistance * sin(GRD_TO_BOG(flYaw))), -(flDistance * cos(GRD_TO_BOG(flYaw))), 0));
	};

	float flBestDistance = 0.f;
	float flBestAngle = -9999999.f;

	pCmd->flForwardMove = 450.f;

	CTraceFilter filter1(g::pLocal);
	Vector vecOrigin = g::pLocal->GetVecOrigin() + Vector(0, 0, 10);

	Vector vecEyeAngle;
	i::EngineClient->GetViewAngles(vecEyeAngle);

	Trace_t TraceFront;
	Vector vecProjectFirstPoint = ProjectPoint(vecOrigin, M::NormalizeYaw((vecEyeAngle.y + 90)), 40);
	i::EngineTrace->TraceRay(Ray_t(vecOrigin, vecProjectFirstPoint), MASK_SOLID, &filter1, &TraceFront);
	AIVizualization.push_front(TraceFront.vecEnd);

	static bool bJumped = false;

	if (!(g::pLocal->GetFlags() & FL_ONGROUND))
		pCmd->iButtons |= IN_DUCK;
	else
		bJumped = false;

	if (TraceFront.flFraction != 1.0f && (g::pLocal->GetFlags() & FL_ONGROUND) && !bJumped) {
		pCmd->iButtons |= IN_JUMP;
		bJumped = true;
	}

	static bool bRandomDirect = true;
	bRandomDirect = false;
	srand(time(NULL));
	int iRandomStart = rand() % 36;
	int i = iRandomStart;
	bool bDoubleStart = false;
	bool bLoop = true;
	while (bLoop) {

		i += bRandomDirect ? 1 : -1;

		if (bRandomDirect && i > 36) {
			i = 0;
			bDoubleStart = true;
		}
		else if (!bRandomDirect && i < 0) {
			i = 36;
			bDoubleStart = true;
		}

		if (bRandomDirect && i >= iRandomStart && bDoubleStart) {
			bLoop = false;
			break;
		}
		else if (!bRandomDirect && i <= iRandomStart && bDoubleStart) {
			bLoop = false;
			break;
		}

		Trace_t TraceInit;
		Vector vecProjectSecondPoint = ProjectPoint(vecOrigin, M::NormalizeYaw(i * 10), 300) + Vector(0, 0, 90);
		i::EngineTrace->TraceRay(Ray_t(vecOrigin, vecProjectSecondPoint), MASK_SOLID, &filter1, &TraceInit);
		AIVizualization.push_front(TraceInit.vecEnd);

		if (TraceInit.flFraction > flBestDistance) {
			flBestAngle = M::NormalizeYaw(i * 10);
			flBestDistance = TraceInit.flFraction;
		}
	}

	flBestAngle = M::NormalizeYaw(flBestAngle - 90);

	float flDelta = M::NormalizeYaw(flBestAngle - vecEyeAngle.y);

	if (flDelta >= 0)
		vecEyeAngle.y = M::NormalizeYaw(vecEyeAngle.y + ((abs(flDelta) <= 5) ? abs(flDelta) : 5));
	else
		vecEyeAngle.y = M::NormalizeYaw(vecEyeAngle.y - ((abs(flDelta) <= 5) ? abs(flDelta) : 5));

	Trace_t TraceLeft;
	Vector vecProjectThirdPoint = ProjectPoint(vecOrigin, M::NormalizeYaw((vecEyeAngle.y - 90) - 45), 300);
	i::EngineTrace->TraceRay(Ray_t(vecOrigin, vecProjectThirdPoint), MASK_SOLID, &filter1, &TraceLeft);
	AIVizualization.push_front(TraceLeft.vecEnd);

	Trace_t TraceRight;
	Vector vecProjectFourthPoint = ProjectPoint(vecOrigin, M::NormalizeYaw((vecEyeAngle.y - 90) + 45), 300);
	i::EngineTrace->TraceRay(Ray_t(vecOrigin, vecProjectFourthPoint), MASK_SOLID, &filter1, &TraceRight);
	AIVizualization.push_front(TraceRight.vecEnd);


	if (TraceLeft.flFraction > TraceRight.flFraction)
		pCmd->flSideMove = -100.f;
	else
		pCmd->flSideMove = 100.f;

	vecEyeAngle.x = 0.f;

	i::EngineClient->SetViewAngles(vecEyeAngle);

	while (AIVizualization.size() >= 32)
		AIVizualization.pop_back();
}

void misc::MoveToPosition(Vector& vecPosition) {

	//Vector vecOriginDelta = vecPosition - g::pLocal->GetAbsOrigin();

	//auto deg2rad = M_DEG2RAD(M::NormalizeAngle(g::pCmd->angViewPoint.y));

	//auto flSideMove = ((cos(deg2rad) * -vecOriginDelta.y) + (sin(deg2rad) * vecOriginDelta.x));
	//auto flForwardMove = ((sin(deg2rad) * vecOriginDelta.y) + (cos(deg2rad) * vecOriginDelta.x));

	//g::pCmd->flSideMove = std::clamp(flSideMove, -450.f, 450.f);
	//g::pCmd->flForwardMove = std::clamp(flForwardMove, -450.f, 450.f);

	Vector vecMoveDirection;
	M::VectorAngles(vecPosition - g::pLocal->GetEyePosition(), vecMoveDirection);

	Vector vecSetAngle = Vector(g::vecOriginalViewAngle.x, vecMoveDirection.y, g::vecOriginalViewAngle.z);
	i::EngineClient->SetViewAngles(vecSetAngle);
	g::pCmd->flForwardMove = 450.f;
	g::pCmd->iButtons |= IN_JUMP;
}

void Friction(float flFriction, Vector* vecVelocity)
{
	static CConVar* sv_friction = i::ConVar->FindVar("sv_friction");
	static CConVar* sv_stopspeed = i::ConVar->FindVar("sv_stopspeed");
	float	speed, newspeed, control;
	float	friction;
	float	drop;

	// Calculate speed
	speed = g::pLocal->GetVelocity().Length();

	// If too slow, return
	if (speed < 0.1f)
	{
		return;
	}

	drop = 0;

	friction = sv_friction->GetFloat() * flFriction;

	control = (speed < sv_stopspeed->GetFloat()) ? sv_stopspeed->GetFloat() : speed;

	drop += control * friction * i::GlobalVars->flFrameTime;

	newspeed = speed - drop;
	if (newspeed < 0)
		newspeed = 0;

	if (newspeed != speed)
		newspeed /= speed;

	*vecVelocity -= (g::pLocal->GetVelocity() * (1.f - newspeed));
}

void misc::BlockBot(CUserCmd* pCmd) {

	if (!g::pLocal || !g::pLocal->IsAlive() || !cfg::misc::blockbot || !GetAsyncKeyState(cfg::misc::blockbotKey))
		return;

	if (CBaseEntity* pBlockedPlayer = reinterpret_cast<CBaseEntity*>(i::EntityList->GetClientEntityFromHandle(g::pLocal->GetGroundEntity())); 
		pBlockedPlayer != nullptr && 
		pBlockedPlayer->IsPlayer()) {

		Vector vecExtrapolatedLocalPos = (g::pLocal->GetVecOrigin() + (g::pLocal->GetVelocity() * (i::GlobalVars->flIntervalPerTick * 3)));

		// allowed difference before we fall down cuz head has a bigger surface (idk why)
		// so if player is going in small circles, we won't fall (or doing small changes that could kill most blockbots)
		if ((vecExtrapolatedLocalPos - pBlockedPlayer->GetVecOrigin()).Length2D() > 3.29217472f) {

			Vector vecAngle;
			M::VectorAngles(pBlockedPlayer->GetVecOrigin() - vecExtrapolatedLocalPos, vecAngle);

			g::vecOriginalViewAngle.y = vecAngle.y;
			g::pCmd->flForwardMove = 450.f;
			g::pCmd->flSideMove = 0.f;
		}
	}
	else {

		float flBestDistance = 250.f;
		CBaseEntity* pTarget = nullptr;

		for (int i = 1; i < i::GlobalVars->nMaxClients; i++)
		{
			CBaseEntity* pEntity = reinterpret_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

			if (!pEntity || !pEntity->IsAlive() || pEntity->IsDormant() || pEntity == g::pLocal)
				continue;

			float flDistance = g::pLocal->GetVecOrigin().DistTo(pEntity->GetVecOrigin());

			if (flDistance < flBestDistance) {

				flBestDistance = flDistance;
				pTarget = pEntity;
			}
		}

		if (!pTarget)
			return;

		Vector vecExtrapolatedLocalPos = (g::pLocal->GetVecOrigin() + (g::pLocal->GetVelocity() * (i::GlobalVars->flIntervalPerTick * 3)));

		Vector vecAngle;
		M::VectorAngles(pTarget->GetVecOrigin() - vecExtrapolatedLocalPos, vecAngle);

		vecAngle.y -= g::pLocal->GetEyeAngles().y;
		vecAngle.NormalizeAngle();

		if (vecAngle.y < -1.0f)
			pCmd->flSideMove = 450.f;
		else if (vecAngle.y > 1.0f)
			pCmd->flSideMove = -450.f;
	}
}

void misc::ClanTag() {

	static auto SetClan = [](const char* csTag, const char* name) {

		static auto pSetClantag = reinterpret_cast<void(__fastcall*)(const char*, const char*)>(util::FindSignature("engine.dll", "53 56 57 8B DA 8B F9 FF"));
		pSetClantag(csTag, name);
	};

	static bool bShouldPrint = true;
	INetChannelInfo* pNetChannel = i::EngineClient->GetNetChannelInfo();

	if (!pNetChannel)
		return;

	if (!bShouldPrint && !cfg::misc::clantag)
		return;

	static float flTime = 1;
	int iTicks = TIME_TO_TICKS(pNetChannel->GetAvgLatency(FLOW_OUTGOING)) + (float)i::GlobalVars->iTickCount;
	float intervals = 0.4f / i::GlobalVars->flIntervalPerTick;
	int iMainTime = (int)(iTicks / intervals) % 17;
	if (iMainTime != flTime)
	{
		if (cfg::misc::clantag) {

			bShouldPrint = true;
			switch (iMainTime) {

			case 0: SetClan("R", "R"); break;
			case 1: SetClan("TR", "TR"); break;
			case 2: SetClan("XTR", "XTR"); break;
			case 3: SetClan("eXTR", "eXTR"); break;
			case 4: SetClan("zeXTR", "zeXTR"); break;
			case 5: SetClan("yzeXTR", "yzeXTR"); break;
			case 6: SetClan("RyzeXTR", "RyzeXTR"); break;
			case 7: SetClan("RyzeXTR", "RyzeXTR"); break;
			case 8: SetClan("RyzeXTR", "RyzeXTR"); break;
			case 9: SetClan("RyzeXTR", "RyzeXTR"); break;
			case 10: SetClan("RyzeXT", "RyzeXT"); break;
			case 11: SetClan("RyzeX", "RyzeX"); break;
			case 12: SetClan("Ryze", "Ryze"); break;
			case 13: SetClan("Ryz", "Ryz"); break;
			case 14: SetClan("Ry", "Ry"); break;
			case 15: SetClan("R", "R"); break;
			case 16: SetClan("", ""); break;
			case 17: SetClan("", ""); break;
			}
		}
		else {
			bShouldPrint = false;
			SetClan(" ", " ");
		}
	}
	flTime = iMainTime;
}

//void misc::CustomBombText(const char* szText) {
//
//	if (!g::pLocal || !g::pLocal->IsAlive())
//		return;
//
//	if (!g::pLocal->GetWeapon() || g::pLocal->GetWeapon()->GetItemDefinitionIndex() != WEAPON_C4)
//		return;
//
//	C4* pWeapon = reinterpret_cast<C4*>(reinterpret_cast<CWeaponCSBase*>(g::pLocal->GetWeapon()));
//	CBaseViewModel* pViewmodel = reinterpret_cast<CBaseViewModel*>(g::pLocal->GetViewModel());
//	CBaseEntity* pViewmodelEntity = reinterpret_cast<CBaseEntity*>(pViewmodel);
//	
//	if (!pWeapon || !pViewmodel || !pViewmodelEntity)
//		return;
//
//	pWeapon->OnFireEvent(pViewmodel, Vector(0, 0, 0), Vector(0, 0, 0), 7001, szText);
//}