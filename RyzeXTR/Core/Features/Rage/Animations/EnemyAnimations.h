#pragma once
#include "../../../SDK/Menu/config.h"
#include "../../../SDK/math.h"
#include "Lagcompensation.h"

class Animations {

public:
	/* enemy anims */

	int iLastGuessedYaw{};
	float flGuessedYaw{};
	int nInvalidateFlags{};
	Lagcompensation::AnimationInfo_t pPlayerLogs[ 65 ];

	struct AnimationData_t
	{
		float flPrimaryCycle{};
		float flMovePlaybackRate{};
		float flFeetWeight{};
		float flVelocityLengthXY{};
		int iMoveState{};
	};

	AnimationData_t pAnimationData[ 65 ];
	Lagcompensation::AnimationInfo_t pLogs[ 65 ];

	bool NewDataRecievedFromServer( CBaseEntity* pPlayer );
	void FixAnimatingInSameFrame( CBaseEntity* pEntity );
	void UpdateClientSideAnimations( CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord );
	void Interpolation( CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious, float flLerp );
	void RebuildWalkToRunTransition( CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord );
	void FixVelocity( CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious = nullptr, int i = 0, float flLerp = 0.f );
	float GetMaxSpeed( CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious, int i );
	void FixJumpFallAnimation( CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord = nullptr, Lagcompensation::LagRecord_t* pPrevious = nullptr, float flSimulationTime = 0.f );
	void SkipToCalcAbsoluteVelocity( CBaseEntity* pEntity );
	void SkipToCalcAbsolutePosition( CBaseEntity* pEntity );
	void OnSave( CBaseEntity* pEntity );
	void FixLowerbody( CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious, int i );
	void UpdateEnemyAnimations( CBaseEntity* m_pEntity, Lagcompensation::LagRecord_t* pRecord );

	/* enemy anims */

	/* resolver */

	void ResolverHandler( IGameEvent* );
	void ResolverLogic( );
	void SetGoalFeetYaw( CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord, Lagcompensation::LagRecord_t* pPrevious, float flServerVelocityXY, float flPlaybackrate = 0.f, int brutePhase = 0 );
	int missedShots[ 65 ];
	bool didHurt = false;
	Vector bulletImpact = Vector( 0, 0, 0 );

	/* resolver */

private:
	void FakePitchResolver( CBaseEntity* m_pEntity, Lagcompensation::LagRecord_t* m_pRecord );
	std::pair<CAnimationLayer*, float*> BuildSideLayerAndPose(CBaseEntity* pEnt, float sideAngle);
};
inline Animations anims;