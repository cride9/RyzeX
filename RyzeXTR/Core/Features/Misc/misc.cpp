#include "misc.h"
#include <fstream>
#include "../../SDK/Menu/config.h"
#include "../../SDK/math.h"
#include "../Rage/doubletap.h"
#define CheckIfNonValidNumber(x) (fpclassify(x) == FP_INFINITE || fpclassify(x) == FP_NAN || fpclassify(x) == FP_SUBNORMAL)

void misc::CreateMove(CUserCmd* pCmd, Vector& vecViewAngle,bool& bSendPacket) {

	BunnyHop(pCmd);
	FakeLag(bSendPacket);
	AutoStrafe(vecViewAngle, pCmd);
	AspectRatio();
	Slowwalk(pCmd, cfg::antiaim::fakewalk); // need menu element && keybind
	FastStop(pCmd);
	IdealTick(pCmd);
	FakeDuck(pCmd);
	BulletImpact();
	SlideFix();
	DefensiveDoubletap();
	OnlyCheatLogs();
	//ViewModel();
}

void misc::IdealTick(CUserCmd* pCmd) {

	if (!pCmd || !g::pLocal)
		return;

	if (!cfg::antiaim::idealTick)
		return;

	static bool position;

	static Vector origin;

	static Vector FUCK_GO_BACK_WITH_TRANNY_CASES;

	if (GetAsyncKeyState(cfg::antiaim::idealTickBind)) {

		if (!position) {

			position = true;
			origin = g::pLocal->GetAbsOrigin();

		}

	}
	else {

		position = false;
		origin = Vector(0, 0, 0);

	}

	if (position && origin != Vector(0, 0, 0) && GetAsyncKeyState(cfg::antiaim::idealTickBind) && bRetreat) {

		FUCK_GO_BACK_WITH_TRANNY_CASES = origin - g::pLocal->GetAbsOrigin();

		FUCK_GO_BACK_WITH_TRANNY_CASES.Normalize();

		auto sMove = ((cos(M_DEG2RAD(pCmd->angViewPoint.y)) * -FUCK_GO_BACK_WITH_TRANNY_CASES.y) + (sin(M_DEG2RAD(pCmd->angViewPoint.y)) * FUCK_GO_BACK_WITH_TRANNY_CASES.x));
		auto fMove = ((sin(M_DEG2RAD(pCmd->angViewPoint.y)) * FUCK_GO_BACK_WITH_TRANNY_CASES.y) + (cos(M_DEG2RAD(pCmd->angViewPoint.y)) * FUCK_GO_BACK_WITH_TRANNY_CASES.x));

		pCmd->flSideMove = std::clamp(sMove * 50, -450.f, 450.f);
		pCmd->flForwardMove = std::clamp(fMove * 50, -450.f, 450.f);

		if (origin == g::pLocal->GetAbsOrigin())
			bRetreat = false;
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

void misc::DefensiveDoubletap() {

	static int timer = 0;

	//if (cfg::rage::doubletap && GetKeyState(cfg::rage::doubletapkey)) {

	//	if (++timer >= 14)
	//		timer = 0;

	//	if (timer > 0) {

	//		doubletap::defensiveCommandNumber = g::pCmd->iCommandNumber;
	//		doubletap::defensiveTickbase = prediction.GetTickBase(g::pCmd, g::pLocal);
	//		doubletap::defensiveCurtime = i::GlobalVars->flCurrentTime;

	//		g::defensiveTickbase = 16;
	//	}
	//	else {

	//		g::defensiveTickbase = 0;
	//	}
	//}
}

void misc::SlideFix() {

	if (!g::pLocal || !g::pLocal->IsAlive() || g::pLocal->GetMoveType() == MOVETYPE_LADDER)
		return;

	g::pCmd->iButtons &= ~(IN_FORWARD | IN_BACK | IN_MOVERIGHT | IN_MOVELEFT);
}

void misc::BulletImpact() {

	if (!g::pLocal || !g::pLocal->IsAlive())
		return;

	CConVar* sv_bulletimpact = i::ConVar->FindVar("sv_showimpacts");

	sv_bulletimpact->SetValue(cfg::misc::bulletImpact ? 1 : 0);
}

void misc::BuyBot(IGameEvent* event) { // need menu element

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

		static DWORD* _death_notice = (DWORD*)FindHudElement("CCSGO_HudDeathNotice");
		static void(__thiscall * _clear_notices)(DWORD) = (void(__thiscall*)(DWORD))util::FindSignature("client.dll", "55 8B EC 83 EC 0C 53 56 8B 71 58");

		if (_death_notice)
			_death_notice = (DWORD*)FindHudElement("CCSGO_HudDeathNotice");
		if (_clear_notices)
			_clear_notices(((DWORD)_death_notice - 20));
	}
}

void misc::ViewModel() {

	if (!g::pLocal || !g::pLocal->IsAlive())
		return;

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

	if (!cfg::misc::aspectRatio)
		return;

	float ratio = (cfg::misc::aspectRatioValue * 0.1) / 2;
	if (ratio > 0.001)
		i::ConVar->FindVar("r_aspectratio")->SetValue(ratio); //ayyware hhhh
	else
		i::ConVar->FindVar("r_aspectratio")->SetValue((35 * 0.1f) / 2);
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

	static CConVar* svcheats = i::ConVar->FindVar("sv_cheats");

	if (GetKeyState(cfg::misc::thirdpersonbind) && g::pLocal->IsAlive()) {
		svcheats->SetValue(1);
		i::EngineClient->ExecuteClientCmd("thirdperson");
	}
	else {
		i::EngineClient->ExecuteClientCmd("firstperson");
	}
}

void misc::FakeLag(bool& bSendPacket) {

	if (!g::pLocal || !g::pLocal->IsAlive())
		return;

	if (GetAsyncKeyState(cfg::antiaim::fakeduckbind) && cfg::antiaim::fakeduck) {
		bSendPacket = i::ClientState->nChokedCommands >= 14;
		return;
	}

	bSendPacket = i::ClientState->nChokedCommands >= (cfg::antiaim::fakelag ? (cfg::rage::doubletap && GetKeyState(cfg::rage::doubletapkey)) ? g::bWaiting ? cfg::antiaim::fakelag : 2 : cfg::antiaim::fakelag : cfg::antiaim::fakelag);
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