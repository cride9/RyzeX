#pragma once
#include "../../../SDK/Entity.h"
#include "../../../globals.h"

struct animationData_t {

	float flPoseParameters[24];
	float flGoalFeetYaw;
	float flVelocityModifier;

	int lastUpdateTime[15];

	CAnimationLayer AnimationLayer[15];
	matrix3x4_t Matrix[128];
	Vector vecViewAngle;
};

class localanimation {

public:

	animationData_t localdata{};

	bool bdoneBuildingLayers = false;
	bool bdoneBuildingPoses = false;

	bool storeLayer = false;
	bool restoreLayer = false;

	bool update = false;

	void FixVelocityModifer();
	void UpdateLocal();

	void SetSequence(CAnimationLayer*, int);
	void SetCycle(CAnimationLayer*, int);
	void SetOrder(CAnimationLayer*, int);
	void SetWeight(CAnimationLayer*, float);
	void AnimlayerFix(CUserCmd*, CAnimState*);
	void SetLayerSequence(CAnimationLayer*, int);
	
private:

	float flLastLayerFix = 0.f;
};
inline localanimation localanim;

struct AnimationRecord_t
{
	int m_nFlags = 0;
	int m_nButtons = 0;
	int m_nMoveType = 0;

	bool m_bIsShooting = false;

	Vector m_angRealAngles = Vector(0, 0, 0);
	Vector m_angFakeAngles = Vector(0, 0, 0);
	Vector m_angAimPunch = Vector(0, 0, 0);

	float m_flDuckAmount = 0.0f;
	float m_flDuckSpeed = 0.0f;

	Vector m_vecOrigin = Vector(0, 0, 0);
	Vector m_vecVelocity = Vector(0, 0, 0);
};

class C_LocalAnimations
{
public:
	virtual void OnCreateMove();
	virtual void BeforePrediction();
	virtual void StoreAnimationRecord();
	virtual void ModifyEyePosition(Vector& vecInputEyePos, matrix3x4_t* aMatrix);
	virtual void SetupPlayerBones(matrix3x4_t* aMatrix, int nMask);
	virtual void InterpolateMatricies();
	virtual void DoAnimationEvent(int nButtons, bool bIsFakeAnimations = false);
	virtual void SimulateStrafe(int nButtons);
	virtual void UpdateDesyncAnimations();
	virtual void TransformateMatricies();
	virtual void CleanSnapshots();
	virtual bool CopyCachedMatrix(matrix3x4_t* aInMatrix, int nBoneCount);
	virtual void SetupShootPosition();
	virtual void CopyPlayerAnimationData(bool bFake);

	virtual float GetYawDelta()
	{
		return m_LocalData.m_flYawDelta;
	}
	virtual Vector GetShootPosition()
	{
		return m_LocalData.m_vecShootPosition;
	}
	virtual void ResetData();
	virtual std::array < matrix3x4_t, MAXSTUDIOBONES > GetDesyncMatrix()
	{
		return m_LocalData.m_Fake.m_Matrix;
	}
private:

	struct
	{
		Vector m_vecAbsOrigin = Vector(0, 0, 0);
		int m_nFlags = 0;
		int m_nSimulationTicks = 0;

		float m_flSpawnTime = 0.0f;
		float m_flYawDelta = 0.0f;

		std::array < AnimationRecord_t, 150 > m_AnimRecords;

		Vector m_vecShootPosition = Vector(0, 0, 0);
		struct
		{
			int m_nMoveType = 0;
			int m_nFlags = 0;

			std::array < CAnimationLayer, ANIMATION_LAYER_COUNT > m_Layers = { };
			std::array < CAnimationLayer, ANIMATION_LAYER_COUNT > m_CleanLayers = { };
			std::array < float, MAXSTUDIOPOSEPARAM > m_PoseParameters = { };

			Vector m_vecMatrixOrigin = Vector(0, 0, 0);
			std::array < matrix3x4_t, MAXSTUDIOBONES > m_Matrix = { };

			CAnimState m_AnimationState;
		} m_Fake;

		struct
		{
			std::array < matrix3x4_t, MAXSTUDIOBONES > m_Matrix = { };
			std::array < CAnimationLayer, ANIMATION_LAYER_COUNT > m_Layers = { };
			std::array < float, MAXSTUDIOPOSEPARAM > m_PoseParameters = { };
			CAnimState m_AnimationState;
		} m_Shoot;

		struct
		{
			int m_nMoveType = 0;
			int m_nFlags = 0;

			std::array < CAnimationLayer, ANIMATION_LAYER_COUNT > m_Layers = { };
			std::array < float, MAXSTUDIOPOSEPARAM > m_PoseParameters = { };

			Vector m_vecMatrixOrigin = Vector(0, 0, 0);
			std::array < matrix3x4_t, MAXSTUDIOBONES > m_Matrix = { };
		} m_Real;
	} m_LocalData;
};
inline C_LocalAnimations* g_LocalAnimations = new C_LocalAnimations();