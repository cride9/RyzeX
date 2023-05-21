#pragma once
#include "../../../SDK/Menu/config.h"
#include "../../../SDK/math.h"
#include "Lagcompensation.h"

#define ANIM_TRANSITION_WALK_TO_RUN 0
#define ANIM_TRANSITION_RUN_TO_WALK 1
#define CSGO_ANIM_WALK_TO_RUN_TRANSITION_SPEED 2.0f
#define CSGO_ANIM_ONGROUND_FUZZY_APPROACH 8.0f
#define CSGO_ANIM_ONGROUND_FUZZY_APPROACH_CROUCH 16.0f
#define CSGO_ANIM_LADDER_CLIMB_COVERAGE 100.0f
#define CSGO_ANIM_RUN_ANIM_PLAYBACK_MULTIPLIER 0.85f

#define CS_PLAYER_SPEED_RUN 260.0f
#define CS_PLAYER_SPEED_VIP 227.0f
#define CS_PLAYER_SPEED_SHIELD 160.0f
#define CS_PLAYER_SPEED_HAS_HOSTAGE 200.0f
#define CS_PLAYER_SPEED_STOPPED 1.0f
#define CS_PLAYER_SPEED_OBSERVER 900.0f

#define CS_PLAYER_SPEED_DUCK_MODIFIER 0.34f
#define CS_PLAYER_SPEED_WALK_MODIFIER 0.52f
#define CS_PLAYER_SPEED_CLIMB_MODIFIER 0.34f
#define CS_PLAYER_HEAVYARMOR_FLINCH_MODIFIER 0.5f

#define CS_PLAYER_DUCK_SPEED_IDEAL 8.0f

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
	float GetVelocityLengthXY( CBaseEntity* );
	void UpdateEnemyAnimations( CBaseEntity* m_pEntity, Lagcompensation::LagRecord_t* pRecord );
	void RebuiltLayer6(CBaseEntity* pEntity, Lagcompensation::LagRecord_t::LayerData_t* pLayer);
	void UpdateSafePointMatrixes(CBaseEntity* pEntity, Lagcompensation::LagRecord_t* pRecord);

	//https://github.com/perilouswithadollarsign/cstrike15_src/blob/29e4c1fda9698d5cebcdaf1a0de4b829fa149bf8/game/shared/cstrike15/csgo_playeranimstate.cpp#L1978
	void UpdateAnimLayer(CBaseEntity* pEntity, Lagcompensation::LagRecord_t::LayerData_t* pLayer, int nSequence, float flPlaybackRate, float flWeight, float flCycle);
	// https://github.com/perilouswithadollarsign/cstrike15_src/blob/29e4c1fda9698d5cebcdaf1a0de4b829fa149bf8/game/shared/cstrike15/csgo_playeranimstate.cpp#L1351
	void LayerSetUpMovementRebuild(CBaseEntity* pEntity, Lagcompensation::LagRecord_t::LayerData_t* pLayer);
	/* enemy anims */
	void RebuildEnemyAnimations(CBaseEntity* pEntity, Lagcompensation::AnimationInfo_t* pLog);

	/* resolver */
	void ResolverHandler( IGameEvent* );
	float GetLocalCycleIncrement( CBaseEntity*, float );
	void UpdateOnFeetYaw( CBaseEntity*, Lagcompensation::LagRecord_t* );
	void SetGoalFeetYaw( CBaseEntity*, Lagcompensation::LagRecord_t*, Lagcompensation::LagRecord_t*, float, float );
	void ResolverLogic( );
	int missedShots[ 65 ];
	bool didHurt = false, didFire = false, didImpact = false, didDie = false;
	Vector bulletImpact = Vector( 0, 0, 0 );

	/* resolver */
	std::pair<CAnimationLayer*, float*> BuildSideLayerAndPose(CBaseEntity*, float);
	bool CopyCachedMatrix(CBaseEntity* pEnt, matrix3x4_t* pMatrix, int nBoneCount);
	void TransformateMatrix(CBaseEntity* pPlayer);
	void InterpolateMatricies();

private:
	void FakePitchResolver( CBaseEntity*, Lagcompensation::LagRecord_t* );
};
inline Animations anims;