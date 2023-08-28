#include "Vizualizer.h"
using namespace cfg::vizualizer;

void vizualizer::DrawInformation() {

	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();
	if (!pLocal || !pLocal->IsAlive())
		return;

	static int vecScreenSize[2];
	i::EngineClient->GetScreenSize(vecScreenSize[0], vecScreenSize[1]);

	for (size_t i = 0; i < BAND_NUM; i++)
	{
		static int iPreviousSize[BAND_NUM]{};
		int iCurrentSize = vecScreenSize[1] - vizualizer::flCurrentLoudness[i];

		int iStep = (iPreviousSize[i] - iCurrentSize) * 0.2f;

		if (vizualizer::flCurrentLoudness[i] > 0) {

			iPreviousSize[i] -= iStep;
			if (iPreviousSize[i] - 100.f > iCurrentSize)
				pLocal->HealthShotBoost() = i::GlobalVars->flCurrentTime + 0.5f;

			//i::Surface->DrawSetColor(Color(255, 255, 255, 255));
			//i::Surface->DrawFilledRectFade(vecScreenSize[0] / 4 + (i * 30), iPreviousSize[i] -= iStep, vecScreenSize[0] / 4 + (i * 30) + 30, vecScreenSize[1], 255, 0, false);
		}
	}
}

// Callback function to process audio data CHATGPT
void CALLBACK WaveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {

	if (uMsg == WIM_DATA) {
		WAVEHDR* pWaveHdr = (WAVEHDR*)dwParam1;
		if (pWaveHdr) {

			float* audioBuffer = (float*)pWaveHdr->lpData;
			unsigned int numSamples = pWaveHdr->dwBufferLength / sizeof(float);

			// Apply the filter to each sample and calculate loudness
			for (size_t j = 0; j < BAND_NUM; j++)
			{
				float sum = 0.f;
				for (unsigned int i = 0; i < numSamples; i++) {

					float filteredSample = vizualizer::ApplyFilter(std::vector<float>(audioBuffer, audioBuffer + numSamples), i, j);
					sum += fabsf(filteredSample); // Calculate the absolute value of the sample
				}

				float loudness = sum / numSamples; // Calculate average loudness
				vizualizer::flCurrentLoudness[j] = loudness / flDivider;
			}

			// Re-queue the buffer
			waveInAddBuffer(hwi, pWaveHdr, sizeof(WAVEHDR));
		}
	}
}

float vizualizer::ApplyFilter(const std::vector<float>& vecInput, size_t nCurrent, int iBand) {

	float flOutput = 0.0f;
	for (size_t i = 0; i < vecFilter[iBand].size(); ++i) {

		int iIndex = nCurrent - i;

		if (iIndex >= 0) 
			flOutput += vecFilter[iBand][i] * vecInput[iIndex];
	}
	return flOutput;
}

void vizualizer::Setup() {

	float startValue = 25.f;
	float endValue = 150.f;
	float stepSize = (endValue - startValue) / BAND_NUM; // Dividing the range into 8 parts

	for (int i = 0; i < BAND_NUM; i++) {

		float lowerBound = startValue + i * stepSize;
		float upperBound = startValue + (i + 1) * stepSize;

		for (float value = lowerBound; value <= upperBound; value++) {
			vizualizer::vecFilter[i].push_back(value);
		}
	}

	hWaveIn;
	WAVEFORMATEX wfx = {};
	wfx.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
	wfx.nChannels = 1;
	wfx.nSamplesPerSec = 48000;
	wfx.wBitsPerSample = sizeof(float) * 8;
	wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

	if (waveInOpen(&hWaveIn, WAVE_MAPPER, &wfx, (DWORD_PTR)WaveInProc, 0, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
		// Handle error
		std::cout << "Handle error\n";
	}

	// Prepare audio buffer
	const unsigned int bufferSize = 512; // Adjust as needed
	float buffer[bufferSize];
	waveHeader = {};
	waveHeader.lpData = reinterpret_cast<LPSTR>(&buffer);
	waveHeader.dwBufferLength = bufferSize * sizeof(float);
	waveInPrepareHeader(hWaveIn, &waveHeader, sizeof(WAVEHDR));
	waveInAddBuffer(hWaveIn, &waveHeader, sizeof(WAVEHDR));

	// Start audio input
	waveInStart(hWaveIn);

	while (!cfg::bDoUnload)
		Sleep(2);

	Destroy();
}

void vizualizer::Destroy() {

	// Stop and clean up
	waveInStop(hWaveIn);
	waveInUnprepareHeader(hWaveIn, &waveHeader, sizeof(WAVEHDR));
	waveInClose(hWaveIn);
}