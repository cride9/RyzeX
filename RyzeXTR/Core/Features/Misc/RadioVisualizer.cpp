#include "RadioVisualizer.h"
#include "../../SDK/Menu/config.h"
#include "../../../Dependecies/ImGui/imgui.h"

namespace {
    using namespace radio_visualizer;
    struct Particle { float x, y, vx, vy, life, duration, size; };
    struct Ring { float age = 0.f; bool active = false; };
    struct Animation {
        Envelope audio;
        std::array<Particle, 160> particles{};
        std::array<Ring, 6> rings{};
        int particleCount = 0;
        float spawnCarry = 0.f;
        float time = 0.f;
        std::uint32_t random = 0x925fe123;
        float Random() {
            random ^= random << 13; random ^= random >> 17; random ^= random << 5;
            return (random & 0xffffff) / 16777216.f;
        }
    };
    Animation overlay, preview;
    bool modalRequested = false;
    bool modalVisible = false;
    float demoTime = 0.f;
    std::uint64_t demoSequence = 0;

    ImU32 Blend(const Settings& s, float t, float alpha = 1.f) {
        ImVec4 color;
        color.x = s.colorA[0] + (s.colorB[0] - s.colorA[0]) * t;
        color.y = s.colorA[1] + (s.colorB[1] - s.colorA[1]) * t;
        color.z = s.colorA[2] + (s.colorB[2] - s.colorA[2]) * t;
        color.w = (s.colorA[3] + (s.colorB[3] - s.colorA[3]) * t) * alpha * s.opacity;
        return ImGui::ColorConvertFloat4ToU32(color);
    }

    void Bars(ImDrawList* draw, ImVec2 center, float radius, float size,
        const Animation& st, const Settings& s, float widthScale, float alpha, bool caps) {
        const int total = GetQuality(s.quality).points;      // 64 / 128 / 192 oszlop
        const float inner = radius * 1.12f;
        const float maxLen = radius * .85f;
        const float width = 2.f * Pi * inner / total * .5f * widthScale;
        const ImVec2 uv = ImGui::GetFontTexUvWhitePixel();
        draw->PrimReserve(total * 6, total * 4);
        const unsigned int base = draw->_VtxCurrentIdx;
        for (int i = 0; i < total; ++i) {
            const float t = (i + .5f) / total;
            const float u = std::abs(2.f * t - 1.f);              // 0 = alul (bass), 1 = fent (magas), tükrözött
            const float angle = -Pi * .5f + t * 2.f * Pi;
            const float c = u * (BandCount - 1);
            const int a = static_cast<int>(c), b = (std::min)(a + 1, BandCount - 1);
            const float v = st.audio.bands[a] + (st.audio.bands[b] - st.audio.bands[a]) * (c - a);
            const float len = size * .006f + std::pow(v, 1.3f) * maxLen;
            const ImVec2 dir(std::cos(angle), std::sin(angle));
            const ImVec2 perp(-dir.y * width * .5f, dir.x * width * .5f);
            const ImVec2 p0(center.x + dir.x * inner, center.y + dir.y * inner);
            const ImVec2 p1(center.x + dir.x * (inner + len), center.y + dir.y * (inner + len));
            const ImU32 lo = Blend(s, u, alpha * .55f), hi = Blend(s, u, alpha);
            draw->PrimWriteVtx(ImVec2(p0.x - perp.x, p0.y - perp.y), uv, lo);
            draw->PrimWriteVtx(ImVec2(p0.x + perp.x, p0.y + perp.y), uv, lo);
            draw->PrimWriteVtx(ImVec2(p1.x + perp.x, p1.y + perp.y), uv, hi);
            draw->PrimWriteVtx(ImVec2(p1.x - perp.x, p1.y - perp.y), uv, hi);
            const unsigned int o = base + i * 4;
            for (unsigned int k : { 0u, 1u, 2u, 0u, 2u, 3u })
                draw->PrimWriteIdx(static_cast<ImDrawIdx>(o + k));
        }
        if (!caps) return;
        for (int i = 0; i < total; ++i) {
            const float t = (i + .5f) / total;
            const float u = std::abs(2.f * t - 1.f);
            const float angle = -Pi * .5f + t * 2.f * Pi;
            const float pk = st.audio.peaks[static_cast<int>(u * (BandCount - 1))];
            if (pk < .06f) continue;
            const float r = inner + size * .010f + std::pow(pk, 1.3f) * maxLen;
            const ImVec2 dir(std::cos(angle), std::sin(angle));
            const ImVec2 perp(-dir.y * width * .5f, dir.x * width * .5f);
            const float th = size * .004f;
            const ImVec2 q(center.x + dir.x * r, center.y + dir.y * r);
            const ImVec2 q2(q.x + dir.x * th, q.y + dir.y * th);
            draw->AddQuadFilled(ImVec2(q.x - perp.x, q.y - perp.y), ImVec2(q.x + perp.x, q.y + perp.y),
                ImVec2(q2.x + perp.x, q2.y + perp.y), ImVec2(q2.x - perp.x, q2.y - perp.y), Blend(s, u, .9f));
        }
    }

    // Three strips give the closed ribbon an opaque core and soft transparent edges.
    // The annular mesh also handles the concave Nation ears without a convex-fill assumption.
    void Ribbon(ImDrawList* draw, const std::array<float, 384>& radii, int count,
        ImVec2 center, float radius, float width, float core, const Settings& settings, float alpha) {
        const ImVec2 uv = ImGui::GetFontTexUvWhitePixel();
        draw->PrimReserve(count * 18, count * 4);
        const unsigned int base = draw->_VtxCurrentIdx;
        for (int point = 0; point < count; ++point) {
            const float angle = point * (2.f * Pi / count);
            const float x = std::cos(angle), y = std::sin(angle);
            const ImU32 color = Blend(settings, .5f + .5f * x, alpha);
            const ImU32 clear = color & ~IM_COL32_A_MASK;
            const float offsets[] = { -width, -core, core, width };
            for (int edge = 0; edge < 4; ++edge) {
                const float r = (std::max)(1.f, radii[point] * radius + offsets[edge]);
                draw->PrimWriteVtx(ImVec2(center.x + x * r, center.y + y * r), uv, edge == 0 || edge == 3 ? clear : color);
            }
        }
        for (int point = 0; point < count; ++point) {
            const unsigned int a = base + point * 4;
            const unsigned int b = base + ((point + 1) % count) * 4;
            for (int strip = 0; strip < 3; ++strip) {
                draw->PrimWriteIdx(static_cast<ImDrawIdx>(a + strip));
                draw->PrimWriteIdx(static_cast<ImDrawIdx>(b + strip));
                draw->PrimWriteIdx(static_cast<ImDrawIdx>(a + strip + 1));
                draw->PrimWriteIdx(static_cast<ImDrawIdx>(a + strip + 1));
                draw->PrimWriteIdx(static_cast<ImDrawIdx>(b + strip));
                draw->PrimWriteIdx(static_cast<ImDrawIdx>(b + strip + 1));
            }
        }
    }

    void Spawn(Animation& state, int count, int limit, bool burst) {
        while (count-- > 0 && state.particleCount < limit) {
            const float angle = state.Random() * 2.f * Pi;
            const float speed = (burst ? .8f : .16f) + state.Random() * .55f;
            const float c = std::cos(angle), s = std::sin(angle);
            Particle& p = state.particles[state.particleCount++];
            p = { c * 1.1f, s * 1.1f, c * speed - s * .1f, s * speed + c * .1f,
                0.f, .65f + state.Random() * 1.15f, .6f + state.Random() * 1.3f };
        }
    }

    void Animate(Animation& state, const Frame& frame, float dt, const Settings& settings) {
        if (state.audio.generation != frame.generation) state = {};
        state.audio.Update(frame, dt, settings, Clock::now());
        state.time += dt;
        const auto quality = GetQuality(settings.quality);
        state.particleCount = (std::min)(state.particleCount, quality.particles);
        for (int particle = 0; particle < state.particleCount;) {
            auto& p = state.particles[particle];
            p.life += dt;
            if (p.life >= p.duration || !settings.particles) {
                p = state.particles[--state.particleCount];
                continue;
            }
            p.x += p.vx * dt; p.y += p.vy * dt;
            ++particle;
        }
        for (auto& ring : state.rings) {
            ring.age += dt;
            if (ring.age >= .8f || !settings.rings) ring.active = false;
        }
        if (settings.particles && IsFresh(frame, Clock::now()) && state.audio.bass > .05f) {
            state.spawnCarry += dt * 14.f * settings.particleDensity;
            const int spawn = static_cast<int>(state.spawnCarry);
            state.spawnCarry -= spawn;
            Spawn(state, spawn, quality.particles, false);
            if (state.audio.beat)
                Spawn(state, static_cast<int>((settings.preset ? 36.f : 22.f) * settings.particleDensity), quality.particles, true);
        }
        if (settings.rings && state.audio.beat) {
            for (auto& ring : state.rings) if (!ring.active) { ring = { 0.f, true }; break; }
        }
    }

    void Draw(ImDrawList* draw, ImVec2 topLeft, float size, ImFont* font, const Animation& state, const Settings& settings) {
        draw->PushClipRect(topLeft, ImVec2(topLeft.x + size, topLeft.y + size), true);
        const ImVec2 center(topLeft.x + size * .5f, topLeft.y + size * .5f);
        const float radius = size * .23f;
        const auto quality = GetQuality(settings.quality);
        std::array<float, 384> contour{};
        std::array<float, 384> circle{};
        circle.fill(1.f);
        for (int point = 0; point < quality.points; ++point)
            contour[point] = ContourRadius(state.audio, settings, point * 2.f * Pi / quality.points);

        for (const auto& ring : state.rings) if (ring.active) {
            const float progress = ring.age / .8f;
            Ribbon(draw, circle, quality.points, center, radius * (1.2f + progress * .8f),
                size * .009f, size * .002f, settings, (1.f - progress) * .35f * settings.ringStrength);
        }
        for (int i = 0; i < state.particleCount; ++i) {
            const auto& p = state.particles[i];
            const float fade = std::sin(Pi * p.life / p.duration);
            const ImVec2 position(center.x + p.x * radius, center.y + p.y * radius);
            const float dot = p.size * size / 320.f;
            if (settings.glow)
                draw->AddCircleFilled(position, dot * 3.f, Blend(settings, i / static_cast<float>(quality.particles), fade * .10f * settings.glowStrength), 8);
            draw->AddCircleFilled(position, dot, Blend(settings, i / static_cast<float>(quality.particles), fade * .8f), 8);
        }

        if (settings.glow) {
            if (settings.preset == 2)
                Bars(draw, center, radius, size, state, settings, 2.6f,
                    settings.glowStrength * .14f * (.6f + state.audio.bass * .4f), false);
            else
                for (int layer = quality.glowLayers; layer > 0; --layer)
                    Ribbon(draw, contour, quality.points, center, radius,
                        size * (.014f + layer * .012f), size * .002f, settings,
                        settings.glowStrength * (.10f / layer) * (.6f + state.audio.bass * .4f));
        }
        if (settings.preset == 2)
            Bars(draw, center, radius, size, state, settings, 1.f, .95f, true);
        else
            Ribbon(draw, contour, quality.points, center, radius, size * .012f,
                size * (settings.preset ? .004f : .003f), settings, .85f);

        const float punch = settings.preset == 2 ? state.audio.punch : state.audio.bass;
        const float pulse = settings.logoPulse ? punch * settings.bassReaction * .12f : 0.f;
        const float diskRadius = radius * (.89f + pulse);
        draw->AddCircleFilled(center, diskRadius, IM_COL32(9, 8, 19, static_cast<int>(230 * settings.opacity)), quality.points / 2);
        Ribbon(draw, circle, quality.points, center, diskRadius, size * .005f, size * .001f, settings, .65f);
        if (font) {
            const ImFontGlyph* glyph = font->FindGlyphNoFallback('R');
            if (glyph && glyph->Visible) {
                const float fontSize = size * .26f * (1.f + pulse);
                const float scale = fontSize / font->FontSize;
                const ImVec2 origin(center.x - (glyph->X0 + glyph->X1) * .5f * scale,
                    center.y - (glyph->Y0 + glyph->Y1) * .5f * scale);
                const auto& color = settings.logoColor;
                font->RenderChar(draw, fontSize, origin, ImGui::ColorConvertFloat4ToU32(
                    ImVec4(color[0], color[1], color[2], color[3] * settings.opacity)), static_cast<ImWchar>(glyph->Codepoint));
            }
        }
        draw->PopClipRect();
    }

    Frame Demo(float dt) {
        demoTime += dt;
        Frame frame;
        frame.generation = std::uint64_t{1} << 63;
        frame.sequence = ++demoSequence;
        frame.sampleRate = 44100;
        frame.timestamp = Clock::now();
        frame.valid = true;
        const float kick = std::pow((std::max)(0.f, std::cos(demoTime * 2.f * Pi * 1.8f)), 12.f);
        for (int bin = 1; bin < BinCount; ++bin) {
            const float frequency = bin * (44100.f / FftSize);
            frame.fft[bin] = .018f * std::exp(-frequency / 5000.f) *
                (.55f + .4f * std::sin(frequency * .0088f + demoTime * 3.f));
            if (frequency < 180.f) frame.fft[bin] += kick * .07f;
        }
        return frame;
    }

    void SettingSlider(const char* label, float* value, float low, float high, const char* format) {
        // This fork adds +/- controls to SliderFloat and positions its own label.
        // Separate the label and reserve room for both buttons so narrow columns remain readable.
        ImGui::PushID(label);
        ImGui::TextUnformatted(label);
        ImGui::PushItemWidth((std::max)(80.f, ImGui::GetContentRegionAvail().x - 56.f));
        ImGui::SliderFloat(" ##value", value, low, high, format);
        ImGui::PopItemWidth();
        ImGui::PopID();
    }

    void SettingColor(const char* label, float* value) {
        ImGui::TextUnformatted(label);
        ImGui::PushID(label);
        ImGui::PushItemWidth(32.f);
        ImGui::ColorEdit4("##color", value, true, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        ImGui::PopItemWidth();
        ImGui::PopID();
    }

    void SettingsControls(Settings& s) {
        const char* presets[] = { "Nation", "Gutter", "Bars" };
        if (ImGui::Combo("Preset", &s.preset, presets, 3)) {
            ResetPreset(s);
            preview = {}; overlay = {};
        }
        const char* qualities[] = { "Low", "Medium", "High" };
        ImGui::Combo("Quality", &s.quality, qualities, 3);
        SettingSlider("Size", &s.size, 180.f, 640.f, "%.0f px");
        SettingSlider("Opacity", &s.opacity, .05f, 1.f, "%.2f");
        ImGui::Spacing();
        ImGui::TextUnformatted("Colors");
        SettingColor("Gradient start", s.colorA);
        SettingColor("Gradient end", s.colorB);
        SettingColor("R logo", s.logoColor);
        ImGui::Spacing();
        ImGui::TextUnformatted("Audio response");
        SettingSlider("Sensitivity", &s.sensitivity, .25f, 3.f, "%.2fx");
        SettingSlider("Smoothing", &s.smoothing, .25f, 3.f, "%.2fx");
        SettingSlider("Bass reaction", &s.bassReaction, 0.f, 2.f, "%.2fx");
        ImGui::Checkbox("Logo pulse", &s.logoPulse);
        ImGui::Spacing();
        ImGui::TextUnformatted("Effects");
        ImGui::Checkbox("Glow", &s.glow);
        if (s.glow) SettingSlider("Glow strength", &s.glowStrength, 0.f, 1.f, "%.2f");
        ImGui::Checkbox("Beat rings", &s.rings);
        if (s.rings) SettingSlider("Ring strength", &s.ringStrength, 0.f, 1.f, "%.2f");
        ImGui::Checkbox("Particles", &s.particles);
        if (s.particles) SettingSlider("Density", &s.particleDensity, 0.f, 1.f, "%.2f");
        ImGui::Spacing();
        if (ImGui::Button("Reset preset", ImVec2(ImGui::GetContentRegionAvail().x, 25.f), true, true)) {
            ResetPreset(s); preview = {}; overlay = {};
        }
        if (ImGui::Button("Reset position", ImVec2(ImGui::GetContentRegionAvail().x, 25.f), true, true)) {
            s.position[0] = .03f; s.position[1] = .97f; s.size = 320.f;
        }
        Sanitize(s);
    }
}

void radio_visualizer::OpenSettings() { modalRequested = true; }

void radio_visualizer::RenderOverlay(ImFont* logoFont, bool menuOpen, bool radioEnabled) {
    auto& settings = cfg::radioVisualizer;
    Sanitize(settings);
    if (!menuOpen) modalVisible = modalRequested = false;
    SetAnalysisEnabled((settings.enabled && radioEnabled) || (menuOpen && (modalVisible || modalRequested)));
    if (!settings.enabled || !radioEnabled) { overlay = {}; return; }
    const auto display = ImGui::GetIO().DisplaySize;
    if (display.x <= 16.f || display.y <= 16.f) return;
    const float available = (std::min)(display.x, display.y) - 16.f;
    const float side = (std::min)(settings.size, available);
    ImVec2 position(std::clamp(settings.position[0] * display.x, 8.f, display.x - side - 8.f),
        std::clamp(settings.position[1] * display.y, 8.f, display.y - side - 8.f));
    struct Placement { float x = -1.f, y = -1.f; ImVec2 display{}; };
    static Placement last;
    if (settings.position[0] != last.x || settings.position[1] != last.y ||
        display.x != last.display.x || display.y != last.display.y)
        ImGui::SetNextWindowPos(position, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(side, side), ImGuiCond_Always);
    ImGui::SetNextWindowSizeConstraints(ImVec2((std::min)(180.f, available), (std::min)(180.f, available)),
        ImVec2((std::min)(640.f, available), (std::min)(640.f, available)), [](ImGuiSizeCallbackData* data) {
            const float dimension = (std::max)(data->DesiredSize.x, data->DesiredSize.y);
            data->DesiredSize = ImVec2(dimension, dimension);
        });
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(1.f, 1.f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
    auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (!menuOpen || modalVisible) flags |= ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    if (ImGui::Begin("##RadioVisualizerOverlay", nullptr, flags)) {
        position = ImGui::GetWindowPos();
        const float actualSize = ImGui::GetWindowSize().x;
        position.x = std::clamp(position.x, 8.f, display.x - actualSize - 8.f);
        position.y = std::clamp(position.y, 8.f, display.y - actualSize - 8.f);
        ImGui::SetWindowPos(position);
        settings.size = Bounded(actualSize, 180.f, 640.f, 320.f);
        settings.position[0] = std::clamp(position.x / display.x, 0.f, 1.f);
        settings.position[1] = std::clamp(position.y / display.y, 0.f, 1.f);
        last = { settings.position[0], settings.position[1], display };
        Animate(overlay, GetFrame(), Bounded(ImGui::GetIO().DeltaTime, 0.f, .1f, 0.f), settings);
        Draw(ImGui::GetBackgroundDrawList(), position, actualSize, logoFont, overlay, settings);
        if (menuOpen && !modalVisible) {
            auto* draw = ImGui::GetWindowDrawList();
            draw->AddRect(position, ImVec2(position.x + actualSize, position.y + actualSize), IM_COL32(150, 125, 200, 75), 5.f);
            draw->AddText(ImVec2(position.x + 9.f, position.y + 8.f), IM_COL32(190, 180, 210, 180), "Radio visualizer - drag / resize");
        }
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
}

void radio_visualizer::DrawSettingsModal(ImFont* logoFont) {
    constexpr const char* name = "Radio visualizer settings";
    if (modalRequested) {
        ImGui::OpenPopup(name);
        modalRequested = false;
    }
    const auto display = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowSize(ImVec2((std::min)(740.f, display.x - 24.f), (std::min)(600.f, display.y - 24.f)), ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(display.x * .5f, display.y * .5f), ImGuiCond_Appearing, ImVec2(.5f, .5f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(250.f, 250.f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.f, 12.f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.f, 4.f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.f, 8.f));
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 8.f);
    bool open = true;
    modalVisible = ImGui::BeginPopupModal(name, &open, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);
    if (modalVisible) {
        auto& settings = cfg::radioVisualizer;
        Sanitize(settings);
        SetAnalysisEnabled(true);
        ImGui::Checkbox("Enable overlay", &settings.enabled);
        const bool wide = ImGui::GetContentRegionAvail().x >= 560.f;
        const float contentHeight = (std::max)(100.f, ImGui::GetContentRegionAvail().y - 60.f);
        const bool table = wide && ImGui::BeginTable("##VisualizerLayout", 2, ImGuiTableFlags_SizingStretchProp);
        if (table) {
            ImGui::TableSetupColumn("Preview", ImGuiTableColumnFlags_WidthStretch, .9f);
            ImGui::TableSetupColumn("Settings", ImGuiTableColumnFlags_WidthStretch, 1.1f);
            ImGui::TableNextColumn();
        }
        ImGui::BeginChild("##VisualizerPreview", ImVec2(0.f, wide ? contentHeight : 180.f), false);
        const auto frame = GetFrame();
        const bool live = IsFresh(frame, Clock::now());
        ImGui::TextUnformatted(live ? "LIVE RADIO" : "DEMO PREVIEW - no live audio");
        if (frame.status == AnalysisStatus::Unsupported)
            ImGui::TextWrapped("Spectrum unavailable: required BASS exports are missing.");
        const float previewSize = (std::max)(1.f, (std::min)(ImGui::GetContentRegionAvail().x,
            ImGui::GetContentRegionAvail().y - (wide ? 65.f : 0.f)));
        const ImVec2 start = ImGui::GetCursorScreenPos();
        const ImVec2 previewStart(start.x + (ImGui::GetContentRegionAvail().x - previewSize) * .5f, start.y);
        const float dt = Bounded(ImGui::GetIO().DeltaTime, 0.f, .1f, 0.f);
        Animate(preview, live ? frame : Demo(dt), dt, settings);
        Draw(ImGui::GetWindowDrawList(), previewStart, previewSize, logoFont, preview, settings);
        ImGui::Dummy(ImVec2(previewSize, previewSize));
        if (wide) ImGui::TextWrapped("Drag and resize the overlay with the menu open. Muting the radio keeps the animation running.");
        ImGui::EndChild();
        if (table) ImGui::TableNextColumn();
        ImGui::BeginChild("##VisualizerControls", ImVec2(0.f, wide ? contentHeight : (std::max)(60.f, ImGui::GetContentRegionAvail().y - 52.f)), false);
        ImGui::PushItemWidth((std::max)(80.f, ImGui::GetContentRegionAvail().x - 108.f));
        SettingsControls(settings);
        ImGui::PopItemWidth();
        ImGui::EndChild();
        if (table) ImGui::EndTable();
        ImGui::Separator();
        if (ImGui::Button("Close", ImVec2(120.f, 25.f), true, true)) {
            ImGui::CloseCurrentPopup();
            modalVisible = false;
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(5);
}
