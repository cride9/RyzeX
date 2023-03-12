#include "enginepred.h"
#include "../../globals.h"
#include "../Rage/Animations/LocalAnimation.h"

void PrePrediction() {

	g::localprediction::before::nFlags = g::pLocal->GetFlags();
	g::localprediction::before::nMoveType = g::pLocal->GetMoveType();
}

void PostPrediction() {

	g::localprediction::after::nFlags = g::pLocal->GetFlags();
	g::localprediction::after::nMoveType = g::pLocal->GetMoveType();
}

void Prediction::Start(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	// @xref: "CPrediction::ProcessMovement"

	if (!pLocal->IsAlive() || i::MoveHelper == nullptr)
		return;

	PrePrediction();

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

		pLocal->Think();
	}

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

	PostPrediction();
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

void Prediction::SaveNetvars( int iCommand )
{
	localanim.localdata.oldAbsOrigin = g::pLocal->GetAbsOrigin();
	localanim.localdata.oldSpawnTime = g::pLocal->GetSpawnTime();

	pNetvarData[ iCommand % 150 ].fFlags = g::pLocal->GetFlags( );
	pNetvarData[ iCommand % 150 ].flDuckAmount = g::pLocal->GetDuckAmount( );
	pNetvarData[ iCommand % 150 ].flDuckSpeed = g::pLocal->GetDuckSpeed( );
	pNetvarData[ iCommand % 150 ].vecOrigin = g::pLocal->GetVecOrigin( );
	pNetvarData[ iCommand % 150 ].vecVelocity = g::pLocal->GetVelocity( );
	pNetvarData[ iCommand % 150 ].vecBaseVelocity = g::pLocal->GetVecBaseVelocity( );
	pNetvarData[ iCommand % 150 ].flFallVelocity = g::pLocal->GetFallVelocity( );
	pNetvarData[ iCommand % 150 ].vecViewOffset = g::pLocal->GetViewOffset( );
	pNetvarData[ iCommand % 150 ].vecAimPunchAngle = g::pLocal->GetAimPunch( );
	pNetvarData[ iCommand % 150 ].vecAimPunchAngleVel = g::pLocal->GetAimPunchVelocity( );
	pNetvarData[ iCommand % 150 ].vecViewPunchAngle = g::pLocal->GetViewPunch( );

	CBaseCombatWeapon* pWeapon = g::pLocal->GetWeapon( );
	if ( !pWeapon )
		return;

	pNetvarData[ iCommand % 150 ].flRecoilIndex = static_cast< CWeaponCSBase* >(pWeapon)->GetRecoilIndex( );
	pNetvarData[ iCommand % 150 ].flAccuracyPenalty = static_cast< CWeaponCSBase* >(pWeapon)->GetAccuracyPenalty( );
}

void Prediction::RestoreNetvars( int iCommand )
{
	g::pLocal->GetFlags( ) = pNetvarData[ iCommand % 150 ].fFlags;
	g::pLocal->GetDuckAmount( ) = pNetvarData[ iCommand % 150 ].flDuckAmount;
	g::pLocal->GetDuckSpeed( ) = pNetvarData[ iCommand % 150 ].flDuckSpeed;
	g::pLocal->GetVecOrigin( ) = pNetvarData[ iCommand % 150 ].vecOrigin;
	g::pLocal->GetVelocity( ) = pNetvarData[ iCommand % 150 ].vecVelocity;
	g::pLocal->GetVecBaseVelocity( ) = pNetvarData[ iCommand % 150 ].vecBaseVelocity;
	g::pLocal->GetFallVelocity( ) = pNetvarData[ iCommand % 150 ].flFallVelocity;
	g::pLocal->GetViewOffset( ) = pNetvarData[ iCommand % 150 ].vecViewOffset;
	g::pLocal->GetAimPunch( ) = pNetvarData[ iCommand % 150 ].vecAimPunchAngle;
	g::pLocal->GetAimPunchVelocity( ) = pNetvarData[ iCommand % 150 ].vecAimPunchAngleVel;
	g::pLocal->GetViewPunch( ) = pNetvarData[ iCommand % 150 ].vecViewPunchAngle;

	CBaseCombatWeapon* pWeapon = g::pLocal->GetWeapon( );
	if ( !pWeapon )
		return;

	static_cast< CWeaponCSBase* >( pWeapon )->GetRecoilIndex( ) = pNetvarData[ iCommand % 150 ].flRecoilIndex;
	static_cast< CWeaponCSBase* >( pWeapon )->GetAccuracyPenalty( ) = pNetvarData[ iCommand % 150 ].flAccuracyPenalty;
}

void Prediction::SaveViewmodelData( )
{
	CBaseViewModel* const hViewmodel = ( CBaseViewModel* )i::EntityList->GetClientEntityFromHandle( g::pLocal->GetViewModel( ) );

	if ( !hViewmodel )
		return;

	iAnimationParity = hViewmodel->iAnimationParity( );
	iSequence = hViewmodel->iSequence( );
	//flCycle = hViewmodel->flCycle( );
	//flAnimTime = hViewmodel->flAnimTime( );
}

void Prediction::AdjustViewmodelData( )
{
	CBaseViewModel* const hViewmodel = ( CBaseViewModel* )i::EntityList->GetClientEntityFromHandle( g::pLocal->GetViewModel( ) );

	if ( !hViewmodel )
		return;

	if ( iSequence != hViewmodel->iSequence( ) || iAnimationParity != hViewmodel->iAnimationParity( ) )
		return;

	//hViewmodel->flCycle( ) = flCycle;
	//hViewmodel->flAnimTime( ) = flAnimTime;
}