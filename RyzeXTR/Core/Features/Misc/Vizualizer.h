#pragma once
/* Used: flFrom, flTo, flDivider */
#include "../../SDK/Menu/config.h"

/* Used: ISurface */
#include "../../Interface/interfaces.h"

/* Used: CBaseEntity */
#include "../../SDK/Entity.h"

namespace vizualizer {

	void Initialize();
	void Shutdown();
	void Draw(CBaseEntity* pLocal);
}
