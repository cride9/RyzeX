#pragma once
#include "../../SDK/CUserCmd.h"
#include "../../globals.h"
#include "../../SDK/Menu/config.h"

enum EPITCH : int {

	OFF,
	UP,
	ZERO,
	DOWN
};

enum EYAW : int {

	FORWARD,
	BACKWARD
};

enum EDESYNC : int {

	DISABLED,
	STATIC,
	EXTENDED,
	JITTER,
	FLICK,
	AUTOMATIC,
	ADAPTIVE,
	EXTENDED_HOLD,
	EXTENDED_PULSE,
	EXTENDED_COVER,
	EXTENDED_HYBRID
};

enum AATYPE : int {

	STANDING,
	MOVING,
	INAIR
};

class IGameEvent;
namespace adaptiveaa { struct Statistics; }

namespace antiaim {
	constexpr bool IsAdaptiveMode(int mode) { return mode >= ADAPTIVE && mode <= EXTENDED_HYBRID; }
	const adaptiveaa::Statistics& GetAdaptiveStatistics();
	bool FinalizeAdaptive(CUserCmd*, bool sendPacket);
	void HandleAdaptiveEvent(IGameEvent* event);
	void ResetAdaptive();
	void ObserveLowerBody(CUserCmd*);
	void ApplyAdaptivePreset(int type);
	void ApplyExtendedPreset(int type);
	void DoAdaptive(CUserCmd*, bool& sendPacket, AATYPE);
	void DoExtended(CUserCmd*, bool& sendPacket, AATYPE, int inverter);

	inline bool needMicromovement = false;
	inline bool bHideFlick = false;

	inline bool shotInvert = false;
	void AntiAim(CUserCmd*, bool&);
	
	enum LBYUpdateType : int
	{
		LBYUPDATE_None,
		LBYUPDATE_Standing,
		LBYUPDATE_Moving
	};

	bool ShouldDisableAntiaim(CUserCmd* pCmd, bool& bSendPacket);
	
	bool FreeStandingDistance(CUserCmd*, Vector&);
	bool FreeStandingThreat(Vector& angle);
    void InvertOnShoot(CUserCmd* pCmd);
    void DoRytter(CUserCmd* pCmd, int type);
	bool DesyncSide( );
	int ClosestToLocal();
	int ClosesToCrosshair();
	void AtTarget(CUserCmd*, Vector&);

	bool LBYUpdate(CBaseEntity* pLocal);

	void DoAntiaim(CUserCmd* pCmd, bool& bSendPacket, AATYPE);
	bool ManualAA(CUserCmd* pCmd, bool type);
}
