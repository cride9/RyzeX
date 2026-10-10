#pragma once
#include "RadioVisualizerMath.h"

struct ImFont;

namespace radio_visualizer {
    // Initialize/Reset/Capture belong exclusively to the existing radio worker.
    void InitializeAnalysis();
    void ResetStream();
    void Capture(std::uint32_t channel);
    void SetAnalysisEnabled(bool enabled);
    bool IsAnalysisEnabled();
    Frame GetFrame();

    // All UI and animation state belongs exclusively to the ImGui render thread.
    void RenderOverlay(ImFont* logoFont, bool menuOpen, bool radioEnabled);
    void OpenSettings();
    void DrawSettingsModal(ImFont* logoFont);
}
