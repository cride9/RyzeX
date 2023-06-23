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

enum EFixedVelocity
{
	Unresolved = 0,
	MovementLayer,
	AliveLoopLayer,
	LeanLayer
};

enum EPlayerActivity
{
	NoActivity = 0,
	Jump = 1,
	Land = 2
};

struct SequenceObject_t
{
	SequenceObject_t(int iInReliableState, int iOutReliableState, int iSequenceNr, float flCurrentTime)
		: iInReliableState(iInReliableState), iOutReliableState(iOutReliableState), iSequenceNr(iSequenceNr), flCurrentTime(flCurrentTime) { }

	int iInReliableState;
	int iOutReliableState;
	int iSequenceNr;
	float flCurrentTime;
};

class CSimulationData
{
public:
	CSimulationData() : pEntity(nullptr), bOnGround(false)
	{
	}

	~CSimulationData()
	{
	}

	CBaseEntity* pEntity;

	Vector vecOrigin;
	Vector vecVelocity;

	bool bOnGround;

	bool bDataFilled = false;
};

enum EMatrixType : int {

	VISUAL,
	RESOLVE,
	LEFT,
	RIGHT,
	CENTER,
	MAX
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

		LagRecord_t() = default;
		LagRecord_t(CBaseEntity* pEntity);

		void Apply(CBaseEntity* pEntity, bool Backup);
		void Restore(CBaseEntity* pEntity);
		void Apply(CBaseEntity* pEntity);
		void ApplyMatrix(CBaseEntity* pEntity, EMatrixType iType);

		CBaseEntity* pEntity{};

		// 0 - Visual, 1 - Resolve, 2 - Left, 3 - Right, 4 - Center
		matrix3x4_t pMatricies[MAX][128];
		CAnimationLayer pSideLayers[MAX][13];
		LayerData_t LayerData[MAX];

		bool bBreakingLagcompensation{};
		bool bFakewalking{};
		bool bValid = true;
		bool bDormant{};
		bool bBackwards{};
		bool bSideways{};
		bool bForwards{};
		bool bDidShot{};
		bool bRestoreData{};
		bool bFirstAfterDormant{};
		bool bSafeResolve = false;

		Vector vecVelocity{};
		Vector vecAbsVelocity{};
		Vector vecOrigin{};
		Vector vecAbsOrigin{};
		Vector vecMins{};
		Vector vecMaxs{};

		Vector vecLastReliableAngle{};
		Vector vecEyeAngles{};
		Vector vecAbsAngles{};

		float pResolverPlaybackrate[3];

		CAnimationLayer pLayers[13];
		float flPoses[24];

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
		float flEyeYaw{};
		float flMaxSpeed{};
		float flAnimationVelocity{};
		float flDurationInAir{};
		float flActivityPlayback{};
		float flThirdPersonRecoil{};
		float flResolverAngle{};

		int iCachedCount{};
		int iWritableBones{};
		int nVelocityMode{};
		int iActivityTick{};
		int iActivityType{};

		int iFlags{};
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

		bool bLeftDormancy{};

		bool bSafeRecord;
		int iLastUpdateTick;
		int iLastValid;
		int iFirstValid = 32;
		int iShots;
		int iMissedShots; 
		int iWalkToRunTransitionState;
		int iDesyncSide;

		float flTimeSinceLegit;
		float flTimeSinceNoDesync;
		float flTimeSinceBreakingLBY;
		float flTimeSinceBodySwaying;
		float flTimeSinceBodySwayLeft;
		float flTimeSinceBodySwayRight;
		float flSpawntime;
		float flWalkToRunTransition;
		float flExploitTime;

		Lagcompensation::EResolverMode iAntiAimType;
		std::deque<Lagcompensation::LagRecord_t> pRecord;
		std::array<matrix3x4_t, 128> pCachedMatrix{matrix3x4_t()};

		void ClearData() {

			pEntity = nullptr;
			iLastUpdateTick = 0;
			pCachedMatrix = std::array<matrix3x4_t, 128>();
			pRecord.clear();
		}
	};

	/* Everything will be ran inside this */
	void FrameStageNotify() noexcept;

	/* Those functions will run in createmove */
	void StartLagcompensation(CBaseEntity*);
	void FinishLagcompensation(CBaseEntity*);

	// get animation info
	AnimationInfo_t& GetLog(const int iEntIndex);
	// check if player is breaking lagcomp
	bool IsBreakingLagcompensation(Lagcompensation::LagRecord_t* pLagRecord);
	// fix tickcount so we can backtrack
	int FixTickCount(const float& flSimulationTime);

	void UpdateIncomingSequences(INetChannel* pNetChannel);
	void ClearIncomingSequences();
	void AddLatencyToNetChannel(INetChannel* pNetChannel, float flLatency);

	// get client interp amount
	static float GetClientInterpAmount();

	// fuck interpolation
	void SetInterpolationFlags();

	// check if record is valid
	static bool IsValidRecord(float m_flSimulationTime, float m_flRange = 0.199f);

	// extrapolate players breaking lagcomp
	void ExtrapolatePlayer(CBaseEntity* m_pEntity, Lagcompensation::LagRecord_t* m_pCurrentRecord, Lagcompensation::LagRecord_t* m_pPrevious) const;

	std::array<std::pair<Lagcompensation::LagRecord_t, bool>, 65> arrBackupData;

private:
	// filter records after updating them
	void FilterRecords();


	// Values
	/* animation info */
	AnimationInfo_t pPlayerLogs[65];
	/* stored sequences */
	std::deque<SequenceObject_t> vecSequences = { };
	/* our real incoming sequences count */
	int nRealIncomingSequence = 0;
	/* count of incoming sequences what we can spike */
	int nLastIncomingSequence = 0;

	int nInvalidateFlags{};

};
inline Lagcompensation lagcomp;