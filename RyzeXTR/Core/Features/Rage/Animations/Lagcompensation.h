#pragma once
#include <deque>
#include "../../../SDK/Entity.h"
#include "../../../globals.h"
#include "../../../SDK/math.h"

#pragma region lagcompensation_definitions
#define LAG_COMPENSATION_TELEPORTED_DISTANCE_SQR ( 64.0f * 64.0f )
#define LAG_COMPENSATION_EPS_SQR ( 0.1f * 0.1f )
#define LAG_COMPENSATION_ERROR_EPS_SQR ( 4.0f * 4.0f )
#pragma endregion

struct SequenceObject_t
{
	SequenceObject_t( int iInReliableState, int iOutReliableState, int iSequenceNr, float flCurrentTime )
		: iInReliableState( iInReliableState ), iOutReliableState( iOutReliableState ), iSequenceNr( iSequenceNr ), flCurrentTime( flCurrentTime ) { }

	int iInReliableState;
	int iOutReliableState;
	int iSequenceNr;
	float flCurrentTime;
};

class CSimulationData
{
public:
	CSimulationData( ) : pEntity( nullptr ), bOnGround( false )
	{
	}

	~CSimulationData( )
	{
	}

	CBaseEntity* pEntity;

	Vector vecOrigin;
	Vector vecVelocity;

	bool bOnGround;

	bool bDataFilled = false;
};

class Lagcompensation {

public:

	struct LagRecord_t {

		struct LayerData_t
		{
			int nSequence;
			float flPlaybackRate;
			float flCycle;
			float flWeight;
			float flWeightWithAirSmooth;
			float flFeetWeight;

			float flMovementSide;
			Vector angMoveYaw;
			Vector vecDirection;
		};

		LagRecord_t( ) = default;
		LagRecord_t( CBaseEntity* pEntity );

		void Apply( CBaseEntity* pEntity, bool Backup );
		void Restore( CBaseEntity* pEntity );
		void Apply( CBaseEntity* pEntity );

		CBaseEntity* pEntity{};
		matrix3x4_t pMatrix[ 128 ];
		matrix3x4_t pLeftMatrix[128];
		matrix3x4_t pRightMatrix[128];
		matrix3x4_t pCenterMatrix[128];
		bool bSetupMatrixes = false;

		bool bBreakingLagcompensation{};
		bool bFakewalking{};
		bool bValid{};
		bool bDormant{};
		bool bBackwards{};
		bool bSideways{};
		bool bForwards{};
		bool bDidShot{};

		Vector vecVelocity{};
		Vector vecAbsVelocity{};
		Vector vecOrigin{};
		Vector vecAbsOrigin{};
		Vector vecMins{};
		Vector vecMaxs{};

		Vector vecLastReliableAngle{};
		Vector vecEyeAngles{};
		Vector vecAbsAngles{};

		float pResolverPlaybackrate[ 3 ];
		CAnimationLayer pResolverLayers[ 3 ][ 13 ];
		CAnimationLayer pResolverLayers2[ 3 ][ 13 ];

		LayerData_t LayerData[ 4 ];

		CAnimationLayer pLayers[ 13 ];

		float flPoses[ 24 ];

		float flServerTick{};
		float flAnimationTime{};
		float flSimulationTime{};
		float flOldSimulationTime{};
		float flInterpTime{};
		float flDuck{};
		float flLowerBodyYawTarget{};
		float flLastShotTime{};
		float flSpawnTime{};
		float flDeltaAngle{};

		int iCachedCount{};
		int iWritableBones{};

		int fFlags{};
		int iEFlags{};
		int iEffects{};
		int iEntIndex{};
		int iChoked{};
	};

	enum EResolverMode
	{
		LEGIT = 0,
		DESYNC,
		OPPOSITE,
		SWAY,
		FAKE,
		OVERRIDE,
		ONSHOT,
		JITTER,
		ANIMATION
	};


	struct AnimationInfo_t
	{
		CBaseEntity* pEntity;
		int iLastValid;
		int iFirstValid = 32;
		float flSpawntime;
		std::deque<Lagcompensation::LagRecord_t> pRecord;

		// resolve data.
		int iShots;
		int iMissedShots;
		float flTimeSinceLegit;
		float flTimeSinceNoDesync;
		float flTimeSinceBreakingLBY;
		float flTimeSinceBodySwaying;
		float flTimeSinceBodySwayLeft;
		float flTimeSinceBodySwayRight;
		int iWalkToRunTransitionState;
		float flWalkToRunTransition;
		int iDesyncSide;
		Lagcompensation::EResolverMode iAntiAimType;
	};

	/* Everything will be ran inside this */
	void FrameStageNotify();
	// get animation info
	AnimationInfo_t& GetLog( const int iEntIndex );
	// check if player is breaking lagcomp
	bool IsBreakingLagcompensation( Lagcompensation::LagRecord_t* pLagRecord );
	// fix tickcount so we can backtrack
	int FixTickCount( const float& flSimulationTime );
	
	void UpdateIncomingSequences( INetChannel* pNetChannel );
	void ClearIncomingSequences( );
	void AddLatencyToNetChannel( INetChannel* pNetChannel, float flLatency );

	void RemoveInterpolation();

	void SetupPlayerBones(CBaseEntity* pPlayer, Lagcompensation::LagRecord_t* pRecord,matrix3x4_t* Matrix, int nFlags);

	// get client interp amount
	static float GetClientInterpAmount( );
private:
	// filter records after updating them
	void FilterRecords( );
	// extrapolate players breaking lagcomp
	void ExtrapolatePlayer( CBaseEntity* m_pEntity, Lagcompensation::LagRecord_t* m_pCurrentRecord, Lagcompensation::LagRecord_t* m_pPrevious ) const;
	// check if record is valid
	bool IsValidRecord( float m_flSimulationTime, float m_flRange = 0.2f );
	// fuck interpolation
	void SetInterpolationFlags(CBaseEntity* pEnemy);

	// Values
	/* animation info */
	AnimationInfo_t pPlayerLogs[ 65 ];
	/* stored sequences */
	std::deque<SequenceObject_t> vecSequences = { };
	/* our real incoming sequences count */
	int nRealIncomingSequence = 0;
	/* count of incoming sequences what we can spike */
	int nLastIncomingSequence = 0;
	
	int nInvalidateFlags{};
};
inline Lagcompensation lagcomp;