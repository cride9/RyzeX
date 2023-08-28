#pragma once
/* Used: flFrom, flTo, flDivider */
#include "../../SDK/Menu/config.h"

/* Used: ISurface */
#include "../../Interface/interfaces.h"

/* Used: HWAVEIN, WAVEFORMATEX */
#include <mmeapi.h>

/* Used: WAVE_FORMAT_IEEE_FLOAT */
#include <mmreg.h>

/* Used: CBaseEntity* */
#include "../../SDK/Entity.h"

#define BAND_NUM 1

namespace vizualizer {

	inline float flCurrentLoudness[BAND_NUM]{};

	inline std::vector<float> vecFilter[BAND_NUM]{};
	inline WAVEHDR waveHeader{};
	inline HWAVEIN hWaveIn{};

	float ApplyFilter(const std::vector<float>& vecInput, size_t nCurrent, int iBand);
	void Setup();
	void DrawInformation();
	void Destroy();
}