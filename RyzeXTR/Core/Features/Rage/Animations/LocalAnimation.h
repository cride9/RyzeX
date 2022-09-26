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