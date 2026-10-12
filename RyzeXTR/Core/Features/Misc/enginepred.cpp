#include "enginepred.h"
#include "../../globals.h"
#include "../Networking/networking.h"
#include "../Rage/exploits.h"

void Prediction::OnFrameStage(CBaseEntity* local) {
    if (!local || !local->IsAlive()) return;
    // Equal/regressing simtime is an observation, not a reason to throw it away.
    iObservedSimulationOffset = TIME_TO_TICKS(local->GetSimulationTime())
        - static_cast<int>(i::ClientState->clockDriftMgr.nServerTick);
}
void Prediction::RequestReprediction() {
    // Source layout: vector starts at Split + 0x1c, performed shift at +0x30.
    // Check the local SDK contract and basic live counters before writing it.
    static_assert(offsetof(IPrediction::Split_t, bPerformedTickShift) == 0x30);
    static_assert(offsetof(IPrediction, Split) == 0x18);
    static_assert(offsetof(IPrediction, bOldCLPredictValue) == 0x9);
    static_assert(offsetof(IPrediction, bEnginePaused) == 0xa);
    auto& split = i::Prediction->Split[0];
    if (split.nCommandsPredicted < 0 || split.nCommandsPredicted > 150
        || split.nServerCommandsAcknowledged < 0 || split.nServerCommandsAcknowledged > 150
        || (split.iPreviousAckHadErrors != 0 && split.iPreviousAckHadErrors != 1)) return;
    split.bPerformedTickShift = true;
    i::Prediction->iPreviousStartFrame = -1;
}
void Prediction::Start(CUserCmd* pCmd, CBaseEntity* pLocal, int SequenceNumber )
{
	// @xref: "CPrediction::ProcessMovement"

	active = false;
	if (!pLocal->IsAlive() || i::MoveHelper == nullptr)
		return;
	active = true;
	oldTickbase = pLocal->GetTickBase();
	oldCommand = *pLocal->GetCurrentCommand();
	oldLastCommand = pLocal->GetLastCommand();
	oldSeed = *uPredictionRandomSeed;
	oldPredictionPlayer = *pPredictionPlayer;

	localAnim->BeforePrediction(pLocal);

	// start command
	*pLocal->GetCurrentCommand() = pCmd;
	pLocal->GetLastCommand() = *pCmd;

	// random seed was already generated in CInput::CreateMove
	*uPredictionRandomSeed = pCmd->iRandomSeed;
	// set ourselves as a predictable entity
	*pPredictionPlayer = pLocal;

	// backup globals
	flOldCurrentTime = i::GlobalVars->flCurrentTime;
	flOldFrameTime = i::GlobalVars->flFrameTime;
	iOldTickCount = i::GlobalVars->iTickCount;

	const int TickBase = exploits::GetShotTickbase(pCmd->iCommandNumber, oldTickbase);

	pLastCmd = g::pCmd;

	// backup tickbase
	pLocal->GetTickBase() = TickBase;

	// backup prediction states
	const bool bOldIsFirstPrediction = i::Prediction->Split[0].bIsFirstTimePredicted;
	const bool bOldInPrediction = i::Prediction->bInPrediction;

	// set corrected values
	i::GlobalVars->flCurrentTime = TICKS_TO_TIME( TickBase );
	i::GlobalVars->flFrameTime = i::Prediction->bEnginePaused ? 0.f : TICK_INTERVAL;
	i::GlobalVars->iTickCount = TickBase;

	i::Prediction->Split[0].bIsFirstTimePredicted = false;
	i::Prediction->bInPrediction = true;
	if (CBaseCombatWeapon* pWeapon = pLocal->GetWeapon(); pWeapon) {
		pWeapon->UpdateAccuracyPenalty();
		flSpread = pWeapon->GetSpread();
		flInaccuracy = pWeapon->GetInaccuracy();
	}

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
	if (int* iNextThinkTick = pLocal->GetNextThink(); *iNextThinkTick > 0 && *iNextThinkTick <= TickBase )
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

	// restore prediction states
	i::Prediction->bInPrediction = bOldInPrediction;
	i::Prediction->Split[0].bIsFirstTimePredicted = bOldIsFirstPrediction;
}

void Prediction::End(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	if (!active)
		return;
	active = false;

	i::GameMovement->FinishTrackPredictionErrors(pLocal);

	// reset host player
	i::MoveHelper->SetHost(nullptr);

	// restore globals
	i::GlobalVars->flCurrentTime = flOldCurrentTime;
	i::GlobalVars->flFrameTime = flOldFrameTime;
	i::GlobalVars->iTickCount = iOldTickCount;

	// finish command
	*pLocal->GetCurrentCommand() = oldCommand;
	pLocal->GetLastCommand() = oldLastCommand;
	pLocal->GetTickBase() = oldTickbase;

	// reset prediction seed
	*uPredictionRandomSeed = oldSeed;

	// reset prediction entity
	*pPredictionPlayer = oldPredictionPlayer;

	// reset move
	i::GameMovement->Reset();
}

void Prediction::SaveNetvars( int iCommand, CBaseEntity* pLocal)
{
	if (!pLocal->IsAlive() || iCommand <= 0)
		return;
	auto& saved = pNetvarData[iCommand % 150];
	saved = {};
	saved.command = iCommand;
	saved.epoch = exploits::commandTimes.epoch;
	saved.localHandle = pLocal->GetRefEHandle();
	saved.spawnTime = pLocal->GetSpawnTime();
	saved.nTickbase = pLocal->GetTickBase();
	saved.absOrigin = pLocal->GetAbsOrigin();
	saved.absVelocity = pLocal->GetVecAbsVelocity();
	saved.eflags = pLocal->GetEFlags();
	saved.groundHandle = pLocal->GetGroundHandle();
	saved.surfaceFriction = pLocal->GetSurfaceFriction();
	saved.velocityModifier = pLocal->GetVelocityModifier();
	saved.nextThink = *pLocal->GetNextThink();
	saved.buttons = *pLocal->GetButtons();
	saved.buttonLast = pLocal->GetButtonLast();
	saved.buttonPressed = pLocal->GetButtonPressed();
	saved.buttonReleased = pLocal->GetButtonReleased();
	saved.nextAttack = pLocal->GetNextAttack();

	pNetvarData[ iCommand % 150 ].iFlags = pLocal->GetFlags( );
	pNetvarData[ iCommand % 150 ].flDuckAmount = pLocal->GetDuckAmount( );
	pNetvarData[ iCommand % 150 ].flDuckSpeed = pLocal->GetDuckSpeed( );
	pNetvarData[ iCommand % 150 ].vecOrigin = pLocal->GetVecOrigin( );
	pNetvarData[ iCommand % 150 ].vecVelocity = pLocal->GetVelocity( );
	pNetvarData[ iCommand % 150 ].vecBaseVelocity = pLocal->GetVecBaseVelocity( );
	pNetvarData[ iCommand % 150 ].flFallVelocity = pLocal->GetFallVelocity( );
	pNetvarData[ iCommand % 150 ].vecViewOffset = pLocal->GetViewOffset( );
	pNetvarData[ iCommand % 150 ].vecAimPunchAngle = pLocal->GetAimPunch( );
	pNetvarData[ iCommand % 150 ].vecAimPunchAngleVel = pLocal->GetAimPunchVelocity( );
	pNetvarData[ iCommand % 150 ].vecViewPunchAngle = pLocal->GetViewPunch( );

	CBaseCombatWeapon* pWeapon = pLocal->GetWeapon( );
	if ( !pWeapon )
		return;
	saved.weaponHandle = pWeapon->GetRefEHandle();
	saved.nextPrimaryAttack = pWeapon->GetNextPrimaryAttack();
	saved.nextSecondaryAttack = pWeapon->GetNextSecondaryAttack();

	pNetvarData[ iCommand % 150 ].flRecoilIndex = static_cast< CWeaponCSBase* >(pWeapon)->GetRecoilIndex( );
	pNetvarData[ iCommand % 150 ].flAccuracyPenalty = static_cast< CWeaponCSBase* >(pWeapon)->GetAccuracyPenalty( );
}

void Prediction::RestoreNetvars(int iCommand, CBaseEntity* pLocal)
{
	if (!pLocal->IsAlive() || iCommand <= 0)
		return;
	const auto& saved = pNetvarData[iCommand % 150];
	if (saved.command != iCommand || saved.epoch != exploits::commandTimes.epoch
		|| saved.localHandle != pLocal->GetRefEHandle() || saved.spawnTime != pLocal->GetSpawnTime()) return;
	pLocal->GetTickBase() = saved.nTickbase;
	pLocal->SetAbsOrigin(saved.absOrigin);
	pLocal->GetVecAbsVelocity() = saved.absVelocity;
	pLocal->GetGroundHandle() = saved.groundHandle;
	pLocal->GetSurfaceFriction() = saved.surfaceFriction;
	pLocal->GetVelocityModifier() = saved.velocityModifier;
	*pLocal->GetNextThink() = saved.nextThink;
	*pLocal->GetButtons() = saved.buttons;
	pLocal->GetButtonLast() = saved.buttonLast;
	pLocal->GetButtonPressed() = saved.buttonPressed;
	pLocal->GetButtonReleased() = saved.buttonReleased;
	pLocal->GetNextAttack() = saved.nextAttack;

	pLocal->GetFlags( ) = pNetvarData[ iCommand % 150 ].iFlags;
	pLocal->GetDuckAmount( ) = pNetvarData[ iCommand % 150 ].flDuckAmount;
	pLocal->GetDuckSpeed( ) = pNetvarData[ iCommand % 150 ].flDuckSpeed;
	pLocal->GetVecOrigin( ) = pNetvarData[ iCommand % 150 ].vecOrigin;
	pLocal->GetVelocity( ) = pNetvarData[ iCommand % 150 ].vecVelocity;
	pLocal->GetVecBaseVelocity( ) = pNetvarData[ iCommand % 150 ].vecBaseVelocity;
	pLocal->GetFallVelocity( ) = pNetvarData[ iCommand % 150 ].flFallVelocity;
	pLocal->GetViewOffset( ) = pNetvarData[ iCommand % 150 ].vecViewOffset;
	pLocal->GetAimPunch( ) = pNetvarData[ iCommand % 150 ].vecAimPunchAngle;
	pLocal->GetAimPunchVelocity( ) = pNetvarData[ iCommand % 150 ].vecAimPunchAngleVel;
	pLocal->GetViewPunch( ) = pNetvarData[ iCommand % 150 ].vecViewPunchAngle;
	pLocal->GetEFlags() = saved.eflags;

	CBaseCombatWeapon* pWeapon = pLocal->GetWeapon( );
	if (!pWeapon || pWeapon->GetRefEHandle() != saved.weaponHandle)
		return;
	pWeapon->GetNextPrimaryAttack() = saved.nextPrimaryAttack;
	pWeapon->GetNextSecondaryAttack() = saved.nextSecondaryAttack;

	static_cast< CWeaponCSBase* >( pWeapon )->GetRecoilIndex( ) = pNetvarData[ iCommand % 150 ].flRecoilIndex;
	static_cast< CWeaponCSBase* >( pWeapon )->GetAccuracyPenalty( ) = pNetvarData[ iCommand % 150 ].flAccuracyPenalty;
}

void Prediction::SaveViewmodelData(CBaseEntity* pLocal)
{
	if (!pLocal)
		return;

	CBaseViewModel* const hViewmodel = ( CBaseViewModel* )i::EntityList->GetClientEntityFromHandle( pLocal->GetViewModel( ) );

	if ( !hViewmodel )
		return;
	viewmodelHandle = pLocal->GetViewModel();
	viewmodelEpoch = exploits::commandTimes.epoch;

	iAnimationParity = hViewmodel->iAnimationParity( );
	iSequence = hViewmodel->iSequence( );
}

void Prediction::AdjustViewmodelData(CBaseEntity* pLocal)
{
	if (!pLocal)
		return;

	CBaseViewModel* const hViewmodel = ( CBaseViewModel* )i::EntityList->GetClientEntityFromHandle( pLocal->GetViewModel( ) );

	if (!hViewmodel || viewmodelHandle != pLocal->GetViewModel() || viewmodelEpoch != exploits::commandTimes.epoch)
		return;

	if ( iSequence != hViewmodel->iSequence( ) || iAnimationParity != hViewmodel->iAnimationParity( ) )
		return;

	// Sequence/parity alone do not identify the authoritative animation time.
	// Do not overwrite cycle/animtime from an unrelated predicted command.
}
