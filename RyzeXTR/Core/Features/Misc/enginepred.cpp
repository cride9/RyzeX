#include "enginepred.h"
#include "../../globals.h"

void Prediction::Start(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	// @xref: "CPrediction::ProcessMovement"

	if (!pLocal->IsAlive() || i::MoveHelper == nullptr)
		return;

	// start command
	*pLocal->GetCurrentCommand() = pCmd;
	pLocal->GetLastCommand() = *pCmd;

	// random seed was already generated in CInput::CreateMove
	*uPredictionRandomSeed = pCmd->iRandomSeed;
	// set ourselves as a predictable entity
	*pPredictionPlayer = uintptr_t(pLocal);

	// backup globals
	flOldCurrentTime = i::GlobalVars->flCurrentTime;
	flOldFrameTime = i::GlobalVars->flFrameTime;
	iOldTickCount = i::GlobalVars->iTickCount;

	// backup tickbase
	const int iOldTickBase = pLocal->GetTickBase();

	// backup prediction states
	const bool bOldIsFirstPrediction = i::Prediction->bIsFirstTimePredicted;
	const bool bOldInPrediction = i::Prediction->bInPrediction;

	// set corrected values
	i::GlobalVars->flCurrentTime = TICKS_TO_TIME(GetTickBase(pCmd, pLocal));
	i::GlobalVars->flFrameTime = i::Prediction->bEnginePaused ? 0.f : TICK_INTERVAL;
	i::GlobalVars->iTickCount = GetTickBase(pCmd, pLocal);

	i::Prediction->bIsFirstTimePredicted = false;
	i::Prediction->bInPrediction = true;

	/* skipped weapon select and vehicle predicts */

	// synchronize m_afButtonForced & m_afButtonDisabled
	pCmd->iButtons |= pLocal->GetButtonForced();
	pCmd->iButtons &= ~(pLocal->GetButtonDisabled());

	i::GameMovement->StartTrackPredictionErrors(pLocal);

	// update button state
	const int iButtons = pCmd->iButtons;
	const int nLocalButtons = *pLocal->GetButtons();
	const int nButtonsChanged = iButtons ^ nLocalButtons;

	// synchronize m_afButtonLast
	pLocal->GetButtonLast() = nLocalButtons;

	// synchronize m_nButtons
	*pLocal->GetButtons() = iButtons;

	// synchronize m_afButtonPressed
	pLocal->GetButtonPressed() = nButtonsChanged & iButtons;

	// synchronize m_afButtonReleased
	pLocal->GetButtonReleased() = nButtonsChanged & (~iButtons);

	// check if the player is standing on a moving entity and adjusts velocity and basevelocity appropriately
	i::Prediction->CheckMovingGround(pLocal, i::GlobalVars->flFrameTime);

	// copy angles from command to player
	i::Prediction->SetLocalViewAngles(pCmd->angViewPoint);

	// run prethink
	if (pLocal->PhysicsRunThink(THINK_FIRE_ALL_FUNCTIONS))
		pLocal->PreThink();

	// run think
	if (int* iNextThinkTick = pLocal->GetNextThink(); *iNextThinkTick > 0 && *iNextThinkTick <= GetTickBase(pCmd, pLocal))
	{
		*iNextThinkTick = TICK_NEVER_THINK;

		/*
		 * handle no think function
		 * pseudo i guess didnt seen before but not sure, most likely unnecessary
		nEFlags = pPlayer->GetEFlags();
		result = pPlayer->GetEFlags() & EFL_NO_THINK_FUNCTION;
		if (!result)
		{
			result = [&]()
			{
				if (pPlayer->GetNextThinkTick() > 0)
					return 1;
				v3 = *(_DWORD *)(pPlayer + 0x2BC);
				v4 = 0;
				if (v3 > 0)
				{
				v5 = (_DWORD *)(*(_DWORD *)(pPlayer + 0x2B0) + 0x14);
				while (*v5 <= 0)
				{
					++v4;
					v5 += 8;
					if (v4 >= v3)
						return 0;
				}
				return 1;
			}();
			if (!result)
				pPlayer->GetEFlags() = nEFlags | EFL_NO_THINK_FUNCTION;
		}

		 */

		pLocal->Think();
	}

	g::predicted::nFlags = pLocal->GetFlags();
	g::predicted::nMoveType = pLocal->GetMoveType();

	// set host player
	i::MoveHelper->SetHost(pLocal);

	// setup move
	i::Prediction->SetupMove(pLocal, pCmd, i::MoveHelper, &moveData);
	i::GameMovement->ProcessMovement(pLocal, &moveData);

	// finish move
	i::Prediction->FinishMove(pLocal, pCmd, &moveData);
	i::MoveHelper->ProcessImpacts();

	// run post think
	pLocal->PostThink();

	// restore tickbase
	pLocal->GetTickBase() = iOldTickBase;

	// restore prediction states
	i::Prediction->bInPrediction = bOldInPrediction;
	i::Prediction->bIsFirstTimePredicted = bOldIsFirstPrediction;
}

void Prediction::End(CUserCmd* pCmd, CBaseEntity* pLocal) const
{
	if (!pLocal->IsAlive() || i::MoveHelper == nullptr)
		return;

	i::GameMovement->FinishTrackPredictionErrors(pLocal);

	// reset host player
	i::MoveHelper->SetHost(nullptr);

	// restore globals
	i::GlobalVars->flCurrentTime = flOldCurrentTime;
	i::GlobalVars->flFrameTime = flOldFrameTime;
	i::GlobalVars->iTickCount = iOldTickCount;

	// finish command
	*pLocal->GetCurrentCommand() = nullptr;

	// reset prediction seed
	*uPredictionRandomSeed = -1;

	// reset prediction entity
	*pPredictionPlayer = 0;

	// reset move
	i::GameMovement->Reset();
}

int Prediction::GetTickBase(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	static int iTick = 0;

	if (pCmd != nullptr)
	{
		static CUserCmd* pLastCmd = nullptr;

		// if command was not predicted - increment tickbase
		if (pLastCmd == nullptr || pLastCmd->bHasBeenPredicted)
			iTick = pLocal->GetTickBase();
		else
			iTick++;

		pLastCmd = pCmd;
	}

	return iTick;
}