#include "aimbot.h"

void CAimBot::CreateMove(CUserCmd* pCmd, CBaseEntity* pLocal) {

	static CConVar* recoilScale = i::ConVar->FindVar(XorStr("weapon_recoil_scale"));

	if (!cfg::rage::bEnable)
		return;

	exploits::bCanCharge = true;
	misc::RevolverCreateMove();
	aimData = rageBotData_t();
	CBaseCombatWeapon* pWeapon = pLocal->GetWeapon();
	if (pWeapon == nullptr)
		return;

	if (pWeapon->IsGrenade() || pWeapon->IsKnife() || !pWeapon->IsWeapon())
		return;

	if (!pWeapon->GetAmmo())
		return;

	curConfig = GetWeaponConfiguration(pWeapon->GetItemDefinitionIndex());
	curConfig.pWeapon = pWeapon;
	std::vector<Lagcompensation::AnimationInfo_t*> vecTargets = GetTargetableEntities(pLocal);

	if (vecTargets.empty())
		return;

	vecEyePosition = pLocal->GetEyePosition(false);
	Vector vecShootPosition = ScanHitboxes(vecTargets, pLocal);

	if (aimData.pRecord == nullptr || vecShootPosition == Vector(0, 0, 0))
		return;

	exploits::bCanCharge = false;
	if (curConfig.bAutoScope && !pLocal->IsScoped() && !pLocal->IsResumingScope())
		pCmd->iButtons |= IN_ZOOM;

	Vector vecAimAngle = (M::VectorAngles(vecShootPosition - vecEyePosition) -= (pLocal->GetAimPunch() * recoilScale->GetFloat()));

	aimData.bCanShoot = HitChance(pCmd, pLocal, vecShootPosition, vecAimAngle, aimData.pRecord);
	AutoStop(pLocal, pCmd);

	if (!aimData.bCanShoot) 
		return;
	
	pCmd->angViewPoint = vecAimAngle;
	if (!cfg::rage::bSilentAim) i::EngineClient->SetViewAngles(vecAimAngle);

	pCmd->iButtons |= IN_ATTACK;
	aimData.iTickcount = pCmd->iTickCount;

	iTickCount = TIME_TO_TICKS(aimData.flTargetSimulation + lagcomp.GetClientInterpAmount());
	bShouldSendPacket = true;
	hitlogData = aimData;
}

void CAimBot::PostPrediction(CUserCmd* pCmd, bool& bSendPacket) {

	if (bShouldSendPacket) {

		bSendPacket = true;
		pCmd->iTickCount = iTickCount;
		bShouldSendPacket = false;
	}
}

Vector CAimBot::ScanHitboxes(std::vector<Lagcompensation::AnimationInfo_t*>& vecIn, CBaseEntity* pLocal) {

	std::vector<Hitscan_t> vecHitscan{};
	for (Lagcompensation::AnimationInfo_t* it : vecIn) {

		if (it->iLastValid >= it->pRecord.size())
			continue;

		for (size_t i = 0; i < it->iLastValid; i++) {

			Lagcompensation::LagRecord_t* pRecord = &it->pRecord.at(i);
			for (auto& iHitbox : curConfig.vecHitboxes[NORMAL]) {
				bool bShouldMultiPoint = std::find(curConfig.vecHitboxes[MULTIPOINT].begin(), curConfig.vecHitboxes[MULTIPOINT].end(), iHitbox) != curConfig.vecHitboxes[MULTIPOINT].end();
				bool bShouldForceSafePoint = std::find(curConfig.vecHitboxes[SAFE].begin(), curConfig.vecHitboxes[SAFE].end(), iHitbox) != curConfig.vecHitboxes[SAFE].end();
				bool bShouldSafe = curConfig.bSafePoint;

				if (cfg::rage::iAimbotFov < 180) { // we don't wanna multipoint if they are outside of our fov
					float flRadius = 0.f;
					Vector vecCenter = pRecord->pEntity->GetHitboxPosition(iHitbox, pRecord->pMatricies[RESOLVE], flRadius);

					Vector output;
					M::VectorAngles(vecCenter - vecEyePosition, output);
					Vector vecDistanceBetween = (g::vecOriginalViewAngle - output.NormalizeAngle());

					if (abs((vecDistanceBetween).NormalizeAngle().Length2D()) > cfg::rage::iAimbotFov)
						continue;
				}

				std::vector<Vector> vecWorldPoints = CreatePoints(vecEyePosition, curConfig.pWeapon, pRecord, iHitbox, RESOLVE, bShouldMultiPoint);
				for (Vector& vecHitboxPoint : vecWorldPoints) {

					if (bShouldSafe) {

						int iCollidePoints = autowall.SafePoint(vecEyePosition, curConfig.pWeapon, pRecord, vecHitboxPoint, iHitbox);
						if (iCollidePoints < 2)
							continue;

						if (bShouldForceSafePoint && iCollidePoints < 3)
							continue;
					}

					FireBulletData_t pData;
					float flDamage = autowall.GetDamage(pLocal, vecEyePosition, vecHitboxPoint, curConfig.pWeapon, &pData);
					if (flDamage != -1.f) {
						vecHitscan.push_back(Hitscan_t(pRecord, vecHitboxPoint, pData, iHitbox != HITBOX_HEAD && flDamage > pRecord->pEntity->GetHealth(), (bShouldSafe || bShouldForceSafePoint)));
						if (iHitbox != HITBOX_HEAD)
							break;
					}
				}
			}
		}
	}

	if (vecHitscan.empty())
		return Vector(0, 0, 0);

	std::sort(vecHitscan.begin(), vecHitscan.end());

	for (auto& refRecord : vecHitscan) {

		float flTransformedDamage = curConfig.iMinimumDamage;
		if (curConfig.iMinimumDamage > 100)
			flTransformedDamage = refRecord.pRecord->pEntity->GetHealth() + (curConfig.iMinimumDamage - 100);

		if (refRecord.flDamage < flTransformedDamage)
			continue;

		aimData.SetTarget(refRecord.pRecord, vecEyePosition, refRecord.bBacktrack);
		aimData.flDamage = refRecord.flDamage;
		aimData.iHitbox = refRecord.iHitbox;
		aimData.iHitGroup = refRecord.iHitgroup;
		aimData.vecTargetShootPosition = refRecord.vecPoint;

		return refRecord.vecPoint;
	}
}

void CAimBot::AutoStop(CBaseEntity* pLocal, CUserCmd* pCmd) {

	static CConVar* weapon_accuracy_nospread = i::ConVar->FindVar(XorStr("weapon_accuracy_nospread"));
	if (!curConfig.bAutostop)
		return;

	if (aimData.bCanShoot && !curConfig.bConditions[CONDITION_BETWEEN_SHOTS])
		return;

	if (!pLocal->CanShoot(curConfig.pWeapon) && !curConfig.bConditions[CONDITION_BETWEEN_SHOTS])
		return;

	if ((pCmd->iButtons & IN_JUMP || !(pLocal->GetFlags() & FL_ONGROUND)) && !curConfig.bConditions[CONDITION_INAIR])
		return;

	if (weapon_accuracy_nospread->GetInt() == 1)
		return;

	float flMultiplier = 0.28f;
	switch (curConfig.iAutostopValue) {
	case 1: flMultiplier = 0.26f;
		break;
	case 2: flMultiplier = 0.24f;
		break;
	case 3: flMultiplier = 0.20f;
		break;
	}

	float flIdealSpeed = (flMultiplier) * (pLocal->IsScoped() ? curConfig.pWeapon->GetCSWpnData()->flMaxSpeed[1] : curConfig.pWeapon->GetCSWpnData()->flMaxSpeed[0]);

	pCmd->iButtons &= ~IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT;

	// Get the ideal speed for shooting (playstyle)
	Vector vecVelocity = pLocal->GetVelocity();
	Vector vecDirection;
	Vector vecRealView;

	if (flIdealSpeed > vecVelocity.Length2D())
		return;

	M::VectorAngles(vecVelocity, vecDirection);
	i::EngineClient->GetViewAngles(vecRealView);

	vecDirection.y = vecRealView.y - vecDirection.y;

	Vector vecForward;
	M::AngleVectors(vecDirection, &vecForward);

	static CConVar* cl_forwardspeed = i::ConVar->FindVar(XorStr("cl_forwardspeed"));
	static CConVar* cl_sidespeed = i::ConVar->FindVar(XorStr("cl_sidespeed"));

	auto flNegativeForwardSpeed = -cl_forwardspeed->GetFloat();
	auto flNegativeSideSpeed = -cl_sidespeed->GetFloat();

	auto flNegativeForwardDirection = vecForward * flNegativeForwardSpeed;
	auto flNegativeSideDirection = vecForward * flNegativeSideSpeed;

	pCmd->flForwardMove = flNegativeForwardDirection.x;
	pCmd->flSideMove = flNegativeSideDirection.y;
}

bool CAimBot::HitChance(CUserCmd* pCmd, CBaseEntity* pLocal, Vector vecWorldPosition, Vector vecAimPosition, Lagcompensation::LagRecord_t* pRecord) {

	matrix3x4_t* pMatrix = pRecord->pMatricies[RESOLVE];

	if (curConfig.iHitchance <= 1)
		return true;

	CBaseCombatWeapon* pWeapon = curConfig.pWeapon;
	const CCSWeaponInfo* pWeaponData = curConfig.pWeapon->GetCSWpnData();
	const Vector vecShootPosition = vecEyePosition;
	const float flGetSpread = prediction.flSpread;
	const float flGetInaccuracy = prediction.flInaccuracy;

	if (vecShootPosition.DistTo(vecWorldPosition) > pWeaponData->flRange)
		return 0.f;

	if (HasEnoughAccuracy(pLocal, flGetInaccuracy))
		return 100.f;

	Vector vecForward = Vector(0, 0, 0);
	Vector vecRight = Vector(0, 0, 0);
	Vector vecUp = Vector(0, 0, 0);

	M::AngleVectors(vecAimPosition, &vecForward, &vecRight, &vecUp);

	vecForward.Normalize();
	vecRight.Normalize();
	vecUp.Normalize();

	bool bSpecialWeapon = pWeapon->GetItemDefinitionIndex() == EItemDefinitionIndex::WEAPON_AWP || pWeapon->GetItemDefinitionIndex() == EItemDefinitionIndex::WEAPON_SSG08;
	int iAccuracry = bSpecialWeapon ? 256 : 128;

	static bool bSetupSpreadValues = true;
	static float flSpreadValues[256][6];

	if (bSetupSpreadValues)
	{
		bSetupSpreadValues = false;

		for (auto i = 0; i < iAccuracry; ++i)
		{
			M::RandomSeed(i + 1);

			float a = M::RandomFloat(0.0f, 1.0f);
			float b = M::RandomFloat(0.0f, 6.283185307f);
			float c = M::RandomFloat(0.0f, 1.0f);
			float d = M::RandomFloat(0.0f, 6.283185307f);

			flSpreadValues[i][0] = a;
			flSpreadValues[i][1] = c;

			auto flSinB = 0.0f, flCosB = 0.0f;
			M::SinCos(b, &flSinB, &flCosB);

			auto flSinD = 0.0f, flCosD = 0.0f;
			M::SinCos(b, &flSinD, &flCosD);

			flSpreadValues[i][2] = flSinB;
			flSpreadValues[i][3] = flCosB;
			flSpreadValues[i][4] = flSinD;
			flSpreadValues[i][5] = flCosD;
		}
	}

	int iHits = 0;
	for (auto i = 0; i < iAccuracry; ++i)
	{
		float flInacc = flSpreadValues[i][0] * flGetInaccuracy;
		float flSpread = flSpreadValues[i][1] * flGetSpread;

		float flSpreadX = flSpreadValues[i][3] * flInacc + flSpreadValues[i][5] * flSpread;
		float flSpreadY = flSpreadValues[i][2] * flInacc + flSpreadValues[i][4] * flSpread;

		Vector vecDirection = Vector(0, 0, 0);
		vecDirection.x = vecForward.x + vecRight.x * flSpreadX + vecUp.x * flSpreadY;
		vecDirection.y = vecForward.y + vecRight.y * flSpreadX + vecUp.y * flSpreadY;
		vecDirection.z = vecForward.z + vecRight.z * flSpreadX + vecUp.z * flSpreadY;

		Vector vecEnd = vecEyePosition + vecDirection * pWeaponData->flRange;

		if (autowall.bCollidePoint(vecShootPosition, vecEnd, aimData.pRecord->pEntity->StudioHitbox(aimData.iHitbox), pMatrix))
			iHits++;
	}

	float flFinalHitchance = static_cast<int>((float(iHits) / (iAccuracry / 100.f)));

	if (flFinalHitchance >= curConfig.iHitchance) {
		aimData.flHitchance = flFinalHitchance;
		return true;
	}

	return false;
}

bool CAimBot::HasEnoughAccuracy(CBaseEntity* pLocal, float flWeaponInAccuracy) {

	static CConVar* weapon_accuracy_nospread = i::ConVar->FindVar(XorStr("weapon_accuracy_nospread"));
	if (weapon_accuracy_nospread->GetInt() == 1)
		return true;

	CBaseCombatWeapon* pWeapon = curConfig.pWeapon;

	// dont spamshot please
	if (pWeapon->GetNextPrimaryAttack() == i::GlobalVars->flCurrentTime)
		return false;

	// jumpscout
	if (pWeapon->GetItemDefinitionIndex() == WEAPON_SSG08 || pWeapon->GetItemDefinitionIndex() == WEAPON_REVOLVER)
		if (!(pLocal->GetFlags() & FL_ONGROUND))
			if (flWeaponInAccuracy <= 0.009f)
				return true;

	return false;
}

weaponConfig_t CAimBot::GetWeaponConfiguration(short iItemDefinitionIndex) {

	using namespace cfg::rage;

	int iWeapon = OTHER;
	
	if (iItemDefinitionIndex == WEAPON_USP_SILENCER || iItemDefinitionIndex == WEAPON_HKP2000 || iItemDefinitionIndex == WEAPON_ELITE || iItemDefinitionIndex == WEAPON_P250 || iItemDefinitionIndex == WEAPON_FIVESEVEN || iItemDefinitionIndex == WEAPON_CZ75A || iItemDefinitionIndex == WEAPON_GLOCK || iItemDefinitionIndex == WEAPON_TEC9)
		iWeapon = PISTOL;
	else if (iItemDefinitionIndex == WEAPON_REVOLVER || iItemDefinitionIndex == WEAPON_DEAGLE)
		iWeapon = HEAVY_PISTOL;
	else if (iItemDefinitionIndex == WEAPON_SSG08)
		iWeapon = SCOUT;
	else if (iItemDefinitionIndex == WEAPON_AWP)
		iWeapon = AWP;
	else if (iItemDefinitionIndex == WEAPON_SCAR20 || iItemDefinitionIndex == WEAPON_G3SG1)
		iWeapon = AUTO;
	else if (iItemDefinitionIndex == WEAPON_TASER)
		iWeapon = ZEUS;

	weaponConfig_t ret{};

	ret.iMinimumDamage = (bOverride && IPT::HandleInput(iOverrideBind)) ? iOverride[iWeapon] : iMinDamages[iWeapon];
	ret.iHitchance = iHitchances[iWeapon];
	ret.iHeadScale = iHeadPoints[iWeapon];
	ret.iBodyScale = iBodyPoints[iWeapon];
	ret.bSafePoint = bSafePoint[iWeapon];
	ret.bAutostop = bAutostop[iWeapon];
	ret.iAutostopValue = bAutostopAggressiveness[iWeapon];
	ret.bConditions[CONDITION_BETWEEN_SHOTS] = bConditions[iWeapon][CONDITION_BETWEEN_SHOTS];
	ret.bConditions[CONDITION_INAIR] = bConditions[iWeapon][CONDITION_INAIR];
	ret.bAutoScope = (iWeapon == SCOUT || iWeapon == AWP || iWeapon == AUTO) ? bAutoScope[iWeapon] : false;
	GetHitBoxes(NORMAL, ret.vecHitboxes[NORMAL], iWeapon);
	GetHitBoxes(MULTIPOINT, ret.vecHitboxes[MULTIPOINT], iWeapon);
	GetHitBoxes(SAFE, ret.vecHitboxes[SAFE], iWeapon);

	return ret;
}

void CAimBot::GetHitBoxes(int i, std::vector<int>& vecOut, int iWeapon) {

	if (i::ConVar->FindVar(XorStr("mp_damage_headshot_only"))->GetBool()) {
		vecOut.push_back(HITBOX_HEAD);
		return;
	}

	if (cfg::rage::bForceBaim && IPT::HandleInput(cfg::rage::iForceBaimKey)) {

		vecOut.push_back(HITBOX_STOMACH);
		vecOut.push_back(HITBOX_CHEST);
		return;
	}

	for (size_t iIndex = 0; iIndex < 6; iIndex++)
	{
		if (!cfg::rage::bHitboxes[i][iWeapon][iIndex])
			continue;

		switch (iIndex)
		{
		case 0:
			vecOut.push_back(HITBOX_HEAD);
			break;

		case 1:
			vecOut.push_back(HITBOX_UPPER_CHEST);
			break;

		case 2:
			vecOut.push_back(HITBOX_CHEST);
			break;

		case 3:
			vecOut.push_back(HITBOX_STOMACH);
			break;

		case 4:
			vecOut.push_back(HITBOX_LEFT_UPPER_ARM);
			vecOut.push_back(HITBOX_RIGHT_UPPER_ARM);
			vecOut.push_back(HITBOX_RIGHT_FOREARM);
			vecOut.push_back(HITBOX_LEFT_FOREARM);
			break;

		case 5:
			vecOut.push_back(HITBOX_RIGHT_FOOT);
			vecOut.push_back(HITBOX_LEFT_FOOT);
			break;
		}
	}
}

std::vector<Lagcompensation::AnimationInfo_t*> CAimBot::GetTargetableEntities(CBaseEntity* pLocal) {

	std::vector<Lagcompensation::AnimationInfo_t*> ret{};
	for (size_t i = 1; i < i::GlobalVars->nMaxClients; i++) {

		CBaseEntity* pEntity = static_cast<CBaseEntity*>(i::EntityList->GetClientEntity(i));

		if (!pEntity || !pEntity->IsEnemy(pLocal) || !pEntity->IsAlive() || pEntity->IsDormant() || pEntity->HasImmunity())
			continue;

		Lagcompensation::AnimationInfo_t* pLog = &lagcomp.GetLog(i);
		if (!pLog || pLog->pRecord.empty())
			continue;

		ret.push_back(pLog);
	}

	if (!ret.empty())
		std::sort(ret.begin(), ret.end(), 
			[](Lagcompensation::AnimationInfo_t* pLog1, Lagcompensation::AnimationInfo_t* pLog2) {
			return pLog1->pEntity->GetHealth() < pLog2->pEntity->GetHealth(); });

	return ret;
}

std::vector<Vector> CAimBot::CreatePoints(Vector vecEyePosition, CBaseCombatWeapon* pWeapon, Lagcompensation::LagRecord_t* pRecord, int iHitbox, EMatrixType iType, bool bShouldMultipoint) {

	std::vector<Vector> refVecPoints{};
	int* pHeadPoints = &curConfig.iHeadScale;
	int* pBodyPoints = &curConfig.iBodyScale;

	float flRadius = 0.f;
	mstudiobbox_t* refStudioBox = pRecord->pEntity->StudioHitbox(iHitbox);
	Vector vecCenter = pRecord->pEntity->GetHitboxPosition(iHitbox, pRecord->pMatricies[iType], flRadius);

	if (!refStudioBox || flRadius < 0 || !bShouldMultipoint) {
		refVecPoints.push_back(vecCenter);
		return refVecPoints;
	}

	matrix3x4_t& refMatrixBone = pRecord->pMatricies[iType][refStudioBox->iBone];

	float flHitboxDistance = flRadius * ((iHitbox == HITBOX_HEAD ? *pHeadPoints : *pBodyPoints) * 0.01f);

	Vector vecCurrentAngles;
	M::VectorAngles(vecCenter - vecEyePosition, vecCurrentAngles);
	Vector vecForward; M::AngleVectors(vecCurrentAngles, &vecForward);

	const Vector vecRight = vecForward.CrossProduct(Vector(0, 0, 1));
	const Vector vecLeft = Vector(-vecRight.x, -vecRight.y, vecRight.z);
	const Vector vecTop = Vector(0, 0, 1);

	refVecPoints.push_back(vecCenter);
	if (iHitbox == HITBOX_HEAD) {
		refVecPoints.push_back(vecCenter + vecTop * flHitboxDistance);
		refVecPoints.push_back(vecCenter - vecTop * flHitboxDistance);
		refVecPoints.push_back(vecCenter + vecRight * (flHitboxDistance * 0.5f));
		refVecPoints.push_back(vecCenter + vecLeft * (flHitboxDistance * 0.5f));

		refVecPoints.push_back(vecCenter + (vecTop * flHitboxDistance) + (vecLeft * (flHitboxDistance * 0.5f)));
		refVecPoints.push_back(vecCenter + (vecTop * flHitboxDistance) + (vecRight * (flHitboxDistance * 0.5f)));
	}
	refVecPoints.push_back(vecCenter + vecLeft * flHitboxDistance);
	refVecPoints.push_back(vecCenter + vecRight * flHitboxDistance);

	return refVecPoints;
}
