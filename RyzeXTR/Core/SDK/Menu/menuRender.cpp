#include "gui.h"
#include "../../Features/Rage/antiaim.h"

void menu::HandleMenuElements() noexcept {

    ImGui::Begin(("##main"), NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings /*| ImGuiWindowFlags_NoResize*/);
    {
        if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
            if (ImGui::IsKeyPressed(ImGuiKey_S)) {
                bPressedSave = true;
                bWarningMethod = true;
            }
        }

        if (bPressedSave && cfg::configID != -1)
            SaveWarning(bPressedSave, bWarningMethod);

        static float lmao = 0.f;
        DrawBackgroundTexture(lmao += ImGui::GetIO().DeltaTime);
        menu::DrawCustomChildRounding(("##Tabs"), ImVec2(120, ImGui::GetContentRegionAvail().y), true, 0, ImDrawCornerFlags_TopLeft | ImDrawCornerFlags_BotLeft);
        {
            float save = ImGui::GetCursorPosY();
            ImGui::PushFont(weaponIcons);
            ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 32.f, save + 19.f));
            ImGui::Text(("R#1"));
            ImGui::PopFont();

            ImGui::PushFont(tabIcons);
            ImGui::SetCursorPosY(save + 100);
            if (menu::ButtonCenteredOnLine(("c"), 0.5f, iSelectedTab == RAGE_TAB))
                iSelectedTab = RAGE_TAB;
            if (menu::ButtonCenteredOnLine(("f"), 0.5f, iSelectedTab == ANTIAIM_TAB))
                iSelectedTab = ANTIAIM_TAB;
            if (menu::ButtonCenteredOnLine(("g"), 0.5f, iSelectedTab == VISUAL_TAB))
                iSelectedTab = VISUAL_TAB;
            if (menu::ButtonCenteredOnLine(("a"), 0.5f, iSelectedTab == MISC_TAB))
                iSelectedTab = MISC_TAB;
            if (menu::ButtonCenteredOnLine(("e"), 0.5f, iSelectedTab == SKIN_TAB))
                iSelectedTab = SKIN_TAB;
            if (menu::ButtonCenteredOnLine(("b"), 0.5f, iSelectedTab == PLAYERLST))
                iSelectedTab = PLAYERLST;
            if (menu::ButtonCenteredOnLine(("d"), 0.5f, iSelectedTab == CONFIG_TAB))
                iSelectedTab = CONFIG_TAB;
            ImGui::PopFont();
        }
        ImGui::EndChild();

        ImGui::SameLine();
        ImVec2 savedCursorPosition = ImGui::GetCursorPos();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.f, 10.f));

        ImGui::Spacing();
        ImGui::SetCursorPos(ImVec2(savedCursorPosition.x - ImGui::GetStyle().WindowPadding.x, savedCursorPosition.y + ImGui::GetWindowHeight() - 25));
        menu::DrawCustomChildRounding(("##BotmBar"), ImVec2(ImGui::GetContentRegionAvail().x, 25), true, 0, ImDrawCornerFlags_BotRight);
        {
            menu::Text(("2023 - RyzeX"), 1.05f);
        }
        ImGui::EndChild();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.f, 2.f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.f);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.f);
        ImGui::SetCursorPos(savedCursorPosition);
        switch (iSelectedTab) {

        case RAGE_TAB: Rage(savedCursorPosition);
            break;

        case ANTIAIM_TAB: AntiAim(savedCursorPosition);
            break;

        case VISUAL_TAB: Visual(savedCursorPosition);
            break;

        case MISC_TAB: Misc(savedCursorPosition);
            break;

        case SKIN_TAB: Skins(savedCursorPosition);
            break;

        case CONFIG_TAB: Config(savedCursorPosition);
            break;

        case PLAYERLST: PlayerList(savedCursorPosition);
            break;
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleVar(4);

    }
    ImGui::End();
}

void menu::Rage(ImVec2 savedCursorPosition) {

    using namespace cfg::rage;

    static int iSelect = 0;
    static const char* arrHitboxNames[] = {("Head"), ("Upper chest"), ("Lower chest"), ("Stomach"), ("Arms"), ("Legs")};
    static const char* arrConditionNames[] = { ("Between shots"), ("In air") };

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ImGui::GetStyle().WindowPadding.x + 1);
    menu::DrawCustomChildRounding(("##TopBar"), ImVec2(ImGui::GetContentRegionAvail().x, 80), true, 0, ImDrawCornerFlags_TopRight);
    {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().WindowPadding.y);
        ImGui::PushFont(weaponIcons);
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);
            ImVec2 size = ImVec2((ImGui::GetContentRegionAvail().x - 5 * 9) / 7.f, ImGui::GetContentRegionAvail().y + ImGui::GetStyle().WindowPadding.y);
            if (ImGui::Button(("A##2"), size, iSelect == PISTOL))
                iSelect = PISTOL;
            ImGui::SameLine();

            if (ImGui::Button(("B##2"), size, iSelect == HEAVY_PISTOL))
                iSelect = HEAVY_PISTOL;
            ImGui::SameLine();

            if (ImGui::Button(("F##2"), size, iSelect == SCOUT))
                iSelect = SCOUT;
            ImGui::SameLine();

            if (ImGui::Button(("C##2"), size, iSelect == AWP))
                iSelect = AWP;
            ImGui::SameLine();

            if (ImGui::Button(("D##2"), size, iSelect == AUTO))
                iSelect = AUTO;
            ImGui::SameLine();

            if (ImGui::Button(("G##2"), size, iSelect == ZEUS))
                iSelect = ZEUS;
            ImGui::PopFont();

            ImGui::SameLine();
            ImGui::PushFont(tabIconsPreview);
            {
                ImGui::PushStyleColor(ImGuiCol_Text, clr::text_preview);
                menu::Text("c", 0.9f, true);
                ImGui::PopStyleColor();
            }
            ImGui::PopFont();

            ImGui::PopStyleVar();
        }
    }
    ImGui::EndChild();

    ImVec2 Padding = ImVec2(ImGui::GetStyle().WindowPadding.x, ImGui::GetStyle().WindowPadding.y);
    ImGui::SetCursorPos(ImVec2(savedCursorPosition.x, savedCursorPosition.y + 80 + ImGui::GetStyle().WindowPadding.y));

    ImVec2 TopLeftSize = ImVec2(ImGui::GetContentRegionAvail().x / 2 - Padding.x, ImGui::GetContentRegionAvail().y / 2 - Padding.y - 25.f);
    ImGui::BeginChild(("##TopLeft"), ImVec2(ImGui::GetContentRegionAvail().x / 2 - Padding.x, ImGui::GetContentRegionAvail().y / 2 - Padding.y - 25.f), true);
    {
        ImGui::Checkbox(("Enabled##0"), &bEnable);
        ImGui::Keybind(("aimbotkey"), &iAimbotKey);
        ImGui::Checkbox(("Silent"), &bSilentAim);
        ImGui::SliderFloat(("Fov"), &iAimbotFov, 0.f, 180.f, ("%.f")); 
        ImGui::Checkbox(("Resolver"), &bResolver);

        ImGui::Checkbox(("Force baim"), &bForceBaim);
        ImGui::Keybind(("fbaimKey"), &iForceBaimKey);

        ImGui::Checkbox(("Override"), &bOverride);
        ImGui::Keybind(("Overridekey"), &iOverrideBind);
    }
    ImGui::EndChild();

    ImGui::SetCursorPos(ImVec2(savedCursorPosition.x, savedCursorPosition.y + 80 + TopLeftSize.y + Padding.y * 2));

    ImGui::BeginChild(("##BotLeft"), ImVec2(ImGui::GetContentRegionAvail().x / 2 - Padding.x, ImGui::GetContentRegionAvail().y - Padding.y - 25.f), true);
    {
        ImGui::Checkbox(("Doubletap"), &bDoubletap);
        ImGui::Keybind(("##iDoubletapKey"), &iDoubletapKey);

        ImGui::Checkbox(("Hideshot"), &bHideshot);
        ImGui::Keybind(("##iHideShotKey"), &iHideShotKey);
    }
    ImGui::EndChild();

    ImGui::SetCursorPos(ImVec2(savedCursorPosition.x + TopLeftSize.x + Padding.x, savedCursorPosition.y + 80 + ImGui::GetStyle().WindowPadding.y));

    ImGui::BeginChild(("##RightWhole"), ImVec2(ImGui::GetContentRegionAvail().x - Padding.x, ImGui::GetContentRegionAvail().y - Padding.y - 25.f), true);
    {
        ImGui::MultiComboBox(("Hitbox"), arrHitboxNames, bHitboxes[iSelect], IM_ARRAYSIZE(arrHitboxNames));
        ImGui::SliderInt(("Hitchance"), &iHitchances[iSelect], 0, 100);
        ImGui::SliderInt(("Damage"), &iMinDamages[iSelect], 0, 110);
        ImGui::SliderInt(("Override##2"), &iOverride[iSelect], 0, 110);

        ImGui::MultiComboBox(("Pointscale"), arrHitboxNames, bMultiHitboxes[iSelect], IM_ARRAYSIZE(arrHitboxNames));
        ImGui::SliderInt(("HeadScale"), &iHeadPoints[iSelect], 0, 100);
        ImGui::SliderInt(("BodyScale"), &iBodyPoints[iSelect], 0, 100);

        ImGui::MultiComboBox(("SafeHitbox"), arrHitboxNames, bSafeHitboxes[iSelect], IM_ARRAYSIZE(arrHitboxNames));
        ImGui::Checkbox(("Force overlap"), &bForceSafePoint[iSelect]);

        ImGui::Checkbox(("Auto stop"), &bAutostop[iSelect]);
        if (bAutostop[iSelect]) {
            bool bConditionsarray[] = { bConditions[iSelect][0], bConditions[iSelect][1] };
            ImGui::MultiComboBox(("Condition"), arrConditionNames, bConditionsarray, IM_ARRAYSIZE(arrConditionNames));
        }
        if (iSelect < 3) {

            ImGui::Checkbox(("Auto scope"), &bAutoScope[iSelect]);
        }
    }
    ImGui::EndChild();
}

void menu::AntiAim(ImVec2 savedCursorPosition) {

    using namespace cfg::antiaim;

    static int iSelect = 0;
    static const char* arrPitches[] = { ("Off"), ("Up"), ("Zero"), ("Down") };
    static const char* arrYawBases[] = { ("Local view"), ("At target") };
    static const char* arrYaws[] = { ("Forward"), ("Backward") };
    static const char* arrModifiers[] = { ("Off"), ("Jitter"), ("Random") };
    static const char* arrDesyncs[] = { ("Off"), ("Static"), ("Extended"), ("Jitter"), ("Flick"), ("Rytter")};
    static const char* arrFreestands[] = { ("Off"), ("Circular"), ("Predictive")};
    static const char* arrFakelagType[] = { ("Normal"), ("Adaptive"), ("Jitter") };

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ImGui::GetStyle().WindowPadding.x + 1);
    menu::DrawCustomChildRounding(("##TopBar"), ImVec2(ImGui::GetContentRegionAvail().x, 80), true, 0, ImDrawCornerFlags_TopRight);
    {
        ImGui::PushFont(defaultFontBigger);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().WindowPadding.y);

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);

        ImVec2 size = ImVec2((ImGui::GetContentRegionAvail().x - 5 * 9) / 4.f, ImGui::GetContentRegionAvail().y + ImGui::GetStyle().WindowPadding.y);
        if (ImGui::Button(("Standing"), size, iSelect == STANDING))
            iSelect = STANDING;
        ImGui::SameLine();

        if (ImGui::Button(("Moving"), size, iSelect == MOVING))
            iSelect = MOVING;
        ImGui::SameLine();

        if (ImGui::Button(("Jumping"), size, iSelect == INAIR))
            iSelect = INAIR;

        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::PushFont(tabIconsPreview);
        {
            ImGui::PushStyleColor(ImGuiCol_Text, clr::text_preview);
            menu::Text("f", 1.f, true);
            ImGui::PopStyleColor();
        }
        ImGui::PopFont();

        ImGui::PopStyleVar();
    }
    ImGui::EndChild();

    ImVec2 Padding = ImVec2(ImGui::GetStyle().WindowPadding.x, ImGui::GetStyle().WindowPadding.y);
    ImGui::SetCursorPos(ImVec2(savedCursorPosition.x, savedCursorPosition.y + 80 + ImGui::GetStyle().WindowPadding.y));

    ImVec2 TopLeftSize = ImVec2(ImGui::GetContentRegionAvail().x / 2.f - Padding.x, ImGui::GetContentRegionAvail().y - Padding.y - 25.f);
    ImGui::BeginChild(("##LeftWhole"), ImVec2(ImGui::GetContentRegionAvail().x / 2.f - Padding.x, ImGui::GetContentRegionAvail().y - Padding.y - 25.f), true);
    {
        ImGui::Checkbox(("Enable"), &bEnabled[iSelect]);
        if ((bEnabled[STANDING] && iSelect == STANDING) || (bEnabled[MOVING] && iSelect == MOVING) || (bEnabled[INAIR] && iSelect == INAIR)) {

            ImGui::Combo(("Pitch"), &iPitch[iSelect], arrPitches, IM_ARRAYSIZE(arrPitches));
            ImGui::Combo(("Base"), &iYawBase[iSelect], arrYawBases, IM_ARRAYSIZE(arrYawBases));
            ImGui::Combo(("Yaw"), &iYaw[iSelect], arrYaws, IM_ARRAYSIZE(arrYaws));
            ImGui::Combo(("Modifier"), &iModifier[iSelect], arrModifiers, IM_ARRAYSIZE(arrModifiers));
            if (iModifier[iSelect] != 0) {
                ImGui::SliderInt(("Value"), &iJitterValue[iSelect], 0, 90);
                ImGui::Checkbox(("Anti Prediction"), &bAntiJitter[iSelect]);
            }
            ImGui::Combo(("Desync"), &iDesyncType[iSelect], arrDesyncs, IM_ARRAYSIZE(arrDesyncs));
            if (iDesyncType[iSelect] != 0 && iDesyncType[iSelect] != 5) {
                ImGui::Checkbox(("Inverter"), &bInverter);
                ImGui::Keybind(("##iInverterBind"), &iInverterBind);
                ImGui::SliderFloat(("Lean"), &flBodyLean[0][iSelect], -90.f, 90.f, ("%.f"));
                ImGui::SliderFloat(("Invert Lean"), &flBodyLean[1][iSelect], -90.f, 90.f, ("%.f"));
                if (iDesyncType[iSelect] == 4) {
                    ImGui::SliderInt(("Offset"), &iFlickOffset[iSelect], -90, 90);
                    ImGui::SliderInt(("Switch tick"), &flickAngleSwitch[iSelect], 0, iFakeLagMax);
                }
            }
            else if (iDesyncType[iSelect] == 5) {

                if (ImGui::Button(("Add way##1"), ImVec2(ImGui::GetContentRegionAvail().x / 2, 20)) && iEnabledJitters[STANDING] < vecJitterWays[STANDING].size() - 1)
                    iEnabledJitters[STANDING]++;
                ImGui::SameLine();
                if (ImGui::Button(("Remove way##1"), ImVec2(ImGui::GetContentRegionAvail().x, 20)) && iEnabledJitters[STANDING] > 0)
                    iEnabledJitters[STANDING]--;
            }
            ImGui::Checkbox(("Invert on shot"), &bInvertOnShoot[iSelect]);
            ImGui::Combo(("Freestand"), &iFreestand[iSelect], arrFreestands, IM_ARRAYSIZE(arrFreestands));
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImVec2 NextWindowCursor = ImVec2(ImGui::GetCursorPos().x, ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y / 2.f - 25.f);

    ImGui::BeginChild(("##TopRight"), ImVec2(ImGui::GetContentRegionAvail().x - Padding.x, ImGui::GetContentRegionAvail().y / 2.f - Padding.y - 25.f), true);
    {
        ImGui::Checkbox(("Fakelag"), &bFakelag);
        if (bFakelag) {

            iFakelagMin = std::clamp(iFakelagMin, 0, iFakelag);
            iFakeLagMax = std::clamp(iFakeLagMax, iFakelagMin, iFakelag);

            ImGui::Combo(("Type"), &iFakeLagType, arrFakelagType, IM_ARRAYSIZE(arrFakelagType));
            ImGui::SliderInt(("Base"), &iFakelag, 0, 14);
            ImGui::SliderInt(("Min"), &iFakelagMin, 0, iFakelag);
            ImGui::SliderInt(("Max"), &iFakeLagMax, iFakelagMin, iFakelag);

            ImGui::Checkbox(("Break lagcompenstaion"), &bDefensive);

            ImGui::Checkbox(("Fake ping"), &cfg::misc::bFakePing);
            if (cfg::misc::bFakePing)
                ImGui::SliderFloat(("Value"), &cfg::misc::flFakePingFactor, 0.f, 1000.f, ("%.f"));
        }
    }
    ImGui::EndChild();

    ImGui::SetCursorPos(NextWindowCursor);
    ImGui::BeginChild(("##BotRight"), ImVec2(ImGui::GetContentRegionAvail().x - Padding.x, ImGui::GetContentRegionAvail().y - Padding.y - 25.f), true);
    {
        ImGui::Checkbox(("Slow walk"), &bFakeWalk);
        ImGui::Keybind(("##iFakeWalkKey"), &iFakeWalkKey);
        if (bFakeWalk)
        ImGui::SliderFloat(("Speed"), &iFakeWalkSpeed, 0, 100, ("%.f"));
        ImGui::Checkbox(("Fakeduck"), &bFakeDuck);
        ImGui::Keybind(("##iFakeDuckKey"), &iFakeDuckKey);
    }
    ImGui::EndChild();
}

void menu::Visual(ImVec2 savedCursorPosition) {

    static int iSelect = 0;
    static const char* arrFlagNames[] = {("Name"), ("Health"), ("Armor"), ("Ammo"), ("Money"), ("Weapon"), ("Resolver"), ("Fakeduck"), ("Aimbot")};
    static const char* arrChamsType[] = { ("Default"), ("Flat"), ("Glow"), ("Thin glow"), ("Animated") };
    static const char* arrMaterialType[] = { ("Default"), ("Flat") };
    static const char* arrSkyboxes[] = { "None", "Baggage", "Tibet", "Vietnam",  "Lunacy", "Embassy", "Italy", "Jungle", "Office", "Daylight1", "Daylight2", "Daylight3", "Daylight4", "Day", "Nuke", "Dust", "Venice", "Cloudy", "Night1", "Night2", "Vertigo", "VertigoHdr", "SkyDust", "Aztec"  };
    
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ImGui::GetStyle().WindowPadding.x + 1);
    menu::DrawCustomChildRounding(("##TopBar"), ImVec2(ImGui::GetContentRegionAvail().x, 80), true, 0, ImDrawCornerFlags_TopRight);
    {
        ImGui::PushFont(defaultFontBigger);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().WindowPadding.y);

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);

        ImVec2 size = ImVec2((ImGui::GetContentRegionAvail().x - 5 * 9) / 4.f, ImGui::GetContentRegionAvail().y + ImGui::GetStyle().WindowPadding.y);
        if (ImGui::Button(("Enemy"), size, iSelect == 0))
            iSelect = 0;
        ImGui::SameLine();

        if (ImGui::Button(("Team"), size, iSelect == 1))
            iSelect = 1;
        ImGui::SameLine();

        if (ImGui::Button(("Local"), size, iSelect == 2))
            iSelect = 2;

        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::PushFont(tabIconsPreview);
        {
            ImGui::PushStyleColor(ImGuiCol_Text, clr::text_preview);
            menu::Text("g", 1.f, true);
            ImGui::PopStyleColor();
        }
        ImGui::PopFont();

        ImGui::PopStyleVar();
    }
    ImGui::EndChild();

    ImVec2 Padding = ImVec2(ImGui::GetStyle().WindowPadding.x, ImGui::GetStyle().WindowPadding.y);
    ImGui::SetCursorPos(ImVec2(savedCursorPosition.x, savedCursorPosition.y + 80 + ImGui::GetStyle().WindowPadding.y));

    ImVec2 TopLeftSize = ImVec2(ImGui::GetContentRegionAvail().x / 2.f - Padding.x, ImGui::GetContentRegionAvail().y * 0.7f - Padding.y - 25.f);
    ImGui::BeginChild(("##LeftTop"), ImVec2(ImGui::GetContentRegionAvail().x / 2.f - Padding.x, ImGui::GetContentRegionAvail().y * 0.7f - Padding.y - 25.f), true);
    {
        using namespace cfg::visual;
        ImGui::Checkbox(("Enable##0"), &bEnable[iSelect]);
        ImGui::Checkbox(("Bounding bBox"), &bBox[iSelect]);
        ImGui::ColorEdit4(("##flBoxColor"), flBoxColor[iSelect]);

        ImGui::Checkbox(("Health bar"), &bHealth[iSelect]);
        ImGui::ColorEdit4(("##flHealthColorStart"), flHealthColorStart[iSelect], true);
        ImGui::ColorEdit4(("##flHealthColorEnd"), flHealthColorEnd[iSelect]);

        ImGui::Checkbox(("Name"), &bName[iSelect]);
        ImGui::ColorEdit4(("##flNameColor"), flNameColor[iSelect]);

        ImGui::Checkbox(("Weapon"), &bWeapon[iSelect]);
        ImGui::ColorEdit4(("##flWeaponColor"), flWeaponColor[iSelect]);

        ImGui::Checkbox(("Ammo"), &bAmmo[iSelect]);
        ImGui::ColorEdit4(("##flAmmoColor"), flAmmoColor[iSelect]);

        ImGui::Checkbox(("Glow"), &bGlow[iSelect]);
        ImGui::ColorEdit4(("##flGlowColor"), flGlowColor[iSelect]);

        ImGui::Checkbox(("Skeleton"), &bSkeleton[iSelect]);
        ImGui::ColorEdit4(("##flSkeletonColor"), flSkeletonColor[iSelect]);

        ImGui::Checkbox(("Bullet tracer"), &bBulletTracer[iSelect]);
        ImGui::ColorEdit4(("##flBulletTracerColor"), flBulletTracerColor[iSelect]);

        if (iSelect == ENEMY) {

            ImGui::Checkbox(("Out of fov"), &cfg::misc::bOOF);
            if (cfg::misc::bOOF) {
                ImGui::ColorEdit4(("##flOOF"), cfg::misc::flOOF);
                ImGui::SliderInt(("Distance##2"), &cfg::misc::iOOFDistance, 1, 100);
                ImGui::SliderInt(("Size"), &cfg::misc::iOOFSize, 1, 30);
            }
        }
        ImGui::MultiComboBox(("Flags"), arrFlagNames, bFlags[iSelect], IM_ARRAYSIZE(arrFlagNames));
    }
    ImGui::EndChild();

    ImGui::SetCursorPos(ImVec2(savedCursorPosition.x, savedCursorPosition.y + 80 + ImGui::GetStyle().WindowPadding.y + TopLeftSize.y + Padding.y));
    ImGui::BeginChild(("##LeftBot"), ImVec2(ImGui::GetContentRegionAvail().x / 2.f - Padding.x, ImGui::GetContentRegionAvail().y - Padding.y - 25.f), true);
    {
        using namespace cfg::model;
        // normal
        ImGui::Checkbox(("Player"), &bChams[iSelect]);
        ImGui::SameLine();
        ImGui::Checkbox(("Wireframe##1"), &bXhair[iSelect]);
        ImGui::ColorEdit4(("##ChamsColor"), ChamsColor[iSelect]);

        ImGui::Checkbox(("Player behind wall"), &bChamsXQZ[iSelect]);
        ImGui::SameLine();
        ImGui::Checkbox(("Wireframe##2"), &bXhairXQZ[iSelect]);
        ImGui::ColorEdit4(("##ChamsColorXQZ"), ChamsColorXQZ[iSelect]);
        ImGui::Combo(("Material"), &iType[iSelect], arrMaterialType, IM_ARRAYSIZE(arrMaterialType));

        if (iSelect == ENEMY) {

            ImGui::Checkbox(("Backtrack"), &enemyBTEnable);
            if (enemyBTEnable) {
                ImGui::SameLine();
                ImGui::Checkbox(("WireFrame##9"), &enemyBTXhair);
                ImGui::ColorEdit4(("##enemyBTColor"), enemyBTColor);
                ImGui::Combo(("Material##5"), &enemyBTType, arrChamsType, IM_ARRAYSIZE(arrChamsType));
            }
        }
        else if (iSelect == LOCAL) {

            ImGui::Checkbox(("Desync"), &localDesync);
            if (enemyBTEnable) {
                ImGui::SameLine();
                ImGui::Checkbox(("WireFrame##9"), &localDesyncXhair);
                ImGui::ColorEdit4(("##localDesyncColor"), localDesyncColor);
                ImGui::Combo(("Material##4"), &localDesyncType, arrChamsType, IM_ARRAYSIZE(arrChamsType));
            }
        }

        static const char* iOverlayTypes[] = { ("Glow"), ("Thin"), ("Animated") };
        static int iSelectedOverlay = 0;
        ImGui::Combo(("Overlay##1337"), &iSelectedOverlay, iOverlayTypes, IM_ARRAYSIZE(iOverlayTypes));
        switch (iSelectedOverlay) {
        case 0:
            // overlay
            ImGui::Checkbox(("Overlay##1"), &bOverlay[iSelect]);
            ImGui::SameLine();
            ImGui::Checkbox(("Wireframe##7"), &bOverlayXhair[iSelect]);
            ImGui::ColorEdit4(("##OverlayColor"), OverlayColor[iSelect]);

            ImGui::Checkbox(("Overlay behind wall##1"), &bOverlayXQZ[iSelect]);
            ImGui::SameLine();
            ImGui::Checkbox(("Wireframe##8"), &bOverlayXhairXQZ[iSelect]);
            ImGui::ColorEdit4(("##OverlayColorXQZ"), OverlayColorXQZ[iSelect]);
            break;

        case 1:
            // thin
            ImGui::Checkbox(("Overlay##2"), &bThinOverlay[iSelect]);
            ImGui::SameLine();
            ImGui::Checkbox(("Wireframe##3"), &bThinOverlayXhair[iSelect]);
            ImGui::ColorEdit4(("##ThinOverlayColor"), ThinOverlayColor[iSelect]);

            ImGui::Checkbox(("Overlay behind wall##2"), &bThinOverlayXQZ[iSelect]);
            ImGui::SameLine();
            ImGui::Checkbox(("Wireframe##4"), &bThinOverlayXhairXQZ[iSelect]);
            ImGui::ColorEdit4(("##ThinOverlayColorXQZ"), ThinOverlayColorXQZ[iSelect]);
            break;

        case 2:
            // animated
            ImGui::Checkbox(("Overlay##3"), &bAnimOverlay[iSelect]);
            ImGui::SameLine();
            ImGui::Checkbox(("Wireframe##5"), &bAnimOverlayXhair[iSelect]);
            ImGui::ColorEdit4(("##AnimOverlayColor"), AnimOverlayColor[iSelect]);

            ImGui::Checkbox(("Overlay behind wall##3"), &bAnimOverlayXQZ[iSelect]);
            ImGui::SameLine();
            ImGui::Checkbox(("Wireframe##6"), &bAnimOverlayXhairXQZ[iSelect]);
            ImGui::ColorEdit4(("##AnimOverlayColorXQZ"), AnimOverlayColorXQZ[iSelect]);
            break;
        }

    }
    ImGui::EndChild();

    ImGui::SetCursorPos(ImVec2(savedCursorPosition.x + TopLeftSize.x + Padding.x, savedCursorPosition.y + 80 + ImGui::GetStyle().WindowPadding.y));
    ImVec2 TopRightSize = ImVec2(ImGui::GetContentRegionAvail().x - Padding.x, ImGui::GetContentRegionAvail().y * 0.4f - Padding.y - 25.f);
    ImGui::BeginChild(("##RightTop"), ImVec2(ImGui::GetContentRegionAvail().x - Padding.x, ImGui::GetContentRegionAvail().y * 0.4f - Padding.y - 25.f), true);
    {
        using namespace cfg::misc;
        using namespace cfg::visual;

        ImGui::Checkbox(("Always draw viewmodel"), &bOnScopeViewmodel);

        ImGui::Checkbox(("Dropped weapons"), &bDroppedWeaponESP);
        ImGui::ColorEdit4(("##flProjectileESP"), flProjectileESP);

        ImGui::Checkbox(("Projectile"), &bProjectileESP);
        ImGui::ColorEdit4(("##flDroppedWeaponESP"), flDroppedWeaponESP);

        ImGui::Checkbox(("Bullet Impact"), &bDrawCapsule);
        ImGui::ColorEdit4(("##flImpactColor1"), flImpactColor[0], true);
        ImGui::ColorEdit4(("##flImpactColor2"), flImpactColor[1]);

        ImGui::Checkbox(("World crosshair"), &bWorldCrosshair);
        ImGui::ColorEdit4(("##flWorldCrosshairColor"), flWorldCrosshairColor);

        ImGui::Checkbox(("Capsule on hit"), &bProjectileESP);
        ImGui::ColorEdit4(("##flDrawCapsuleColor"), flDrawCapsuleColor, true);
        ImGui::ColorEdit4(("##flDrawCapsuleColorHit"), flDrawCapsuleColorHit);

        ImGui::Checkbox(("Model scale"), &bSkinnyBoy);
        if (bSkinnyBoy)
            ImGui::SliderInt(("Scale"), &iSkinnyBoy, 0, 100);
            
        ImGui::Checkbox(("Samurai hat"), &bHat);
        ImGui::ColorEdit4(("##flHat"), flHat);

        ImGui::Checkbox(("Custom hud"), &bCustomHud);
    }
    ImGui::EndChild();

    ImGui::SetCursorPos(ImVec2(savedCursorPosition.x + TopLeftSize.x + Padding.x, savedCursorPosition.y + 80 + ImGui::GetStyle().WindowPadding.y + TopRightSize.y + Padding.y));
    ImGui::BeginChild(("##RightBot"), ImVec2(ImGui::GetContentRegionAvail().x - Padding.x, ImGui::GetContentRegionAvail().y - 25.f - Padding.y), true);
    {       
        using namespace cfg::misc;

        ImGui::Checkbox(("Nightmode"), &bNightmode);
        ImGui::ColorEdit4(("##flNightmodeColor"), flNightmodeColor, true);
        ImGui::ColorEdit4(("##flPropColor"), flPropColor);

        ImGui::Combo(("##Skybox"), &iSkybox, arrSkyboxes, IM_ARRAYSIZE(arrSkyboxes));
        ImGui::ColorEdit4(("##flSkyboxColor"), flSkyboxColor);

        ImGui::Checkbox(("Lamp color"), &bOverrideLampColors);
        ImGui::ColorEdit4(("##flLampColors"), flLampColors);
        if (bOverrideLampColors)
            ImGui::SliderInt(("Flicker"), &iFlicker, 0, 255);

        ImGui::Checkbox(("Aspect ratio"), &bAspectRatio);
        if (bAspectRatio)
            ImGui::SliderInt(("##Value"), &iAspectRatio, 0, 100);

        ImGui::Checkbox(("Remove smoke"), &bRemovals[0]);
        ImGui::Checkbox(("Remove flash"), &bRemovals[1]);
        ImGui::Checkbox(("Remove recoil"), &bRemovals[2]);
        ImGui::Checkbox(("Remove zoom"), &bRemovals[3]);
        ImGui::Checkbox(("Remove scope"), &bRemovals[5]);
        if (bRemovals[5])
        {
            ImGui::ColorEdit4(("##scope stuff"), flScopeColor, true);
            ImGui::ColorEdit4(("##scope stuff2"), flScopeColorEnd);
            ImGui::SliderInt(("Scope length"), &iScopeLength, 0.f, 100.f);
        }
        ImGui::Checkbox(("Remove post processing"), &bRemovals[4]);
    }
    ImGui::EndChild();
}

void menu::Misc(ImVec2 savedCursorPosition) {

    static const char* arrRadios[] = { ("2000's"), ("Rock"), ("Techno"), ("Rap"), ("Chill"), ("Club"), ("House"), ("8-Bit"), ("8-Bit Alternative"), ("Lo-Fi"), ("Eurobeat"), ("Nightcore"),("Radio 1"), ("Phonk") };
    using namespace cfg::misc;

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ImGui::GetStyle().WindowPadding.x + 1);
    menu::DrawCustomChildRounding(("##TopBar"), ImVec2(ImGui::GetContentRegionAvail().x, 80), true, 0, ImDrawCornerFlags_TopRight);
    {
        ImGui::SameLine();
        ImGui::PushFont(tabIconsPreview);
        {
            ImGui::PushStyleColor(ImGuiCol_Text, clr::text_preview);
            menu::Text("a", 1.f, true);
            ImGui::PopStyleColor();
        }
        ImGui::PopFont();
    }
    ImGui::EndChild();

    ImVec2 Padding = ImVec2(ImGui::GetStyle().WindowPadding.x, ImGui::GetStyle().WindowPadding.y);
    ImGui::SetCursorPos(ImVec2(savedCursorPosition.x, savedCursorPosition.y + 80 + ImGui::GetStyle().WindowPadding.y));

    ImVec2 TopLeftSize = ImVec2(ImGui::GetContentRegionAvail().x / 2.f - Padding.x, ImGui::GetContentRegionAvail().y - Padding.y - 25.f);
    ImGui::BeginChild("##LeftWhole", ImVec2(ImGui::GetContentRegionAvail().x / 2.f - Padding.x, ImGui::GetContentRegionAvail().y - Padding.y - 25.f), true);
    {
        ImGui::Checkbox(("Thirdperson"), &bThirdPerson);
        ImGui::Keybind(("##iThirdPersonKey"), &iThirdPersonKey);
        if (bThirdPerson)
            ImGui::SliderInt(("Distance"), &iThirdPersonDistance, 0, 300);

        ImGui::SliderInt(("Viewmodel"), &iViewModelFov, 45, 140);
        ImGui::SliderInt(("Debug fov"), &iDebugFov, 40, 140);
        ImGui::Checkbox(("Bunny hop"), &bBunnyHop);
        ImGui::Checkbox(("Auto strafe"), &bAutoStrafe);
        ImGui::Checkbox(("Fast stop"), &bFastStop);
        ImGui::Checkbox(("Infinite duck"), &bInfiniteDuck);
        ImGui::Checkbox(("Blockbot"), &bBlockbot);
        ImGui::Keybind(("##iBlockbotKey"), &iBlockbotKey);

        ImGui::Checkbox(("Clantag"), &bClantag);
        ImGui::Checkbox(("Invert knife"), &bInvertKnife);
        ImGui::Checkbox(("Persistent kill feed"), &bPreserveKillfeed);
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImVec2 NextWindowCursor = ImVec2(ImGui::GetCursorPos().x, ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y / 2.f - 25.f);
    ImGui::BeginChild(("##TopRight"), ImVec2(ImGui::GetContentRegionAvail().x - Padding.x, ImGui::GetContentRegionAvail().y / 2.f - Padding.y - 25.f), true);
    {
        ImGui::Checkbox(("Killsay"), &bKillsay);
        if (bKillsay) {
            char killsayBuffer[32]{};
            strcpy(killsayBuffer, szKillsayBuffer.c_str());
            ImGui::InputText(("Text"), killsayBuffer, sizeof(killsayBuffer));
            szKillsayBuffer = killsayBuffer;
        }

        char bombCharBuffer[12]{};
        strcpy(bombCharBuffer, szBombBuffer.c_str());
        ImGui::InputText(("Bomb text"), bombCharBuffer, sizeof(bombCharBuffer));
        szBombBuffer = bombCharBuffer;

        ImGui::Checkbox(("Filter logs"), &bOnlyCheatlog);

        static const char* arrHitSoundOptions[] = { ("None"), ("Default"), ("Custom") };
        ImGui::Combo(("Hitsound"), &iHitSound, arrHitSoundOptions, IM_ARRAYSIZE(arrHitSoundOptions));
        if (iHitSound == 2) {

            static int soundItemCurrent1 = -1;
            static std::string soundItem;
            if (ImGui::ListBoxVector(("##soundFiles"), &soundItemCurrent1, Config2->vecSoundFileNames, 5)) {

                soundItem = Config2->vecSoundFileNames[soundItemCurrent1];
                cfg::misc::szWavPath = std::filesystem::path(Config2->SoundPath.c_str() + soundItem).string();
            }

            if (ImGui::Button(("Refresh"), ImVec2(ImGui::GetContentRegionAvail().x, -1), true, true))
                Config2->RefreshSounds();
        }

        ImGui::Checkbox(("Radio"), &bEnableRadio);
        ImGui::Keybind(("##iRadioMuteHotKey"), &iRadioMuteHotKey);

        if (bEnableRadio) {
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::SliderFloat(("Volume"), &flRadioVolume, 0.f, 100.f, ("%.1f%%"));
            ImGui::ListBox(("##Radio"), &iRadioStation, arrRadios, IM_ARRAYSIZE(arrRadios));
            ImGui::PopItemWidth();
        }

        for (size_t i = 0; i < 65; i++) {
            if (playerList::arrPlayers[i].bLocalPlayer) {
                if (playerList::arrPlayers[i].teamID == TEAM_TT) {

                    static const char* pistolsT[] = { ("none"), ("glock"), ("dual beretta"), ("p250"),  ("tec9 / cz-auto"), ("desert eagle / revolver") };
                    static const char* riflesT[] = { ("none"), ("galil ar"), ("ak47"), ("sg 553") };
                    static const char* snipersT[] = { ("none"), ("ssg08"), ("awp"), ("g3sg1") };
                    static const char* equipmentsT[] = { ("kevlar + helmet"), ("zeus") };

                    static const char* grenadesT[] = { ("molotov"), ("decoy grenade"),("flashbang"),("he grenade"), ("smoke grenade") };

                    ImGui::Checkbox(("Autobuy masterswitch"), &bAutobuy);

                    ImGui::Combo(("Pistols"), &cfg::misc::iPistols, pistolsT, IM_ARRAYSIZE(pistolsT));
                    ImGui::Combo(("Snipers"), &cfg::misc::iSnipers, snipersT, IM_ARRAYSIZE(snipersT));

                    ImGui::MultiComboBox(("Equipments"), equipmentsT, bEquipments, IM_ARRAYSIZE(equipmentsT));
                    ImGui::MultiComboBox(("Grenades"), grenadesT, bGrenades, IM_ARRAYSIZE(grenadesT));
                }
                else {
                    static const char* pistolsCT[] = { ("None"), ("Usp-s / p2000"), ("Dual beretta"), ("P250"), ("Five-seven / Cz-auto"), ("Desert eagle / Revolver") };
                    static const char* riflesCT[] = { ("None"), ("Famas"), ("M4a1-s / M4a4"), ("Aug") };
                    static const char* snipersCT[] = { ("None"), ("Ssg08"), ("Awp"), ("Scar-20") };
                    static const char* equipmentsCT[] = { ("Kevlar + Helmet"), ("Zeus"), ("Defuse kit") };
                    static const char* grenadesCT[] = { ("Incendiary grenade"), ("Decoy grenade"), ("Flashbang"), ("He grenade"), ("Smoke grenade") };
                   
                    ImGui::Checkbox(("Autobuy masterswitch"), &bAutobuy);

                    ImGui::Combo(("Pistols"), &cfg::misc::iPistols, pistolsCT, IM_ARRAYSIZE(pistolsCT));
                    ImGui::Combo(("Snipers"), &cfg::misc::iSnipers, snipersCT, IM_ARRAYSIZE(snipersCT));

                    ImGui::MultiComboBox(("Equipments"), equipmentsCT, bEquipments, IM_ARRAYSIZE(equipmentsCT));
                    ImGui::MultiComboBox(("Grenades"), grenadesCT, bGrenades, IM_ARRAYSIZE(grenadesCT));
                }
                break;
            }
        }
    }
    ImGui::EndChild();

    ImGui::SetCursorPos(NextWindowCursor);
    ImGui::BeginChild("##BotRight", ImVec2(ImGui::GetContentRegionAvail().x - Padding.x, ImGui::GetContentRegionAvail().y - Padding.y - 25.f), true);
    {

    }
    ImGui::EndChild();
}

void menu::Skins(ImVec2 savedCursorPosition) {

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ImGui::GetStyle().WindowPadding.x + 1);
    menu::DrawCustomChildRounding(("##TopBar"), ImVec2(ImGui::GetContentRegionAvail().x, 80), true, 0, ImDrawCornerFlags_TopRight);
    {
        ImGui::SameLine();
        ImGui::PushFont(tabIconsPreview);
        {
            ImGui::PushStyleColor(ImGuiCol_Text, clr::text_preview);
            menu::Text("e", 1.f, true);
            ImGui::PopStyleColor();
        }
        ImGui::PopFont();
    }
    ImGui::EndChild();

    ImVec2 Padding = ImVec2(ImGui::GetStyle().WindowPadding.x, ImGui::GetStyle().WindowPadding.y);
    ImGui::SetCursorPos(ImVec2(savedCursorPosition.x, savedCursorPosition.y + 80 + ImGui::GetStyle().WindowPadding.y));

    ImVec2 TopLeftSize = ImVec2(ImGui::GetContentRegionAvail().x / 2.f - Padding.x, ImGui::GetContentRegionAvail().y - Padding.y - 25.f);
    ImGui::BeginChild("##LeftWhole", ImVec2(ImGui::GetContentRegionAvail().x / 2.f - Padding.x, ImGui::GetContentRegionAvail().y - Padding.y - 25.f), true);
    {

    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImVec2 NextWindowCursor = ImVec2(ImGui::GetCursorPos().x, ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y * 0.7f - 25.f);
    ImGui::BeginChild("##TopRight", ImVec2(ImGui::GetContentRegionAvail().x - Padding.x, ImGui::GetContentRegionAvail().y * 0.7f - Padding.y - 25.f), true);
    {

    }
    ImGui::EndChild();

    ImGui::SetCursorPos(NextWindowCursor);
    ImGui::BeginChild("##BotRight", ImVec2(ImGui::GetContentRegionAvail().x - Padding.x, ImGui::GetContentRegionAvail().y - Padding.y - 25.f), true);
    {

    }
    ImGui::EndChild();
}

void menu::Config(ImVec2 savedCursorPosition) {

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ImGui::GetStyle().WindowPadding.x + 1);
    menu::DrawCustomChildRounding(("##TopBar"), ImVec2(ImGui::GetContentRegionAvail().x, 80), true, 0, ImDrawCornerFlags_TopRight);
    {
        ImGui::SameLine();
        ImGui::PushFont(tabIconsPreview);
        {
            ImGui::PushStyleColor(ImGuiCol_Text, clr::text_preview);
            menu::Text("d", 1.f, true);
            ImGui::PopStyleColor();
        }
        ImGui::PopFont();
    }
    ImGui::EndChild();

    ImVec2 Padding = ImVec2(ImGui::GetStyle().WindowPadding.x, ImGui::GetStyle().WindowPadding.y);
    ImGui::SetCursorPos(ImVec2(savedCursorPosition.x, savedCursorPosition.y + 80 + ImGui::GetStyle().WindowPadding.y));

    ImVec2 TopLeftSize = ImVec2(ImGui::GetContentRegionAvail().x / 2.f - Padding.x, ImGui::GetContentRegionAvail().y - Padding.y - 25.f);
    ImGui::BeginChild(("##LeftWhole"), ImVec2(ImGui::GetContentRegionAvail().x / 2.f - Padding.x, ImGui::GetContentRegionAvail().y - Padding.y - 25.f), true);
    {
        static std::string selectedConfig = "";
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x); 
        if (ImGui::ListBoxVector(("##configs"), &cfg::configID, Config2->vecConfigs, 15)) {
            selectedConfig = Config2->vecConfigs[cfg::configID];
        }

        static char buf[255]{};
        ImGui::InputText(("Config name"), buf, sizeof(buf));

        if (ImGui::Button(("Refresh"), ImVec2(ImGui::GetContentRegionAvail().x, 20.f), true, true)) {
            Config2->RefreshConfigs();
        }

        ImGui::Spacing();
        if (ImGui::Button(("Save"), ImVec2(ImGui::GetContentRegionAvail().x, 20.f), true, true)) {
            bPressedSave = true;
            bWarningMethod = true;
        }

        ImGui::Spacing();
        if (ImGui::Button(("Load"), ImVec2(ImGui::GetContentRegionAvail().x, 20.f), true, true)) {
            bPressedSave = true;
            bWarningMethod = false;
        }

        ImGui::Spacing();
        if (ImGui::Button(("Create"), ImVec2(ImGui::GetContentRegionAvail().x, 20.f), true, true)) {

            Config2->Save(buf);
        }

        ImGui::Spacing();
        if (ImGui::Button(("Delete"), ImVec2(ImGui::GetContentRegionAvail().x, 20.f), true, true)) {

            Config2->DeleteConfig(selectedConfig);
            Config2->RefreshConfigs();
        }

        ImGui::Spacing();
        if (ImGui::Button(("Open config location"), ImVec2(ImGui::GetContentRegionAvail().x, 20.f), true, true)) {

            ShellExecuteA(NULL, ("open"), Config2->ConfigPath.c_str(), NULL, NULL, SW_SHOWNORMAL);;
        }
        ImGui::PopItemWidth();
    }
    ImGui::EndChild();
}

void menu::PlayerList(ImVec2 savedCursorPosition) {

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ImGui::GetStyle().WindowPadding.x + 1);
    menu::DrawCustomChildRounding(("##TopBar"), ImVec2(ImGui::GetContentRegionAvail().x, 80), true, 0, ImDrawCornerFlags_TopRight);
    {
        ImGui::SameLine();
        ImGui::PushFont(tabIconsPreview);
        {
            ImGui::PushStyleColor(ImGuiCol_Text, clr::text_preview);
            menu::Text("b", 1.f, true);
            ImGui::PopStyleColor();
        }
        ImGui::PopFont();
    }
    ImGui::EndChild();

    ImVec2 Padding = ImVec2(ImGui::GetStyle().WindowPadding.x, ImGui::GetStyle().WindowPadding.y);
    ImGui::SetCursorPos(ImVec2(savedCursorPosition.x, savedCursorPosition.y + 80 + ImGui::GetStyle().WindowPadding.y));

    ImVec2 TopLeftSize = ImVec2(ImGui::GetContentRegionAvail().x / 2.f - Padding.x, ImGui::GetContentRegionAvail().y - Padding.y - 25.f);
    ImGui::BeginChild("##LeftWhole", ImVec2(ImGui::GetContentRegionAvail().x / 2.f - Padding.x, ImGui::GetContentRegionAvail().y - Padding.y - 25.f), true);
    {

    }
    ImGui::EndChild();
}

void menu::SaveWarning(bool& saved, bool type) noexcept {

    ImGui::SetNextWindowSizeConstraints(ImVec2(180, 101), ImVec2(180, 101));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(180, 101)); 
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.f);

    ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x * 0.5f) - (180 * 0.5f), ImGui::GetIO().DisplaySize.y * 0.5f + (101 * 0.5f)));
    ImGui::Begin(("##savestuff"), NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    {
        ImGui::BeginChild(("##yes"), ImGui::GetContentRegionAvail(), true);
        {
            ImGui::SetCursorPos(ImVec2(ImGui::GetContentRegionAvail().x / 4 - 3, ImGui::GetContentRegionAvail().y / 5));
            ImGui::Text(("Are you sure?"));

            static ImVec2 buttonSize = ImVec2(ImGui::GetContentRegionAvail().x / 3, ImGui::GetContentRegionAvail().y / 2);
            if (ImGui::Button(("Yes"), buttonSize, true, false)) {
                saved = false;
                type ? Config2->Save(Config2->vecConfigs[cfg::configID]) : Config2->Load(Config2->vecConfigs[cfg::configID]);
            }
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x / 2);
            if (ImGui::Button(("No"), buttonSize, true, false))
                saved = false;
        }
        ImGui::EndChild();
    }
    ImGui::PopStyleVar(2); 
    ImGui::End();
}