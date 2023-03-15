#pragma once
#include <deque>
#include "../../../SDK/Entity.h"
#include "../../../globals.h"

struct SequenceObject_t
{
	SequenceObject_t( int iInReliableState, int iOutReliableState, int iSequenceNr, float flCurrentTime )
		: iInReliableState( iInReliableState ), iOutReliableState( iOutReliableState ), iSequenceNr( iSequenceNr ), flCurrentTime( flCurrentTime ) { }

	int iInReliableState;
	int iOutReliableState;
	int iSequenceNr;
	float flCurrentTime;
};

struct record_t {

	/* Validity check to not shoot lagcomp breaking ppl */
	bool bValid;

	/* Later usage: fix Jumpfall animationlayer */
	int iFlags;

	Vector vecOrigin;
	Vector vecAngles;
	Vector vecMins;
	Vector vecMaxs;
	Vector vecVelocity;
	Vector vecAbsAngles;

	/* For backtrack / updatetime / choke calculation */
	float flSimulationTime;
	float flOldSimulationTime;
	float flMaxSpeed;
	float flLowerBody;

	/* Getting accurate poses */
	float flPoseParameters[24];

	/* Will be replaced with uninterpolated layers (easier to make a resolver for it) */
	CAnimationLayer pLayers[13];

	/* Aimbot will shoot the generated matrix */
	matrix3x4_t pMatrix[128];

	void StoreRecord(CBaseEntity* pEnt) {

		bValid = true;

		iFlags = pEnt->GetFlags();

		vecOrigin = pEnt->GetVecOrigin();
		vecAngles = pEnt->GetEyeAngles();
		vecMins = pEnt->GetCollideable()->OBBMins();
		vecMaxs = pEnt->GetCollideable()->OBBMaxs();
		vecVelocity = pEnt->GetVelocity();
		vecAbsAngles = pEnt->GetAbsAngles();

		flSimulationTime = pEnt->GetSimulationTime();
		flOldSimulationTime = pEnt->GetOldSimulationTime();
		flMaxSpeed = pEnt->GetWeapon() ? pEnt->GetWeapon()->GetCSWpnData()->flMaxSpeed[0] : 260.f;
		flLowerBody = pEnt->GetLowerBodyYaw();

		pEnt->GetPoseParameters(flPoseParameters);
		pEnt->GetAnimationLayers(pLayers);
	}

	void ApplyRecord(CBaseEntity* pEnt) {

		pEnt->GetFlags() = iFlags;

		pEnt->GetVecOrigin() = vecOrigin;
		pEnt->GetEyeAngles() = vecAngles;
		pEnt->GetCollideable()->OBBMins() = vecMins;
		pEnt->GetCollideable()->OBBMaxs() = vecMaxs;
		pEnt->GetVelocity() = vecVelocity;
		pEnt->SetAbsAngles(vecAbsAngles);

		pEnt->GetSimulationTime() = flSimulationTime;
		pEnt->GetOldSimulationTime() = flOldSimulationTime;
		pEnt->GetLowerBodyYaw() = flLowerBody;

		pEnt->SetPoseParameters(flPoseParameters);
		pEnt->SetAnimationLayers(pLayers);
		pEnt->SetBoneCache(pMatrix);
	}
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
	/* Everything will be ran inside this */
	void FrameStageNotify(EStage curStage);
	// extrapolate players breaking lagcomp
	void ExtrapolatePlayer( CBaseEntity* m_pEntity, record_t* m_pCurrentRecord, record_t* m_pPrevious ) const;
	
	/* Lerp Time */
	float LerpTime();

	void UpdateIncomingSequences( INetChannel* pNetChannel );
	void ClearIncomingSequences( );
	void AddLatencyToNetChannel( INetChannel* pNetChannel, float flLatency );

	void ResolverHandler(IGameEvent*);
	void ResolverLogic();
	float Resolver(CBaseEntity*, int);

	/* Every entity data will be placed into this deque */
	std::deque<record_t> deqRecords[65];
	int missedShots[65];
	bool didHurt = false;
	Vector bulletImpact = Vector(0, 0, 0);

private:
	void RebuildWalkToRunTransition( CBaseEntity* pEntity, record_t* pRecord );
	/* Will be called with the function upper */
	void UpdateAnimation(CBaseEntity* pEnt);

	/* Backup animationlayers before interpolation */
	void GetAnimationLayers(CBaseEntity*);

	/* Fix animation time (velocity) and origin */
	void FixAbsoluteAngVec(CBaseEntity*, record_t*, record_t*);

	// Values
	std::deque<SequenceObject_t> vecSequences = { };
	/* our real incoming sequences count */
	int nRealIncomingSequence = 0;
	/* count of incoming sequences what we can spike */
	int nLastIncomingSequence = 0;
};
inline Lagcompensation lagcomp;