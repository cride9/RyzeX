#include "gui.h"
#include "config.h"
#include "../Entity.h"
#include "../../globals.h"

ETabs selectedTab = RAGE_TAB;
EEntity selectedEsp = ENEMY;
void menu::HandleMenuElements() noexcept {

    ImGui::Begin("RyzeX", NULL, ImGuiWindowFlags_NoTitleBar);
    {
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
        }

        ImGui::PopFont();
    }
    ImGui::End();
}

void menu::Tabselection() noexcept {

    ImGui::PushFont(tabFont);
    ImGui::BeginChild("leftchild", ImVec2(100.f, ImGui::GetContentRegionAvail().y), true);
    {
        if (ImGui::Button("Ragebot", ImVec2(ImGui::GetContentRegionMax().x, 40), selectedTab == RAGE_TAB))
            selectedTab = RAGE_TAB;

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20);
        if (ImGui::Button("Antiaim", ImVec2(ImGui::GetContentRegionMax().x, 40), selectedTab == ANTIAIM_TAB))
            selectedTab = ANTIAIM_TAB;

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20);
        if (ImGui::Button("Visual", ImVec2(ImGui::GetContentRegionMax().x, 40), selectedTab == VISUAL_TAB))
            selectedTab = VISUAL_TAB;

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20);
        if (ImGui::Button("Misc", ImVec2(ImGui::GetContentRegionMax().x, 40), selectedTab == MISC_TAB))
            selectedTab = MISC_TAB;

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20);
        if (ImGui::Button("Skins", ImVec2(ImGui::GetContentRegionMax().x, 40), selectedTab == SKIN_TAB))
            selectedTab = SKIN_TAB;
    }
    ImGui::EndChild();
    ImGui::PopFont();

    ImGui::SameLine();
}

void menu::Ragetab() noexcept {

    using namespace cfg::rage;
    ImVec2 savedCursorPos = ImVec2(ImGui::GetCursorPos().x, ImGui::GetCursorPos().y + ImGui::GetContentRegionAvail().y / 2 + ImGui::GetStyle().WindowPadding.y + 2);

    ImGui::BeginChild("leftside", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y), true);
    {
        ImGui::Checkbox("Enable", &enable);
        ImGui::Checkbox("Auto Stop", &autostop);
        if (autostop) {
            ImGui::Checkbox("Auto Stop In Air", &m_bAutoStopInAir);
            ImGui::Checkbox("Between shots", &betweenshots);
        }
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(219.f / 255.f, 216.f / 255.f, 0.f, 1.f));
        ImGui::Checkbox("Doubletap", &doubletap);
        ImGui::PopStyleColor();
        ImGui::Keybind("DoubletapKey", &doubletapkey);
        ImGui::Checkbox("Anti-aim correction", &resolver);
        ImGui::SliderInt("Backtrack ticks", &backtrackTicks, 1, 12);
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("rightside", ImGui::GetContentRegionAvail(), true);
    {
        static const char* item[] = { "auto", "scout", "awp", "pistol", "heavy pistol", "other" };
        static int selectedWeapon = 0;
        ImGui::Combo("Weapon Config", &selectedWeapon, item, IM_ARRAYSIZE(item));

        static const char* items[] = { "head", "upper chest", "lower chest", "stomach", "arms", "legs" };

        static auto buildTabForWeapon = [](bool* hitboxes, int& hitChance, int& minDmg, int& headPoints, int& bodyPoints, int& overridedmg, EWEAPON weapon) {

            ImGui::MultiComboBox("Hitboxes", items, hitboxes, IM_ARRAYSIZE(items));

            ImGui::SliderInt("Hitchance", &hitChance, 0, 100);
            ImGui::SliderInt("Minimum Damage", &minDmg, 0, 110);

            ImGui::SliderInt("Head Pointscale", &headPoints, 0, 100);
            ImGui::SliderInt("Body Pointscale", &bodyPoints, 0, 100);

            ImGui::Checkbox("Auto-Scope", &autoscope[weapon]);
            ImGui::SliderInt("Damage override", &overridedmg, 0, 110);
            ImGui::Keybind("damageoverridebind", &overrideBind);
        };

        switch (selectedWeapon)
        {
        case AUTO: buildTabForWeapon(autoHitboxes, autoHitchance, autoMindmg, autoHeadPoints, autoBodyPoints, autoOverride, AUTO);
            break;
        case SCOUT:buildTabForWeapon(scoutHitboxes, scoutHitchance, scoutMindmg, scoutHeadPoints, scoutBodyPoints, scoutOverride, SCOUT);
            break;
        case AWP: buildTabForWeapon(awpHitboxes, awpHitchance, awpMindmg, awpHeadPoints, awpBodyPoints, awpOverride, AWP);
            break;
        case PISTOL: buildTabForWeapon(pistolHitboxes, pistolHitchance, pistolMindmg, pistolHeadPoints, pistolBodyPoints, pistolOverride, PISTOL);
            break;
        case HEAVY_PISTOL: buildTabForWeapon(heavypistolHitboxes, heavypistolHitchance, heavypistolMindmg, heavypistolHeadPoints, heavypistolBodyPoints, heavypistolOverride, HEAVY_PISTOL);
            break;
        case OTHER: buildTabForWeapon(etcHitboxes, etcHitchance, etcMindmg, etcHeadPoints, etcBodyPoints, etcOverride, OTHER);
            break;
        }
    }
    ImGui::EndChild();
}

void menu::Antiaimtab() noexcept {

    using namespace cfg::antiaim;

    static const char* yawList[] = { "Forward", "Backward" };
    static const char* pitchList[] = { "Up", "Zero" ,"Down" };
    static const char* desyncList[] = { "Off", "Static", "Jitter" };
    static const char* yawBaseList[] = { "Local view", "Freestand" };
    static const char* fakelagTypeList[] = { "Normal", "Adaptive", "Jitter" };
    static const char* yawModifierList[] = { "Off", "Jitter", "Random" };

    ImGui::BeginChild("LeftChild", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y), true);
    {
        ImGui::Checkbox("Enabled", &enabled);
        ImGui::Combo("Pitch", &pitch, pitchList, IM_ARRAYSIZE(pitchList));
        ImGui::Combo("Yaw base", &yawBase, yawBaseList, IM_ARRAYSIZE(yawBaseList));
        ImGui::Combo("Yaw", &yaw, yawList, IM_ARRAYSIZE(yawList));
        ImGui::Combo("Modifier", &modifier, yawModifierList, IM_ARRAYSIZE(yawModifierList));
        if (modifier != 0)
            ImGui::SliderInt("Modifier value", &jittervalue, 0, 90);
        ImGui::Combo("Lower body yaw target", &desynctype, desyncList, IM_ARRAYSIZE(desyncList));
        if (desynctype != 0) {
            ImGui::SliderFloat("Yaw desync angle", &desyncvalue, 0.f, 58.f, "%.f");
            ImGui::Keybind("invertButton", &desyncinverter);
            ImGui::Combo("Yaw target modifier", &desyncModifier, yawModifierList, IM_ARRAYSIZE(yawModifierList));
            if (desyncModifier != 0)
                ImGui::SliderInt("Modifier value ##2", &desyncModifierValue, 0, 58);
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImVec2 savedPosition = ImVec2(ImGui::GetCursorPos().x, ImGui::GetCursorPos().y + ImGui::GetContentRegionAvail().y / 2 + ImGui::GetStyle().WindowPadding.y + 2.f);

    ImGui::BeginChild("righttop", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 2), true);
    {
        fakelagmin = std::clamp(fakelagmin, 0, fakelag);
        fakelagmax = std::clamp(fakelagmax, fakelagmin, fakelag);
        ImGui::Checkbox("Enabled", &enableFakelag);
        ImGui::Combo("Choke type", &fakelagType, fakelagTypeList, IM_ARRAYSIZE(fakelagTypeList));
        ImGui::SliderInt("Base amount", &fakelag, 0, 15);
        ImGui::SliderInt("Minimum choke", &fakelagmin, 0, fakelag);
        ImGui::SliderInt("Maximum choke", &fakelagmax, fakelagmin, fakelag);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(219.f / 255.f, 216.f / 255.f, 0.f, 1.f));
        ImGui::Checkbox("Break lagcompenstaion", &defensive);
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();

    ImGui::SetCursorPos(savedPosition);

    ImGui::BeginChild("rightbot", ImGui::GetContentRegionAvail(), true);
    {
        ImGui::Checkbox("Slow motion", &fakewalkenable);
        ImGui::Keybind("slowmotionKey", &fakewalkKey);
        ImGui::SliderFloat("Slow motion speed", &fakewalk, 0.f, 100.f, "%.f");

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

    float buttonSize = (ImGui::GetContentRegionAvail().x / 4) - (2 * 4);
    ImVec2 savedPosition = ImVec2(ImGui::GetCursorPos().x, ImGui::GetCursorPos().y + 30.f + ImGui::GetStyle().WindowPadding.y + 2.f);
   
    ImGui::BeginChild("selectPlayer", ImVec2(ImGui::GetContentRegionAvail().x, 30.f), true);
    {
        if (ImGui::Button("Enemy", ImVec2(buttonSize, 20), selectedEsp == ENEMY))
            selectedEsp = ENEMY;
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX());
        if (ImGui::Button("Teammate", ImVec2(buttonSize, 20), selectedEsp == TEAM))
            selectedEsp = TEAM;

        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX());
        if (ImGui::Button("Local", ImVec2(buttonSize, 20), selectedEsp == LOCAL))
            selectedEsp = LOCAL;

        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX());
        if (ImGui::Button("World", ImVec2(buttonSize, 20), selectedEsp == WORLD))
            selectedEsp = WORLD;
    }
    ImGui::EndChild();

    ImGui::SetCursorPos(savedPosition);


    switch (selectedEsp)
    {
    using namespace cfg::visual;
    case ENEMY:
        HandleVisualTypeGeneration(
            enemyEsp,
            enemyName,
            enemyNameColor,
            enemyBox,
            enemyBoxColor,
            enemyHealth,
            enemyHealthColor,
            enemyGlow,
            enemyGlowColor,
            enemyArmor,
            enemyArmorColor,
            enemyAmmo,
            enemyAmmoColor,
            enemyMoney,
            enemyMoneyColor,
            enemyWeapon,
            enemyWeaponColor);
        break;

    case TEAM:
        HandleVisualTypeGeneration(
            teamEsp,
            teamName,
            teamNameColor,
            teamBox,
            teamBoxColor,
            teamHealth,
            teamHealthColor,
            teamGlow,
            teamGlowColor,
            teamArmor,
            teamArmorColor,
            teamAmmo,
            teamAmmoColor,
            teamMoney,
            teamMoneyColor,
            teamWeapon,
            teamWeaponColor);
        break;

    case LOCAL:
        HandleVisualTypeGeneration(
            localEsp,
            localName,
            localNameColor,
            localBox,
            localBoxColor,
            localHealth,
            localHealthColor,
            localGlow,
            localGlowColor,
            localArmor,
            localArmorColor,
            localAmmo,
            localAmmoColor,
            localMoney,
            localMoneyColor,
            localWeapon,
            localWeaponColor);
        break;

    case WORLD:
        using namespace cfg::misc;

        ImGui::BeginChild("left", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y), true);
        {
            ImGui::Checkbox("Thirdperson", &thirdperson);
            ImGui::Keybind("tpKey", &thirdpersonbind);

            ImGui::Checkbox("Nightmode", &nightmode);
            ImGui::ColorEdit4("nightmodecolor", nightmodeColor);

            ImGui::Checkbox("Preserve killfeed", &preserveKillfeed);

            ImGui::Checkbox("Aspect ratio", &aspectRatio);
            ImGui::SliderInt("Ratio", &aspectRatioValue, 0, 100);

            ImGui::SliderInt("Viewmodel fov", &viewmodelFov, 45, 140);
            ImGui::SliderInt("Debug fov", &fov, 40, 140);

            ImGui::Checkbox("Remove smoke", &removals[0]);
            ImGui::Checkbox("Remove flash", &removals[1]);
            ImGui::Checkbox("Remove recoil", &removals[2]);
            ImGui::Checkbox("Remove zoom", &removals[3]);
            ImGui::Checkbox("Remove post processing", &removals[4]);
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("right", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true);
        {
            ImGui::Checkbox("Show impact", &bulletImpact);
            ImGui::Checkbox("Paper mode", &cfg::model::paperMode);
            ImGui::Checkbox("Keybind list", &keyBindList);
        }
        ImGui::EndChild();

        return;
    }
    ImGui::SameLine();

    static const char* chamsType[] = { "Default", "Flat" };
    static const char* glowType[] = { "Glow", "Thin glow", "Animated" };
    static const char* enemyTypes[] = { "Glow", "Thin glow", "Animated", "Backtrack"};
    static const char* localTypes[] = { "Glow", "Thin glow", "Animated", "Desync"};
    static const char* allType[] = { "Default", "Flat", "Glow", "Thin glow", "Animated" };

    ImGui::BeginChild("modelchild", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true);
    {
        using namespace cfg::model;

        static int selectedTeam = 0;
        static int selectedEnemy = 0;
        static int selectedLocal = 0;

        switch (selectedEsp)
        {
        case ENEMY:

            ImGui::Combo("Model", &enemyType, chamsType, IM_ARRAYSIZE(chamsType));

            ImGui::Checkbox("Player", &enemy);
            ImGui::ColorEdit4("##enemycolor", enemyColor);
            ImGui::Checkbox("Wireframe", &enemyXhair);

            ImGui::Checkbox("Player behind wall", &enemyXQZ);
            ImGui::ColorEdit4("##enemycolorXQZ", enemyXQZColor);
            ImGui::Checkbox("Wireframe behind wall", &enemyXQZXhair);

            ImGui::Combo("Overlay type", &selectedEnemy, enemyTypes, IM_ARRAYSIZE(enemyTypes));

            switch (selectedEnemy)
            {
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

            case 3:
                ImGui::Checkbox("Enabled", &enemyBTEnable);
                ImGui::ColorEdit4("##enemyBTColor", enemyBTColor);
                ImGui::Combo("Backtrack type", &enemyBTType, allType, IM_ARRAYSIZE(allType));
                ImGui::Checkbox("Wireframe ##2", &enemyBTXhair);
                break;
            }

            break;
        case TEAM:
            ImGui::Combo("Model", &teamType, chamsType, IM_ARRAYSIZE(chamsType));

            ImGui::Checkbox("Player", &team);
            ImGui::ColorEdit4("##teamcolor", teamColor);
            ImGui::Checkbox("Wireframe", &teamXhair);

            ImGui::Checkbox("Player behind wall", &teamXQZ);
            ImGui::ColorEdit4("##teamcolorXQZ", teamXQZColor);
            ImGui::Checkbox("Wireframe behind wall", &teamXQZXhair);

            ImGui::Combo("Overlay type", &selectedTeam, glowType, IM_ARRAYSIZE(glowType));

            switch (selectedTeam)
            {
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
            break;
        case LOCAL:

            ImGui::Combo("Model", &localType, chamsType, IM_ARRAYSIZE(chamsType));

            ImGui::Checkbox("Player", &local);
            ImGui::ColorEdit4("##localcolor", localColor);
            ImGui::Checkbox("Wireframe", &localXhair);

            ImGui::Checkbox("Player behind wall", &localXQZ);
            ImGui::ColorEdit4("##localcolorXQZ", localXQZColor);
            ImGui::Checkbox("Wireframe behind wall", &localXQZXhair);

            ImGui::Combo("Overlay type", &selectedLocal, localTypes, IM_ARRAYSIZE(localTypes));

            switch (selectedLocal)
            {
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

            case 3:

                ImGui::Checkbox("Enabled", &localDesync);
                ImGui::ColorEdit4("##localDesyncColor", localDesyncColor);

                ImGui::Combo("Desync type", &localDesyncType, allType, IM_ARRAYSIZE(allType));
                ImGui::Checkbox("Wireframe ##2", &localDesyncXhair);
                break;
            }
            break;
        }
    }
    ImGui::EndChild();

}

void menu::Misctab() noexcept {

    using namespace cfg::misc;
    ImGui::BeginChild("left", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y), true);
    {
        ImGui::Checkbox("Bunny hop", &bunnyhop);
        ImGui::Checkbox("Auto strafe", &autoStrafe);
        ImGui::Checkbox("Fast stop", &faststop);

#if _DEBUG
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(219.f / 255.f, 216.f / 255.f, 0.f, 1.f));
        ImGui::Checkbox("!Local hitboxes!", &m_bDrawServerHitbox);
        ImGui::Checkbox("!Entity hitboxes!", &m_bDrawServerHitboxOnAllEntities);
        ImGui::PopStyleColor();
#endif

        ImGui::Checkbox("Fake ping", &fakePing);
        ImGui::SliderFloat("Latency", &fakePingFactor, 0.f, 1000.f, "%.f");

        if (g::pLocal) {
            if (g::pLocal->GetTeam() == TEAM_CT) {
                static const char* pistolsCT[] = { "None", "Usp-s / p2000", "Dual beretta", "P250", "Five-seven / Cz-auto", "Desert eagle / Revolver" };
                static const char* riflesCT[] = { "None", "Famas", "M4a1-s / M4a4", "Aug" };
                static const char* snipersCT[] = { "None", "Ssg08", "Awp", "Scar-20" };

                static const char* equipmentsCT[] = { "Kevlar + Helmet", "Zeus", "Defuse kit" };
                static const char* grenadesCT[] = { "Incendiary grenade", "Decoy grenade", "Flashbang", "He grenade", "Smoke grenade" };

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

                ImGui::Combo("Pistols", &cfg::misc::pistols, pistolsT, IM_ARRAYSIZE(pistolsT));
                ImGui::Combo("Snipers", &cfg::misc::snipers, snipersT, IM_ARRAYSIZE(snipersT));

                ImGui::MultiComboBox("Equipments", equipmentsT, equipments, IM_ARRAYSIZE(equipmentsT));
                ImGui::MultiComboBox("Grenades", grenadesT, grenades, IM_ARRAYSIZE(grenadesT));
            }
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("modelchild", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true);
    {
        static const char* item[] = { "semi-rage", "hvh", "baim", "headshot" };
        ImGui::Combo("configs", &cfg::configID, item, IM_ARRAYSIZE(item));

        if (ImGui::Button("save", ImVec2(ImGui::GetContentRegionAvail().x, 50.f)))
            Config2->Save(item[cfg::configID]);

        ImGui::Spacing();
        if (ImGui::Button("load", ImVec2(ImGui::GetContentRegionAvail().x, 50.f)))
            Config2->Load(item[cfg::configID]);
    }
    ImGui::EndChild();
}

void menu::Skintab() noexcept {

#if _DEBUG
    ImGui::BeginChild("left", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y), true);
        ImGui::Checkbox("Debug button", &cfg::debugSwitch);
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
}

void menu::HandleVisualTypeGeneration(
    bool& enable,
    bool& name,
    float* nameColor,

    bool& box,
    float* boxColor,

    bool& health,
    float* healthColor,

    bool& glow,
    float* glowColor,

    bool& armor,
    float* armorColor,

    bool& ammo,
    float* ammoColor,

    bool& money,
    float* moneyColor,

    bool& weapon,
    float* weaponColor
) 
noexcept {

    ImGui::BeginChild("espchild", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y), true);
    {
        ImGui::Checkbox("Enabled", &enable);
        ImGui::Checkbox("Bounding box", &box);
        ImGui::ColorEdit4("##boxcolor", boxColor);

        ImGui::Checkbox("Glow", &glow);
        ImGui::ColorEdit4("##glowcolor", glowColor);

        ImGui::Checkbox("Name", &name);
        ImGui::ColorEdit4("##namecolor", nameColor);

        ImGui::Checkbox("Health", &health);
        ImGui::ColorEdit4("##healthcolor", healthColor);

        ImGui::Checkbox("Armor", &armor);
        ImGui::ColorEdit4("##armorcolor", armorColor);

        ImGui::Checkbox("Weapon", &weapon);
        ImGui::ColorEdit4("##weaponcolor", weaponColor);

        ImGui::Checkbox("Ammo", &ammo);
        ImGui::ColorEdit4("##ammocolor", ammoColor);

        ImGui::Checkbox("Money", &money);
        ImGui::ColorEdit4("##moneycolor", moneyColor);
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
            if (GetKeyState(cfg::misc::thirdpersonbind)) {
                ImGui::Text("Thirdperson [ON]");
                height += 20;
            }
            else {
                ImGui::Text("Thirdperson [OFF]");
                height += 20;
            }
            if (GetKeyState(cfg::antiaim::desyncinverter)) {

                ImGui::Text("Invert [ON]");
                height += 20;
            }
            else {
                ImGui::Text("Invert [OFF]");
                height += 20;
            }
            if (GetAsyncKeyState(cfg::antiaim::fakeduckbind) && cfg::antiaim::fakeduck) {
                ImGui::Text("Fakeduck [ON]");
                height += 20;
            }
            if (cfg::rage::doubletap && GetKeyState(cfg::rage::doubletapkey)) {
                ImGui::Text("Doubletap [ON]");
                height += 20;
            }
            if (GetAsyncKeyState(cfg::rage::overrideBind)) {
                ImGui::Text("Override [ON]");
                height += 20;
            }
            if (GetAsyncKeyState(cfg::antiaim::idealTickBind) && cfg::antiaim::idealTick) {
                ImGui::Text("Autopeek [ON]");
                height += 20;
            }
            if (GetAsyncKeyState(cfg::antiaim::fakewalkKey) && cfg::antiaim::fakewalkenable) {
                ImGui::Text("Slow motion [ON]");
                height += 20;
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
    ImGui::PopFont();
}