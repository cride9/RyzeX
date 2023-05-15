#include "gui.h"
#include "config.h"
#include "../Entity.h"
#include "../../globals.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#pragma comment(lib, "winmm.lib")

#include "../../SDK/InputSystem.h"

ETabs selectedTab = RAGE_TAB;
EEntity selectedEsp = ENEMY;
void menu::HandleMenuElements() noexcept {

    ImGui::Begin("RyzeX", NULL, ImGuiWindowFlags_NoTitleBar);
    {
        ImGui::GetBackgroundDrawList( )->AddRectFilled( ImVec2( 0.f, 0.f ), ImGui::GetIO( ).DisplaySize, IM_COL32( 0, 0, 0, 100 ) );

        if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
            if (ImGui::IsKeyDown(ImGuiKey_S)) {
                pressedSave = true;
                warningMethod = true;
            }
        }

        if (pressedSave && cfg::configID != -1)
            SaveWarning(pressedSave, warningMethod);

        HandleLogoDrawing();

        Tabselection();

        ImGui::PushFont(childFont);

        switch (selectedTab)
        {
        case RAGE_TAB: Ragetab();
            break;
        case ANTIAIM_TAB: Antiaimtab();
            break;
        case VISUAL_TAB: Visualtab();
            break;
        case MISC_TAB: Misctab();
            break;
        case SKIN_TAB: Skintab();
            break;
		case CONFIG_TAB: ConfigTab();
			break;
        }

        ImGui::PopFont();
    }
    ImGui::End();
}

void menu::Tabselection() noexcept {

    ImGui::PushFont(tabFont);
    ImGui::BeginChild("leftchild", ImVec2(100.f, ImGui::GetContentRegionAvail().y), true, ImGuiWindowFlags_NoMove );
    {
        if (ImGui::Button("Ragebot", ImVec2(ImGui::GetContentRegionAvail().x, 50), selectedTab == RAGE_TAB))
            selectedTab = RAGE_TAB;

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
        if (ImGui::Button("Antiaim", ImVec2(ImGui::GetContentRegionAvail().x, 50), selectedTab == ANTIAIM_TAB))
            selectedTab = ANTIAIM_TAB;

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
        if (ImGui::Button("Visual", ImVec2(ImGui::GetContentRegionAvail().x, 50), selectedTab == VISUAL_TAB))
            selectedTab = VISUAL_TAB;

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
        if (ImGui::Button("Misc", ImVec2(ImGui::GetContentRegionAvail().x, 50), selectedTab == MISC_TAB))
            selectedTab = MISC_TAB;

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
        if (ImGui::Button("Skins", ImVec2(ImGui::GetContentRegionAvail().x, 50), selectedTab == SKIN_TAB))
            selectedTab = SKIN_TAB;

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
		if (ImGui::Button("Config", ImVec2(ImGui::GetContentRegionAvail().x, 50), selectedTab == CONFIG_TAB))
			selectedTab = CONFIG_TAB;
    }
    ImGui::EndChild();
    ImGui::PopFont();

    ImGui::SameLine();
}

void menu::Ragetab() noexcept {

    using namespace cfg::rage;
    ImVec2 savedCursorPos = ImVec2(ImGui::GetCursorPos().x, ImGui::GetCursorPos().y + ImGui::GetContentRegionAvail().y / 2 + ImGui::GetStyle().WindowPadding.y + 2);
	static const char* autoStopPhases[] = { "Disabled", "Slight", "Normal", "Aggressive" };
	static const char* aimbotTargetSelectionList[] = { "Health", "Field of view" };

    ImGui::BeginChild("leftside", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y), true, ImGuiWindowFlags_NoMove );
    {
        ImGui::Checkbox("Enable", &enable);
		ImGui::Keybind("##ragekey", &ragebotbind);
		ImGui::Checkbox("Silent", &bSilentAim);
		ImGui::Combo("Target selection", &aimbotTargetSelection, aimbotTargetSelectionList, IM_ARRAYSIZE(aimbotTargetSelectionList));
		ImGui::SliderInt("Aimbot fov", &iAimbotFov, 1.f, 180.f);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(219.f / 255.f, 216.f / 255.f, 0.f, 1.f));
        ImGui::Checkbox("Doubletap", &doubletap);
        ImGui::PopStyleColor();
        ImGui::Keybind("DoubletapKey", &doubletapkey);
        ImGui::Checkbox("Anti-aim correction", &resolver);
        ImGui::Checkbox( "Backtrack", &m_bEnableBacktrack );

        ImGui::Checkbox("Force baim", &forceBaim);
        ImGui::Keybind("##forcebaimkey", &forceBaimKey);
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("rightside", ImGui::GetContentRegionAvail(), true, ImGuiWindowFlags_NoMove );
    {
        static const char* item[] = { "Auto", "Scout", "Awp", "Pistol", "Heavy pistol", "Other" };
        static int selectedWeapon = 0;
        ImGui::Combo("Weapon Config", &selectedWeapon, item, IM_ARRAYSIZE(item));

        static const char* items[] = { "Head", "Upper chest", "Lower chest", "Stomach", "Arms", "Legs" };

        static auto buildTabForWeapon = [](bool* hitboxes, bool* multiHitboxes, int& hitChance, int& minDmg, int& headPoints, int& bodyPoints, int& overridedmg, EWEAPON weapon, bool* safePoints) {

            ImGui::MultiComboBox("Hitboxes", items, hitboxes, IM_ARRAYSIZE(items));

            ImGui::SliderInt("Hitchance", &hitChance, 0, 100);
            ImGui::SliderInt("Minimum Damage", &minDmg, 0, 110);

            ImGui::MultiComboBox("Multipoint", items, multiHitboxes, IM_ARRAYSIZE(items));

            ImGui::SliderInt("Head Pointscale", &headPoints, 0, 100);
            ImGui::SliderInt("Body Pointscale", &bodyPoints, 0, 100);

			ImGui::MultiComboBox("Safe point", items, safePoints, IM_ARRAYSIZE(items));
			ImGui::Checkbox("Force safe", &forceSafePoint[weapon]);

            if (weapon < 3)
                ImGui::Checkbox("Auto-Scope", &autoscope[weapon]);

            ImGui::Checkbox( "Auto Stop", &autostop[ weapon ] );
            if ( autostop[ weapon ] ) {
                ImGui::Checkbox( "Auto Stop In Air", &m_bAutoStopInAir[ weapon ] );
                ImGui::Checkbox( "Between shots", &betweenshots[ weapon ] );
                ImGui::Combo( "Predicted stop", &autostopAggressiveness[ weapon ], autoStopPhases, IM_ARRAYSIZE( autoStopPhases ) );
            }

            ImGui::SliderInt("Damage override", &overridedmg, 0, 110);
            ImGui::Keybind("damageoverridebind", &overrideBind);
        };

        switch (selectedWeapon)
        {
        case AUTO: buildTabForWeapon(autoHitboxes, autoMultiHitboxes, autoHitchance, autoMindmg, autoHeadPoints, autoBodyPoints, autoOverride, AUTO, autoSafeHitboxes);
            break;
        case SCOUT:buildTabForWeapon(scoutHitboxes, scoutMultiHitboxes, scoutHitchance, scoutMindmg, scoutHeadPoints, scoutBodyPoints, scoutOverride, SCOUT, scoutSafeHitboxes);
            break;
        case AWP: buildTabForWeapon(awpHitboxes, awpMultiHitboxes, awpHitchance, awpMindmg, awpHeadPoints, awpBodyPoints, awpOverride, AWP, awpSafeHitboxes);
            break;
        case PISTOL: buildTabForWeapon(pistolHitboxes, pistolMultiHitboxes, pistolHitchance, pistolMindmg, pistolHeadPoints, pistolBodyPoints, pistolOverride, PISTOL, pistolSafeHitboxes);
            break;
        case HEAVY_PISTOL: buildTabForWeapon(heavypistolHitboxes, heavypistolMultiHitboxes, heavypistolHitchance, heavypistolMindmg, heavypistolHeadPoints, heavypistolBodyPoints, heavypistolOverride, HEAVY_PISTOL, heavypistolSafeHitboxes);
            break;
        case OTHER: buildTabForWeapon(etcHitboxes, etcMultiHitboxes, etcHitchance, etcMindmg, etcHeadPoints, etcBodyPoints, etcOverride, OTHER, etcSafeHitboxes);
            break;
        }
    }
    ImGui::EndChild();
}

void menu::Antiaimtab() noexcept {

    using namespace cfg::antiaim;

    static const char* yawList[] = { "Forward", "Backward" };
    static const char* pitchList[] = { "Off", "Up", "Zero" ,"Down" };
    static const char* desyncList[] = { "Off", "Static", "Extended", "Jitter", "Flick"};
    static const char* yawBaseList[] = { "Local view", "At target" };
    static const char* fakelagTypeList[] = { "Normal", "Adaptive", "Jitter" };
    static const char* yawModifierList[] = { "Off", "Jitter", "Random" };
	static const char* freestandList[] = { "Off", "Distance", "Smart" };

    ImGui::BeginChild("LeftChild", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y), true, ImGuiWindowFlags_NoMove );
    {
        ImGui::Checkbox("Enabled", &bEnabled);
        ImGui::Combo("Pitch", &iPitch, pitchList, IM_ARRAYSIZE(pitchList));
        ImGui::Combo("Yaw base", &iYawBase, yawBaseList, IM_ARRAYSIZE(yawBaseList));
        ImGui::Combo("Yaw", &iYaw, yawList, IM_ARRAYSIZE(yawList));
        ImGui::Combo("Modifier", &modifier, yawModifierList, IM_ARRAYSIZE(yawModifierList));
        if (modifier != 0)
            ImGui::SliderInt("Modifier value", &jittervalue, 0, 90);
        ImGui::Combo("Lower body yaw target", &iDesyncType, desyncList, IM_ARRAYSIZE(desyncList));
        if (iDesyncType != 0) {

			ImGui::SliderFloat("Body lean", &cfg::antiaim::bodyLean[0], -58.f, 58.f, "%.f");
			ImGui::SliderFloat("Body lean inverted", &cfg::antiaim::bodyLean[1], -58.f, 58.f, "%.f");

			if (iDesyncType == 4) {
				ImGui::SliderFloat("Yaw desync angle", &flDesyncValue, 0.f, 58.f, "%.f");
				ImGui::SliderInt("Static offset", &cfg::antiaim::iFlickOffset, 0, 180.f);
				ImGui::SliderInt("Switch angle on tick", &cfg::antiaim::flickAngleSwitch, 0, cfg::antiaim::fakelagmax);
			}
			else {
				if (iDesyncType == 2)
					ImGui::Checkbox("Sway LBY", &m_bSwayDesync);

				ImGui::SliderFloat("Yaw desync angle", &flDesyncValue, 0.f, 58.f, "%.f");
				ImGui::Keybind("invertButton", &iInverterBind);
				ImGui::Combo("Yaw target modifier", &desyncModifier, yawModifierList, IM_ARRAYSIZE(yawModifierList));
				if (desyncModifier != 0)
					ImGui::SliderInt("Modifier value ##2", &desyncModifierValue, 0, 58);
			}
        }
		ImGui::Checkbox("Invert on shoot", &bInvertOnShoot);
		ImGui::Combo("Fresstanding", &freestand, freestandList, IM_ARRAYSIZE(freestandList));
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImVec2 savedPosition = ImVec2(ImGui::GetCursorPos().x, ImGui::GetCursorPos().y + ImGui::GetContentRegionAvail().y / 2 + ImGui::GetStyle().WindowPadding.y + 2.f);

    ImGui::BeginChild("righttop", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 2), true, ImGuiWindowFlags_NoMove );
    {
        fakelagmin = std::clamp(fakelagmin, 0, fakelag);
        fakelagmax = std::clamp(fakelagmax, fakelagmin, fakelag);
        ImGui::Checkbox("Enabled", &enableFakelag);
        ImGui::Combo("Choke type", &fakelagType, fakelagTypeList, IM_ARRAYSIZE(fakelagTypeList));
        ImGui::SliderInt("Base amount", &fakelag, 0, 14);
        ImGui::SliderInt("Minimum choke", &fakelagmin, 0, fakelag);
        ImGui::SliderInt("Maximum choke", &fakelagmax, fakelagmin, fakelag);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(219.f / 255.f, 216.f / 255.f, 0.f, 1.f));
        ImGui::Checkbox("Break lagcompenstaion", &defensive);
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();

    ImGui::SetCursorPos(savedPosition);

    ImGui::BeginChild("rightbot", ImGui::GetContentRegionAvail(), true, ImGuiWindowFlags_NoMove );
    {
        ImGui::Checkbox("Slow motion", &fakewalkenable);
        ImGui::Keybind("slowmotionKey", &fakewalkKey);
        ImGui::SliderFloat("Slow motion speed", &fakewalk, 0.f, 100.f, "%.f");
		ImGui::Checkbox("Leg breaker", &cfg::antiaim::bSlideWalk);

        ImGui::Checkbox("Fake duck", &fakeduck);
        ImGui::Keybind("fakeduckBind", &fakeduckbind);

        ImGui::Checkbox("Auto peek", &idealTick);
        ImGui::Keybind("autopeekBind", &idealTickBind);

        if (idealTick) {
            ImGui::Checkbox("Visualize position", &cfg::model::localIdealTick);
            ImGui::ColorEdit4("##localIdealTickColor", cfg::model::localIdealTickColor);
        }
    }
    ImGui::EndChild();
}

void menu::Visualtab() noexcept {

    float buttonSize = (ImGui::GetContentRegionAvail().x / 5) - (2 * 4);
    ImVec2 savedPosition = ImVec2(ImGui::GetCursorPos().x, ImGui::GetCursorPos().y + 30.f + ImGui::GetStyle().WindowPadding.y + 2.f);
    static const char* chamsType[] = { "Default", "Flat" };
    static const char* glowType[] = { "Glow", "Thin glow", "Animated" };
    static const char* enemyTypes[] = { "Glow", "Thin glow", "Animated", "Backtrack" };
    static const char* localTypes[] = { "Glow", "Thin glow", "Animated", "Desync" };
    static const char* allType[] = { "Default", "Flat", "Glow", "Thin glow", "Animated" };
    static const char* m_szHitsound[] = { "None", "Default", "Custom" };

    ImGui::BeginChild("selectPlayer", ImVec2(ImGui::GetContentRegionAvail().x, 30.f), true, ImGuiWindowFlags_NoMove );
    {
        if (ImGui::Button("Enemy", ImVec2(buttonSize, 20), selectedEsp == ENEMY))
            selectedEsp = ENEMY;

        ImGui::SameLine();
        if (ImGui::Button("Teammate", ImVec2(buttonSize, 20), selectedEsp == TEAM))
            selectedEsp = TEAM;

        ImGui::SameLine();
        if (ImGui::Button("Local", ImVec2(buttonSize, 20), selectedEsp == LOCAL))
            selectedEsp = LOCAL;

        ImGui::SameLine();
        if (ImGui::Button("World", ImVec2(buttonSize, 20), selectedEsp == WORLD))
            selectedEsp = WORLD;

        ImGui::SameLine();
        if (ImGui::Button("Viewmodel", ImVec2(buttonSize, 20), selectedEsp == VIEWMODEL))
            selectedEsp = VIEWMODEL;
    }
    ImGui::EndChild();

    ImGui::SetCursorPos(savedPosition);


    switch (selectedEsp)
    {
    using namespace cfg::visual;
    case ENEMY:
        HandleVisualTypeGeneration(
            bEnable[ENEMY],
            bName[ENEMY],
            flNameColor[ENEMY],
            bBox[ENEMY],
            flBoxColor[ENEMY],
            bHealth[ENEMY],
            flHealthColorStart[ENEMY],
			flHealthColorEnd[ENEMY],
            bGlow[ENEMY],
            flGlowColor[ENEMY],
            bArmor[ENEMY],
            flArmorColor[ENEMY],
            bAmmo[ENEMY],
            flAmmoColor[ENEMY],
            bWeapon[ENEMY],
            flWeaponColor[ENEMY],
			bFlags[ENEMY],
			flFlagsColor[ENEMY],
			bSkeleton[ENEMY],
			flSkeletonColor[ENEMY]);
        break;

    case TEAM:
        HandleVisualTypeGeneration(
			bEnable[TEAM],
			bName[TEAM],
			flNameColor[TEAM],
			bBox[TEAM],
			flBoxColor[TEAM],
			bHealth[TEAM],
			flHealthColorStart[TEAM],
			flHealthColorEnd[TEAM],
			bGlow[TEAM],
			flGlowColor[TEAM],
			bArmor[TEAM],
			flArmorColor[TEAM],
			bAmmo[TEAM],
			flAmmoColor[TEAM],
			bWeapon[TEAM],
			flWeaponColor[TEAM],
			bFlags[TEAM],
			flFlagsColor[TEAM],
			bSkeleton[TEAM],
			flSkeletonColor[TEAM]);
        break;

    case LOCAL:
        HandleVisualTypeGeneration(
			bEnable[LOCAL],
			bName[LOCAL],
			flNameColor[LOCAL],
			bBox[LOCAL],
			flBoxColor[LOCAL],
			bHealth[LOCAL],
			flHealthColorStart[LOCAL],
			flHealthColorEnd[LOCAL],
			bGlow[LOCAL],
			flGlowColor[LOCAL],
			bArmor[LOCAL],
			flArmorColor[LOCAL],
			bAmmo[LOCAL],
			flAmmoColor[LOCAL],
			bWeapon[LOCAL],
			flWeaponColor[LOCAL],
			bFlags[LOCAL],
			flFlagsColor[LOCAL],
			bSkeleton[LOCAL],
			flSkeletonColor[LOCAL]);
        break;

    case WORLD:
        using namespace cfg::misc;

        ImGui::BeginChild("left", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y), true, ImGuiWindowFlags_NoMove );
        {
            ImGui::Checkbox("Thirdperson", &thirdperson);
            ImGui::Keybind("##tpKey", &thirdpersonbind);

            ImGui::Checkbox("Nightmode", &nightmode);
            ImGui::ColorEdit4("nightmodecolor", nightmodeColor);

			static const char* szSkyboxes[] = {
				"Baggage", 
				"Tibet",
				"Vietnam", 
				"Lunacy",
				"Embassy",
				"Italy", 
				"Jungle",
				"Office", 
				"Daylight1", 
				"Daylight2", 
				"Daylight3",
				"Daylight4", 
				"Day", 
				"Nuke",
				"Dust",
				"Venice",
				"Cloudy",
				"Night1", 
				"Night2", 
				"Vertigo",
				"VertigoHdr",
				"SkyDust", 
				"Aztec"
			};

			ImGui::Combo("Skybox", &cfg::misc::iSkybox, szSkyboxes, IM_ARRAYSIZE(szSkyboxes));

            ImGui::Checkbox("Preserve killfeed", &preserveKillfeed);

            ImGui::Checkbox("Aspect ratio", &aspectRatio);
            ImGui::SliderInt("Ratio", &aspectRatioValue, 0, 100);

            ImGui::SliderInt("Viewmodel fov", &viewmodelFov, 45, 140);
            ImGui::SliderInt("Debug fov", &fov, 40, 140);

            ImGui::Checkbox("Always draw viewmodel", &drawViewmodelOnScope);

            ImGui::Checkbox("Remove smoke", &removals[0]);
            ImGui::Checkbox("Remove flash", &removals[1]);
            ImGui::Checkbox("Remove recoil", &removals[2]);
            ImGui::Checkbox("Remove zoom", &removals[3]);
            if (removals[3]) {
                ImGui::ColorEdit4("##scope stuff", scopeColor);
                ImGui::SliderFloat("Scope length", &scopeLength, 0.f, 100.f, "%.f");
            }
            ImGui::Checkbox("Remove post processing", &removals[4]);
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("right", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true, ImGuiWindowFlags_NoMove );
        {
            ImGui::Checkbox("Show impact", &bulletImpact);
			ImGui::ColorEdit4("##impactColor1", impactColor[0], true);
			ImGui::ColorEdit4("##impactColor2", impactColor[1]);
            ImGui::Checkbox( "Bullet tracer", &bulletTracer );
			ImGui::ColorEdit4("##tracerColor", bulletTracerColor);
			ImGui::Checkbox("Draw capsule", &bDrawCapsule);
			ImGui::ColorEdit4("##capsuleCOlor", flDrawCapsuleColor, true);
			ImGui::ColorEdit4("##capsuleColorHit", flDrawCapsuleColorHit);
			ImGui::Checkbox("World hitmarker", &cfg::misc::bWorldCrosshair);
			ImGui::ColorEdit4("##hitmarkercolor", cfg::misc::flWorldCrosshairColor);
            ImGui::Combo( "Hitsound", &m_iHitSound, m_szHitsound, IM_ARRAYSIZE( m_szHitsound ) );
            static int soundItemCurrent1 = -1;
            static std::string soundItem;
            bool bOpen = true;
            if ( m_iHitSound == 2 ) {
                
                ImGui::PushStyleVar( ImGuiStyleVar_FrameBorderSize, 2.f );
                if ( ImGui::Button( "Manage custom sounds" , ImVec2( 150, 17 ) ) )
                    ImGui::OpenPopup( "##sndManager" );
                ImGui::PopStyleVar( );
            }

            ImGui::SetNextWindowSizeConstraints( ImVec2( 0, 0 ), ImVec2( 200, 140) );
            ImGui::PushStyleVar( ImGuiStyleVar_WindowMinSize, ImVec2( 400, 160 ) );
            if ( ImGui::BeginPopupModal( "##sndManager", &bOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar ) )
            {
                ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 0.07f, 0.07f, 0.07f, 1.f ) );
                ImGui::BeginChild( "visuals.sndManagerPopup", ImVec2( 0, 0 ), true );
                {
                    ImGui::Dummy( ImVec2( 0.f, 5.f ) );

                    ImGui::Columns( 2, "##SOUNDS", false );
                    {
                        if ( !Config2->vecSoundFileNames.empty( ) ) {

                            ImGui::PushItemWidth( -1 );
                            if ( ImGui::ListBoxVector( "##soundFiles", &soundItemCurrent1, Config2->vecSoundFileNames, 7 ) ) {
                                soundItem = Config2->vecSoundFileNames[ soundItemCurrent1 ];
                                cfg::misc::m_szWavPath = std::filesystem::path( Config2->SoundPath.c_str() + soundItem ).string( );
                            }
                            ImGui::PopItemWidth( );
                        }
                        else {
                            ImGui::SetCursorPosX( ImGui::GetCursorPosX( ) + ( ImGui::GetColumnWidth( ) / 2 ) - ( ImGui::CalcTextSize( "No sounds" ).x / 2 ) );
                            ImGui::Text( "No sounds" );
                        }
                    }
                    ImGui::NextColumn( );
                    {
                        ImGui::PushItemWidth( -1 );
                        ImGui::PushStyleVar( ImGuiStyleVar_FrameBorderSize, 2.f );

                        if ( ImGui::Button( "Play", ImVec2( -1, 18 ) ) && soundItemCurrent1 >= 0 && !cfg::misc::m_szWavPath.empty( ) ) {

                            PlaySoundA( cfg::misc::m_szWavPath.c_str( ), NULL, SND_FILENAME | SND_ASYNC );
                        }

                        ImGui::Spacing( );
                        if ( ImGui::Button( "Refresh", ImVec2( -1, 18 ) ) )
                            Config2->RefreshSounds( );

                        ImGui::Spacing( );
                        if ( ImGui::Button( "Remove", ImVec2( -1, 18 ) ) ) {
                            std::remove( cfg::misc::m_szWavPath.c_str( ) );
                            Config2->RefreshSounds( );
                            soundItemCurrent1 = -1;
                            cfg::misc::m_szWavPath = "";
                        }

                        ImGui::Spacing( );
                        if ( ImGui::Button( "Folder", ImVec2( -1, 18 ) ) )
                            ShellExecuteA( NULL, "open", Config2->SoundPath.c_str( ), NULL, NULL, SW_SHOWNORMAL );

                        ImGui::Spacing( );
                        if ( ImGui::Button( "Close", ImVec2( -1, 18 ) ) )
                            ImGui::CloseCurrentPopup( );

                        ImGui::PopStyleVar( );
                        ImGui::PopItemWidth( );
                    }

                    ImGui::Columns( 1 );
                    ImGui::EndChild( );
                }
                ImGui::PopStyleColor( 1 );
                ImGui::EndPopup( );
            }
            ImGui::PopStyleVar( );

            if (m_iHitSound > 0)
				ImGui::SliderFloat("Hitsound volume", &m_flHitSoundVolume, 0.f, 100.f, "%.f");
            ImGui::Checkbox("Paper mode", &cfg::model::paperMode);
            ImGui::Checkbox("Keybind list", &keyBindList);
        }
        ImGui::EndChild();

        return;
    case VIEWMODEL:
        using namespace cfg::model;
        ImGui::BeginChild("left", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y), true, ImGuiWindowFlags_NoMove );
        {
            ImGui::Combo("Viewmodel model", &viewmodelType, chamsType, IM_ARRAYSIZE(chamsType));
            ImGui::Checkbox("Enabled", &viewmodel);
            ImGui::ColorEdit4("##viewmodelColor", viewmodelColor);
            ImGui::Checkbox("Wireframe", &viewmodelXhair);

            static int selectedviewmodel = 0;
            ImGui::Combo("Overlay type", &selectedviewmodel, glowType, IM_ARRAYSIZE(glowType));

            switch (selectedviewmodel) {

            case 0:
                ImGui::Checkbox("Enabled ##6", &viewmodelOverlay);
                ImGui::ColorEdit4("##localOverlayColor", viewmodelOverlayColor);
                ImGui::Checkbox("Wireframe ##1", &viewmodelOverlayXhair);
                break;

            case 1:
                ImGui::Checkbox("Enabled ##6", &viewmodelThinOverlay);
                ImGui::ColorEdit4("##viewmodelThinOverlayColor", viewmodelThinOverlayColor);
                ImGui::Checkbox("Wireframe ##1", &viewmodelThinOverlayXhair);
                break;

            case 2:
                ImGui::Checkbox("Enabled ##6", &viewmodelAnimOverlay);
                ImGui::ColorEdit4("##viewmodelAnimOverlayColor", viewmodelAnimOverlayColor);
                ImGui::Checkbox("Wireframe ##1", &viewmodelAnimOverlayXhair);
                break;
            }
        }
        ImGui::EndChild();
        break;
    }
    ImGui::SameLine();

    ImGui::BeginChild("modelchild", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true, ImGuiWindowFlags_NoMove );
    {
        using namespace cfg::model;

		static int selectedModelType = 0;
		static int selectedOverlayType = 0;

        switch (selectedEsp)
        {
        case ENEMY:

			if (ImGui::Button("Model", ImVec2(ImGui::GetContentRegionAvail().x / 3, 15.f), selectedModelType == 0)) 
				selectedModelType = 0;
			ImGui::SameLine();
			if (ImGui::Button("Overlays", ImVec2(ImGui::GetContentRegionAvail().x / 2, 15.f), selectedModelType == 1))
				selectedModelType = 1;
			ImGui::SameLine();
			if (ImGui::Button("Attachments", ImVec2(ImGui::GetContentRegionAvail().x, 15.f), selectedModelType == 2))
				selectedModelType = 2;

			if (selectedModelType == 0) {

				ImGui::Combo("Model##1", &enemyType, chamsType, IM_ARRAYSIZE(chamsType));

				ImGui::Checkbox("Player", &enemy);
				ImGui::ColorEdit4("##enemycolor", enemyColor);
				ImGui::Checkbox("Wireframe", &enemyXhair);

				ImGui::Checkbox("Player behind wall", &enemyXQZ);
				ImGui::ColorEdit4("##enemycolorXQZ", enemyXQZColor);
				ImGui::Checkbox("Wireframe behind wall", &enemyXQZXhair);

				ImGui::Checkbox("Lagcompensation", &enemyBTEnable);
				ImGui::ColorEdit4("##enemyBTColor", enemyBTColor);
				ImGui::Combo("Material", &enemyBTType, allType, IM_ARRAYSIZE(allType));
				ImGui::Checkbox("Wireframe ##2", &enemyBTXhair);
			}
			else if (selectedModelType == 1) {

				if (ImGui::Button("Overlay", ImVec2(ImGui::GetContentRegionAvail().x / 3.f, 15.f), selectedOverlayType == 0))
					selectedOverlayType = 0;

				ImGui::SameLine();
				if (ImGui::Button("Outline", ImVec2(ImGui::GetContentRegionAvail().x / 2, 15.f), selectedOverlayType == 1))
					selectedOverlayType = 1;

				ImGui::SameLine();
				if (ImGui::Button("Animated", ImVec2(ImGui::GetContentRegionAvail().x, 15.f), selectedOverlayType == 2))
					selectedOverlayType = 2;

				switch (selectedOverlayType) {
				case 0:
					ImGui::Checkbox("Enabled", &enemyOverlay);
					ImGui::ColorEdit4("##enemyOverlayColor", enemyOverlayColor);
					ImGui::Checkbox("Wireframe ##2", &enemyOverlayXhair);

					ImGui::Checkbox("Behind wall", &enemyOverlayXQZ);
					ImGui::ColorEdit4("##enemyOverlayXQZColor", enemyOverlayXQZColor);
					ImGui::Checkbox("Wireframe behind wall ##2", &enemyOverlayXQZXhair);
					break;

				case 1:
					ImGui::Checkbox("Enabled", &enemyThinOverlay);
					ImGui::ColorEdit4("##enemyThinOverlayColor", enemyThinOverlayColor);
					ImGui::Checkbox("Wireframe ##2", &enemyThinOverlayXhair);

					ImGui::Checkbox("Behind wall", &enemyThinOverlayXQZ);
					ImGui::ColorEdit4("##enemyThinOverlayXQZColor", enemyThinOverlayXQZColor);
					ImGui::Checkbox("Wireframe behind wall ##2", &enemyThinOverlayXQZXhair);
					break;

				case 2:
					ImGui::Checkbox("Enabled", &enemyAnimOverlay);
					ImGui::ColorEdit4("##enemyThinOverlayColor", enemyAnimOverlayColor);
					ImGui::Checkbox("Wireframe ##2", &enemyAnimOverlayXhair);

					ImGui::Checkbox("Behind wall", &enemyAnimOverlayXQZ);
					ImGui::ColorEdit4("##enemyThinOverlayXQZColor", enemyAnimOverlayXQZColor);
					ImGui::Checkbox("Wireframe behind wall ##2", &enemyAnimOverlayXQZXhair);
					break;
				}
			}
			else {

				ImGui::Combo("Model##3", &attachmentChamsMaterial[ENEMY], chamsType, IM_ARRAYSIZE(chamsType));
				ImGui::Checkbox("Enabled", &attachmentChams[ENEMY]);
				ImGui::ColorEdit4("##attachmentColor", attachmentChamsColor[ENEMY]);
				ImGui::Checkbox("Wireframe##3", &attachmentChamsXhair[ENEMY]);

				if (ImGui::Button("Overlay", ImVec2(ImGui::GetContentRegionAvail().x / 3.f, 15.f), selectedOverlayType == 0))
					selectedOverlayType = 0;

				ImGui::SameLine();
				if (ImGui::Button("Outline", ImVec2(ImGui::GetContentRegionAvail().x / 2, 15.f), selectedOverlayType == 1))
					selectedOverlayType = 1;

				ImGui::SameLine();
				if (ImGui::Button("Animated", ImVec2(ImGui::GetContentRegionAvail().x, 15.f), selectedOverlayType == 2))
					selectedOverlayType = 2;

				switch (selectedOverlayType) {
				case 0:
					ImGui::Checkbox("Enabled##2", &attachmentOverlay[ENEMY]);
					ImGui::ColorEdit4("##attachmentOverlayColor", attachmentOverlayColor[ENEMY]);
					ImGui::Checkbox("Wireframe##4", &attachmentOverlayXhair[ENEMY]);
					break;

				case 1:
					ImGui::Checkbox("Enabled##2", &attachmentThinOverlay[ENEMY]);
					ImGui::ColorEdit4("##attachmentOverlayColor", attachmentThinOverlayColor[ENEMY]);
					ImGui::Checkbox("Wireframe##4", &attachmentThinOverlayXhair[ENEMY]);
					break;

				case 2:
					ImGui::Checkbox("Enabled##2", &attachmentAnimatedOverlay[ENEMY]);
					ImGui::ColorEdit4("##attachmentOverlayColor", attachmentAnimatedOverlayColor[ENEMY]);
					ImGui::Checkbox("Wireframe##4", &attachmentAnimatedOverlayXhair[ENEMY]);
					break;
				}
			}

            break;
        case TEAM:
			if (ImGui::Button("Model", ImVec2(ImGui::GetContentRegionAvail().x / 3, 15.f), selectedModelType == 0))
				selectedModelType = 0;
			ImGui::SameLine();
			if (ImGui::Button("Overlays", ImVec2(ImGui::GetContentRegionAvail().x / 2, 15.f), selectedModelType == 1))
				selectedModelType = 1;
			ImGui::SameLine();
			if (ImGui::Button("Attachments", ImVec2(ImGui::GetContentRegionAvail().x, 15.f), selectedModelType == 2))
				selectedModelType = 2;

			if (selectedModelType == 0) {

				ImGui::Combo("Model##2", &teamType, chamsType, IM_ARRAYSIZE(chamsType));

				ImGui::Checkbox("Player", &team);
				ImGui::ColorEdit4("##teamcolor", teamColor);
				ImGui::Checkbox("Wireframe", &teamXhair);

				ImGui::Checkbox("Player behind wall", &teamXQZ);
				ImGui::ColorEdit4("##teamcolorXQZ", teamXQZColor);
				ImGui::Checkbox("Wireframe behind wall", &teamXQZXhair);
			}
			else if (selectedModelType == 1) {

				if (ImGui::Button("Overlay", ImVec2(ImGui::GetContentRegionAvail().x / 3.f, 15.f), selectedOverlayType == 0))
					selectedOverlayType = 0;

				ImGui::SameLine();
				if (ImGui::Button("Outline", ImVec2(ImGui::GetContentRegionAvail().x / 2, 15.f), selectedOverlayType == 1))
					selectedOverlayType = 1;

				ImGui::SameLine();
				if (ImGui::Button("Animated", ImVec2(ImGui::GetContentRegionAvail().x, 15.f), selectedOverlayType == 2))
					selectedOverlayType = 2;

				switch (selectedOverlayType) {
				case 0:
					ImGui::Checkbox("Enabled", &teamOverlay);
					ImGui::ColorEdit4("##teamOverlayColor", teamOverlayColor);
					ImGui::Checkbox("Wireframe ##2", &teamOverlayXhair);

					ImGui::Checkbox("Behind wall", &teamOverlayXQZ);
					ImGui::ColorEdit4("##teamOverlayXQZColor", teamOverlayXQZColor);
					ImGui::Checkbox("Wireframe behind wall ##2", &teamOverlayXQZXhair);
					break;

				case 1:
					ImGui::Checkbox("Enabled", &teamThinOverlay);
					ImGui::ColorEdit4("##teamThinOverlayColor", teamThinOverlayColor);
					ImGui::Checkbox("Wireframe ##2", &teamThinOverlayXhair);

					ImGui::Checkbox("Behind wall", &teamThinOverlayXQZ);
					ImGui::ColorEdit4("##teamThinOverlayXQZColor", teamThinOverlayXQZColor);
					ImGui::Checkbox("Wireframe behind wall ##2", &teamThinOverlayXQZXhair);
					break;

				case 2:
					ImGui::Checkbox("Enabled", &teamAnimOverlay);
					ImGui::ColorEdit4("##teamThinOverlayColor", teamAnimOverlayColor);
					ImGui::Checkbox("Wireframe ##2", &teamAnimOverlayXhair);

					ImGui::Checkbox("Behind wall", &teamAnimOverlayXQZ);
					ImGui::ColorEdit4("##teamThinOverlayXQZColor", teamAnimOverlayXQZColor);
					ImGui::Checkbox("Wireframe behind wall ##2", &teamAnimOverlayXQZXhair);
					break;
				}
			}
			else {

				ImGui::Combo("Model##3", &attachmentChamsMaterial[TEAM], chamsType, IM_ARRAYSIZE(chamsType));
				ImGui::Checkbox("Enabled", &attachmentChams[TEAM]);
				ImGui::ColorEdit4("##attachmentColor", attachmentChamsColor[TEAM]);
				ImGui::Checkbox("Wireframe##3", &attachmentChamsXhair[TEAM]);

				if (ImGui::Button("Overlay", ImVec2(ImGui::GetContentRegionAvail().x / 3.f, 15.f), selectedOverlayType == 0))
					selectedOverlayType = 0;

				ImGui::SameLine();
				if (ImGui::Button("Outline", ImVec2(ImGui::GetContentRegionAvail().x / 2, 15.f), selectedOverlayType == 1))
					selectedOverlayType = 1;

				ImGui::SameLine();
				if (ImGui::Button("Animated", ImVec2(ImGui::GetContentRegionAvail().x, 15.f), selectedOverlayType == 2))
					selectedOverlayType = 2;

				switch (selectedOverlayType) {
				case 0:
					ImGui::Checkbox("Enabled##2", &attachmentOverlay[TEAM]);
					ImGui::ColorEdit4("##attachmentOverlayColor", attachmentOverlayColor[TEAM]);
					ImGui::Checkbox("Wireframe##4", &attachmentOverlayXhair[TEAM]);
					break;

				case 1:
					ImGui::Checkbox("Enabled##2", &attachmentThinOverlay[TEAM]);
					ImGui::ColorEdit4("##attachmentOverlayColor", attachmentThinOverlayColor[TEAM]);
					ImGui::Checkbox("Wireframe##4", &attachmentThinOverlayXhair[TEAM]);
					break;

				case 2:
					ImGui::Checkbox("Enabled##2", &attachmentAnimatedOverlay[TEAM]);
					ImGui::ColorEdit4("##attachmentOverlayColor", attachmentAnimatedOverlayColor[TEAM]);
					ImGui::Checkbox("Wireframe##4", &attachmentAnimatedOverlayXhair[TEAM]);
					break;
				}
			}
            break;
        case LOCAL:

			if (ImGui::Button("Model", ImVec2(ImGui::GetContentRegionAvail().x / 3, 15.f), selectedModelType == 0))
				selectedModelType = 0;
			ImGui::SameLine();
			if (ImGui::Button("Overlays", ImVec2(ImGui::GetContentRegionAvail().x / 2, 15.f), selectedModelType == 1))
				selectedModelType = 1;
			ImGui::SameLine();
			if (ImGui::Button("Attachments", ImVec2(ImGui::GetContentRegionAvail().x, 15.f), selectedModelType == 2))
				selectedModelType = 2;

			if (selectedModelType == 0) {

				ImGui::Combo("Model##3", &localType, chamsType, IM_ARRAYSIZE(chamsType));

				ImGui::Checkbox("Player", &local);
				ImGui::ColorEdit4("##localcolor", localColor);
				ImGui::Checkbox("Wireframe", &localXhair);

				ImGui::Checkbox("Player behind wall", &localXQZ);
				ImGui::ColorEdit4("##localcolorXQZ", localXQZColor);
				ImGui::Checkbox("Wireframe behind wall", &localXQZXhair);

				ImGui::Checkbox("Desync", &localDesync);
				ImGui::ColorEdit4("##localDesync", localDesyncColor);
				ImGui::Combo("Material", &localDesyncType, allType, IM_ARRAYSIZE(allType));
				ImGui::Checkbox("Wireframe ##2", &localDesyncXhair);
				ImGui::Checkbox("Skeleton ##2", &localDesyncSkeleton);
			}
			else if (selectedModelType == 1) {

				if (ImGui::Button("Overlay", ImVec2(ImGui::GetContentRegionAvail().x / 3.f, 15.f), selectedOverlayType == 0))
					selectedOverlayType = 0;

				ImGui::SameLine();
				if (ImGui::Button("Outline", ImVec2(ImGui::GetContentRegionAvail().x / 2, 15.f), selectedOverlayType == 1))
					selectedOverlayType = 1;

				ImGui::SameLine();
				if (ImGui::Button("Animated", ImVec2(ImGui::GetContentRegionAvail().x, 15.f), selectedOverlayType == 2))
					selectedOverlayType = 2;

				switch (selectedOverlayType) {
				case 0:
					ImGui::Checkbox("Enabled", &localOverlay);
					ImGui::ColorEdit4("##localOverlayColor", localOverlayColor);
					ImGui::Checkbox("Wireframe ##2", &localOverlayXhair);

					ImGui::Checkbox("Behind wall", &localOverlayXQZ);
					ImGui::ColorEdit4("##localOverlayXQZColor", localOverlayXQZColor);
					ImGui::Checkbox("Wireframe behind wall ##2", &localOverlayXQZXhair);
					break;

				case 1:
					ImGui::Checkbox("Enabled", &localThinOverlay);
					ImGui::ColorEdit4("##localThinOverlayColor", localThinOverlayColor);
					ImGui::Checkbox("Wireframe ##2", &localThinOverlayXhair);

					ImGui::Checkbox("Behind wall", &localThinOverlayXQZ);
					ImGui::ColorEdit4("##localThinOverlayXQZColor", localThinOverlayXQZColor);
					ImGui::Checkbox("Wireframe behind wall ##2", &localThinOverlayXQZXhair);
					break;

				case 2:
					ImGui::Checkbox("Enabled", &localAnimOverlay);
					ImGui::ColorEdit4("##localThinOverlayColor", localAnimOverlayColor);
					ImGui::Checkbox("Wireframe ##2", &localAnimOverlayXhair);

					ImGui::Checkbox("Behind wall", &localAnimOverlayXQZ);
					ImGui::ColorEdit4("##localThinOverlayXQZColor", localAnimOverlayXQZColor);
					ImGui::Checkbox("Wireframe behind wall ##2", &localAnimOverlayXQZXhair);
					break;
				}
			}
			else {

				ImGui::Combo("Model##3", &attachmentChamsMaterial[LOCAL], chamsType, IM_ARRAYSIZE(chamsType));
				ImGui::Checkbox("Enabled", &attachmentChams[LOCAL]);
				ImGui::ColorEdit4("##attachmentColor", attachmentChamsColor[LOCAL]);
				ImGui::Checkbox("Wireframe##3", &attachmentChamsXhair[LOCAL]);

				if (ImGui::Button("Overlay", ImVec2(ImGui::GetContentRegionAvail().x / 3.f, 15.f), selectedOverlayType == 0))
					selectedOverlayType = 0;

				ImGui::SameLine();
				if (ImGui::Button("Outline", ImVec2(ImGui::GetContentRegionAvail().x / 2, 15.f), selectedOverlayType == 1))
					selectedOverlayType = 1;

				ImGui::SameLine();
				if (ImGui::Button("Animated", ImVec2(ImGui::GetContentRegionAvail().x, 15.f), selectedOverlayType == 2))
					selectedOverlayType = 2;

				switch (selectedOverlayType) {
				case 0:
					ImGui::Checkbox("Enabled##2", &attachmentOverlay[LOCAL]);
					ImGui::ColorEdit4("##attachmentOverlayColor", attachmentOverlayColor[LOCAL]);
					ImGui::Checkbox("Wireframe##4", &attachmentOverlayXhair[LOCAL]);
					break;

				case 1:
					ImGui::Checkbox("Enabled##2", &attachmentThinOverlay[LOCAL]);
					ImGui::ColorEdit4("##attachmentOverlayColor", attachmentThinOverlayColor[LOCAL]);
					ImGui::Checkbox("Wireframe##4", &attachmentThinOverlayXhair[LOCAL]);
					break;

				case 2:
					ImGui::Checkbox("Enabled##2", &attachmentAnimatedOverlay[LOCAL]);
					ImGui::ColorEdit4("##attachmentOverlayColor", attachmentAnimatedOverlayColor[LOCAL]);
					ImGui::Checkbox("Wireframe##4", &attachmentAnimatedOverlayXhair[LOCAL]);
					break;
				}
			}
            break;
        case VIEWMODEL:

            ImGui::Combo("Weapon model", &weaponType, chamsType, IM_ARRAYSIZE(chamsType));
            ImGui::Checkbox("Enabled##2", &weapon);
            ImGui::ColorEdit4("##weaponColor", weaponColor);
            ImGui::Checkbox("Wireframe##3", &weaponXhair);

            static int selectedweapon = 0;
            ImGui::Combo("Overlay type##2", &selectedweapon, glowType, IM_ARRAYSIZE(glowType));

            switch (selectedweapon) {

            case 0:
                ImGui::Checkbox("Enabled##3", &weaponOverlay);
                ImGui::ColorEdit4("##localOverlayColor", weaponOverlayColor);
                ImGui::Checkbox("Wireframe ##4", &weaponOverlayXhair);
                break;

            case 1:
                ImGui::Checkbox("Enabled##3", &weaponThinOverlay);
                ImGui::ColorEdit4("##weaponThinOverlayColor", weaponThinOverlayColor);
                ImGui::Checkbox("Wireframe ##4", &weaponThinOverlayXhair);
                break;

            case 2:
                ImGui::Checkbox("Enabled##3", &weaponAnimOverlay);
                ImGui::ColorEdit4("##weaponAnimOverlayColor", weaponAnimOverlayColor);
                ImGui::Checkbox("Wireframe ##4", &weaponAnimOverlayXhair);
                break;
            }

            break;
        }
    }
    ImGui::EndChild();

}

void menu::Misctab() noexcept {

    using namespace cfg::misc;
    ImGui::BeginChild("left", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y), true, ImGuiWindowFlags_NoMove );
    {
        ImGui::Checkbox("Bunny hop", &bunnyhop);
        ImGui::Checkbox("Auto strafe", &autoStrafe);
        ImGui::Checkbox("Fast stop", &faststop);
        ImGui::Checkbox("Infinite duck", &infiniteDuck);
		ImGui::Checkbox("Blockbot", &blockbot);
		ImGui::Keybind("##blockbotKey", &blockbotKey);
		ImGui::Checkbox("Clantag", &clantag);

#if _DEBUG
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(219.f / 255.f, 216.f / 255.f, 0.f, 1.f));
        ImGui::Checkbox("!Local hitboxes!", &m_bDrawServerHitbox);
        ImGui::Checkbox("!Entity hitboxes!", &m_bDrawServerHitboxOnAllEntities);
        ImGui::PopStyleColor();
#endif

        ImGui::Checkbox("Russian roulette", &m_bRussianRoulette);
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild( "modelchild", ImVec2( ImGui::GetContentRegionAvail( ).x, ImGui::GetContentRegionAvail( ).y ), true, ImGuiWindowFlags_NoMove );
    {
		ImGui::Checkbox("Fake ping", &fakePing);
		ImGui::SliderFloat("Latency", &fakePingFactor, 0.f, 1000.f, "%.f");

		if (g::pLocal) {
			if (g::pLocal->GetTeam() == TEAM_CT) {
				static const char* pistolsCT[] = { "None", "Usp-s / p2000", "Dual beretta", "P250", "Five-seven / Cz-auto", "Desert eagle / Revolver" };
				static const char* riflesCT[] = { "None", "Famas", "M4a1-s / M4a4", "Aug" };
				static const char* snipersCT[] = { "None", "Ssg08", "Awp", "Scar-20" };

				static const char* equipmentsCT[] = { "Kevlar + Helmet", "Zeus", "Defuse kit" };
				static const char* grenadesCT[] = { "Incendiary grenade", "Decoy grenade", "Flashbang", "He grenade", "Smoke grenade" };

				ImGui::Checkbox("Autobuy masterswitch", &autobuyEnabled);

				ImGui::Combo("Pistols", &cfg::misc::pistols, pistolsCT, IM_ARRAYSIZE(pistolsCT));
				ImGui::Combo("Snipers", &cfg::misc::snipers, snipersCT, IM_ARRAYSIZE(snipersCT));

				ImGui::MultiComboBox("Equipments", equipmentsCT, equipments, IM_ARRAYSIZE(equipmentsCT));
				ImGui::MultiComboBox("Grenades", grenadesCT, grenades, IM_ARRAYSIZE(grenadesCT));
			}
			else if (g::pLocal->GetTeam() == TEAM_TT) {

				static const char* pistolsT[] = { "none", "glock", "dual beretta", "p250", "tec9 / cz-auto", "desert eagle / revolver" };
				static const char* riflesT[] = { "none", "galil ar", "ak47", "sg 553" };
				static const char* snipersT[] = { "none", "ssg08", "awp", "g3sg1" };

				static const char* equipmentsT[] = { "kevlar + helmet", "zeus" };
				static const char* grenadesT[] = { "molotov", "decoy grenade", "flashbang", "he grenade", "smoke grenade" };

				ImGui::Checkbox("Autobuy masterswitch", &autobuyEnabled);

				ImGui::Combo("Pistols", &cfg::misc::pistols, pistolsT, IM_ARRAYSIZE(pistolsT));
				ImGui::Combo("Snipers", &cfg::misc::snipers, snipersT, IM_ARRAYSIZE(snipersT));

				ImGui::MultiComboBox("Equipments", equipmentsT, equipments, IM_ARRAYSIZE(equipmentsT));
				ImGui::MultiComboBox("Grenades", grenadesT, grenades, IM_ARRAYSIZE(grenadesT));
			}
		}
    }
    ImGui::EndChild( );
}

void menu::Skintab() noexcept {

#if _DEBUG
    ImGui::BeginChild("left", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y), true, ImGuiWindowFlags_NoMove );
    {
        ImGui::Checkbox("Debug button", &cfg::debugSwitch);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("test tooltip");

		ImGui::Checkbox("Gather AI information ##itsnot", &cfg::debugSwitch2);
		ImGui::SliderInt("Debug slider speed", &cfg::debugSlider, 0, 16);
    }
    ImGui::EndChild();
#endif
}

void menu::HandleLogoDrawing() noexcept {

    ImGui::PushFont(logoFont);
    ImGui::Text("Ryze");
    ImGui::PopFont();

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(111.f / 255, 203.f / 255, 243.f / 255, 255.f / 255));
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 25);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5);

    ImGui::PushFont(xtrFont);
    ImGui::Text("XTR");

    ImGui::PopFont();
    ImGui::PopStyleColor();

	ImGui::PushFont(xtrFont);
#if ALPHA
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Alpha").x);
	ImGui::Text("Alpha");
#endif
#if _DEBUG
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Developer").x);
	ImGui::Text("Developer");
#endif
#if NDEBUG
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("User").x);
	ImGui::Text("User");
#endif
	ImGui::PopFont();
}

void menu::HandleVisualTypeGeneration(
	bool& enable,
	bool& name,
	float* nameColor,

	bool& box,
	float* boxColor,

	bool& health,
	float* healthColor,
	float* healthColorEnd,

	bool& glow,
	float* glowColor,

	bool& armor,
	float* armorColor,

	bool& ammo,
	float* ammoColor,

	bool& weapon,
	float* weaponColor,

	bool* bFlags,
	float flFlagsColor[5][4],

	bool& bSkeleton,
	float* flSkeletonColor
) 
noexcept {

    ImGui::BeginChild("espchild", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y), true, ImGuiWindowFlags_NoMove );
    {
        ImGui::Checkbox("Enabled", &enable);
        ImGui::Checkbox("Bounding box", &box);
        ImGui::ColorEdit4("##boxcolor", boxColor);

        ImGui::Checkbox("Glow", &glow);
        ImGui::ColorEdit4("##glowcolor", glowColor);

        ImGui::Checkbox("Name", &name);
        ImGui::ColorEdit4("##namecolor", nameColor);

        ImGui::Checkbox("Health", &health);
        ImGui::ColorEdit4("##healthcolor", healthColor, true);
		ImGui::ColorEdit4("##healthcolorEnd", healthColorEnd);

        ImGui::Checkbox("Armor", &armor);
        ImGui::ColorEdit4("##armorcolor", armorColor);

        ImGui::Checkbox("Weapon", &weapon);
        ImGui::ColorEdit4("##weaponcolor", weaponColor);

        ImGui::Checkbox("Ammo", &ammo);
        ImGui::ColorEdit4("##ammocolor", ammoColor);

		ImGui::Checkbox("Skeleton", &bSkeleton);
		ImGui::ColorEdit4("##skeletoncolor", flSkeletonColor);

		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.f);
		if (ImGui::Button("Flags", ImVec2(ImGui::GetContentRegionAvail().x, 20.f)))
			ImGui::OpenPopup("##flagsManager");
		ImGui::PopStyleVar();

		bool bOpen = true;
		ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(200, 200));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(200, 165 ));
		if (ImGui::BeginPopupModal("##flagsManager", &bOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar ) ) {

			ImGui::BeginChild("##flags", ImGui::GetContentRegionAvail(), true);
			{
				ImGui::Checkbox("Name", &bFlags[NAME]);
				ImGui::ColorEdit4("##nameColor", flFlagsColor[NAME]);

				ImGui::Checkbox("Health", &bFlags[HEALTH]);
				ImGui::ColorEdit4("##healthColor", flFlagsColor[HEALTH]);

				ImGui::Checkbox("Armor", &bFlags[ARMOR]);
				ImGui::ColorEdit4("##armorColor", flFlagsColor[ARMOR]);

				ImGui::Checkbox("Ammo", &bFlags[AMMO]);
				ImGui::ColorEdit4("##ammoColor", flFlagsColor[AMMO]);

				ImGui::Checkbox("Money", &bFlags[MONEY]);
				ImGui::ColorEdit4("##moneyColor", flFlagsColor[MONEY]);

				ImGui::Checkbox("Weapon", &bFlags[WEAPON]);
				ImGui::ColorEdit4("##weapon Color", flFlagsColor[WEAPON]);

				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.f);
				if (ImGui::Button("Close", ImVec2(ImGui::GetContentRegionAvail().x, 20.f)))
					ImGui::CloseCurrentPopup();
				ImGui::PopStyleVar();
			}
			ImGui::EndChild();

			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();
    }
    ImGui::EndChild();
}

void menu::ConfigTab() noexcept {

	ImGui::BeginChild("left", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y), true, ImGuiWindowFlags_NoMove);
	{
		static std::string selectedConfig = "";
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		if (ImGui::ListBoxVector("##configs", &cfg::configID, Config2->vecConfigs, 7)) {
			selectedConfig = Config2->vecConfigs[cfg::configID];
		}

		static char buf[255]{};
		ImGui::InputText("Config name", buf, sizeof(buf));

		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.f);
		if (ImGui::Button("Refresh", ImVec2(ImGui::GetContentRegionAvail().x, 20.f))) {
			Config2->RefreshConfigs();
		}

		ImGui::Spacing();
		if (ImGui::Button("Save", ImVec2(ImGui::GetContentRegionAvail().x, 20.f))) {
			pressedSave = true;
			warningMethod = true;
		}

		ImGui::Spacing();
		if (ImGui::Button("Load", ImVec2(ImGui::GetContentRegionAvail().x, 20.f))) {
			pressedSave = true;
			warningMethod = false;
		}

		ImGui::Spacing();
		if (ImGui::Button("Create", ImVec2(ImGui::GetContentRegionAvail().x, 20.f))) {

			Config2->Save(buf);
			Config2->RefreshConfigs();
		}

		ImGui::Spacing();
		if (ImGui::Button("Delete", ImVec2(ImGui::GetContentRegionAvail().x, 20.f))) {

			Config2->DeleteConfig(selectedConfig);
			Config2->RefreshConfigs();
		}

		ImGui::Spacing();
		if (ImGui::Button("Open config location", ImVec2(ImGui::GetContentRegionAvail().x, 20.f))) {

			ShellExecuteA(NULL, "open", Config2->ConfigPath.c_str(), NULL, NULL, SW_SHOWNORMAL);;
		}

		ImGui::PopStyleVar();
		ImGui::PopItemWidth();
	}
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("modelchild", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true, ImGuiWindowFlags_NoMove);
	{
		ImGui::Text("scripts will be here in the future");
	}
	ImGui::EndChild();
}

void menu::KeyBindList() noexcept {

    static int height = 15;

    if (!cfg::misc::keyBindList)
        return;

    ImGui::SetNextWindowSizeConstraints(ImVec2(130, height), ImVec2(180, height));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(180, height));
    ImGui::PushFont(childFont);
    ImGui::Begin("##keystuff", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    {
        height = 15;
        ImGui::PopStyleVar();

        ImGui::BeginChild("##binds", ImGui::GetContentRegionAvail(), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        {
            if ( IPT::HandleInput(cfg::misc::thirdpersonbind)) {
                ImGui::Text("Thirdperson [ON]");
                height += 20;
            }
            else {
                ImGui::Text("Thirdperson [OFF]");
                height += 20;
            }
            if ( IPT::HandleInput(cfg::antiaim::iInverterBind)) {

                ImGui::Text("Invert [ON]");
                height += 20;
            }
            else {
                ImGui::Text("Invert [OFF]");
                height += 20;
            }
            if ( IPT::HandleInput(cfg::antiaim::fakeduckbind) && cfg::antiaim::fakeduck) {
                ImGui::Text("Fakeduck [ON]");
                height += 20;
            }
            if (cfg::rage::doubletap && IPT::HandleInput(cfg::rage::doubletapkey)) {
                ImGui::Text("Doubletap [ON]");
                height += 20;
            }
            if ( IPT::HandleInput(cfg::rage::overrideBind)) {
                ImGui::Text("Override [ON]");
                height += 20;
            }
            if ( IPT::HandleInput(cfg::antiaim::idealTickBind) && cfg::antiaim::idealTick) {
                ImGui::Text("Autopeek [ON]");
                height += 20;
            }
            if ( IPT::HandleInput(cfg::antiaim::fakewalkKey) && cfg::antiaim::fakewalkenable) {
                ImGui::Text("Slow motion [ON]");
                height += 20;
            }
            if ( IPT::HandleInput(cfg::rage::forceBaimKey) && cfg::rage::forceBaim) {
                ImGui::Text("Force baim [ON]");
                height += 20;
            }
			if (IPT::HandleInput(cfg::misc::blockbotKey) && cfg::misc::blockbot) {
				ImGui::Text("Blockbot [ON]");
				height += 20;
			}
        }
        ImGui::EndChild();
    }
    ImGui::End();
    ImGui::PopFont();
}

void menu::SaveWarning(bool& saved, bool type) noexcept {

    ImGui::SetNextWindowSizeConstraints(ImVec2(180, 101), ImVec2(180, 101));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(180, 101));
    ImGui::PushFont(childFont);
    ImGui::SetNextWindowPos( ImVec2( ( ImGui::GetIO().DisplaySize.x * 0.5f ) - ( 180 * 0.5f ), ImGui::GetIO( ).DisplaySize.y * 0.5f + ( 101 * 0.5f ) ));
    ImGui::Begin("##savestuff", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    {
        ImGui::BeginChild("##yes", ImGui::GetContentRegionAvail(), true);
        {
            ImGui::SetCursorPos(ImVec2(ImGui::GetContentRegionAvail().x / 4 - 3, ImGui::GetContentRegionAvail().y / 5));
            ImGui::Text("Are you sure?");

            static ImVec2 buttonSize = ImVec2(ImGui::GetContentRegionAvail().x / 3, ImGui::GetContentRegionAvail().y / 2);
            if (ImGui::Button("Yes", buttonSize)) {
                saved = false;
				type ? Config2->Save(Config2->vecConfigs[cfg::configID]) : Config2->Load(Config2->vecConfigs[cfg::configID]);
            }
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x / 2);
            if (ImGui::Button("No", buttonSize))
                saved = false;
        }
        ImGui::EndChild();
    }
    ImGui::PopStyleVar();
    ImGui::PopFont();
    ImGui::End();
}