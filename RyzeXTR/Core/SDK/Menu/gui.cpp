#include "gui.h"
#include "../../globals.h"
#include "config.h"
#include <string>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProcess(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	if (GetAsyncKeyState(VK_INSERT) & 1)
		menu::open = !menu::open;

	if (menu::open && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {

		return 1L;
	}

	return CallWindowProc(menu::originalWindowProcess, hWnd, msg, wParam, lParam);
}

void Style() {

	ImGuiStyle& style = ImGui::GetStyle();
	ImGuiIO& io = ImGui::GetIO();

	io.IniFilename = "ryzeX";
	io.Fonts->AddFontDefault(); //verdana.ttf
	menu::logoFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\verdana.ttf", 20.0f);
	menu::logoFontsmall = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\verdana.ttf", 15.0f);
	menu::buildDateFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\verdana.ttf", 10.0f);
	IM_ASSERT(menu::logoFont != NULL);

	style.ScaleAllSizes(0.2f);

	style.Alpha = 1.f;

	style.Colors[ImGuiCol_WindowBg] = ImColor(12, 12, 12);
	style.Colors[ImGuiCol_ChildBg] = ImColor(20, 20, 20);
	style.Colors[ImGuiCol_Text] = ImColor(212, 212, 212);
	style.Colors[ImGuiCol_Border] = ImColor(80, 80, 80);

	style.Colors[ImGuiCol_ResizeGripHovered] = ImColor(12, 12, 12);
	style.Colors[ImGuiCol_ResizeGripActive] = ImColor(12, 12, 12);
	style.Colors[ImGuiCol_ResizeGrip] = ImColor(12, 12, 12);

	style.Colors[ImGuiCol_TitleBg] = ImColor(80, 80, 80);
	style.Colors[ImGuiCol_TitleBgActive] = ImColor(80, 80, 80);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImColor(80, 80, 80);

	style.Colors[ImGuiCol_Button] = ImColor(222, 153, 42);
	style.Colors[ImGuiCol_ButtonActive] = ImColor(222, 153, 42);
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(222, 153, 42);

	style.Colors[ImGuiCol_FrameBgActive] = ImColor(50, 50, 50);
	style.Colors[ImGuiCol_FrameBgHovered] = ImColor(50, 50, 50);
	style.Colors[ImGuiCol_FrameBg] = ImColor(40, 40, 40);
	style.Colors[ImGuiCol_CheckMark] = ImColor(222, 153, 42);

	style.Colors[ImGuiCol_SliderGrabActive] = ImColor(222, 153, 42);
	style.Colors[ImGuiCol_SliderGrab] = ImColor(222, 153, 42);

	style.Colors[ImGuiCol_HeaderActive] = ImColor(50, 50, 50);
	style.Colors[ImGuiCol_HeaderHovered] = ImColor(50, 50, 50);
	style.Colors[ImGuiCol_Header] = ImColor(40, 40, 40);

	style.FrameBorderSize = 1.f;

	style.ItemSpacing = ImVec2(5, 5);
	style.ItemInnerSpacing = ImVec2(5, 5);

	style.WindowPadding = ImVec2(10, 10);
	style.WindowMinSize = ImVec2(680, 450);
}

void menu::Render() noexcept {


	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();

	if (menu::open) {

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(222.f / 255.f, 153.f / 255.f, 42.f / 255.f, 1.f));

		ImGui::Begin("RyzeX", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
		{
			ImGui::PopStyleColor();
			ImGui::BeginChild("##tabs", ImVec2(ImGui::GetContentRegionAvail().x, 37.f), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			{
				ImGui::PushFont(buildDateFont);
				ImGui::TextColored(ImColor(222, 153, 42), "Build Date: \n", __DATE__ );
				ImGui::PopFont();
				ImGui::SameLine();

				ImGui::PopStyleVar();

				ImGui::PushFont(logoFont);
				ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x / 2 - 20);

				ImGui::TextColored(ImColor(222, 153, 42), "R");
				ImGui::SameLine();
				ImGui::PopFont();

				ImGui::SetCursorPosY(8);
				ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x / 2 - 10);

				ImGui::PushFont(logoFontsmall);
				ImGui::TextColored(ImColor(255, 255, 255), "yzeXTR");
				ImGui::SameLine();
				ImGui::PopFont();

				if (ImGui::ButtonCenteredOnLine("rage", 0.35f, tabindex == RAGE_TAB))
					tabindex = RAGE_TAB;

				ImGui::SameLine();
				if (ImGui::Button("antiaim", ImVec2(0.f, 0.f), tabindex == ANTIAIM_TAB))
					tabindex = ANTIAIM_TAB;

				ImGui::SameLine();
				if (ImGui::Button("visual", ImVec2(0.f, 0.f), tabindex == VISUAL_TAB))
					tabindex = VISUAL_TAB;

				ImGui::SameLine();
				if (ImGui::Button("misc", ImVec2(0.f, 0.f), tabindex == MISC_TAB))
					tabindex = MISC_TAB;

				ImGui::SameLine();
				if (ImGui::Button("skin", ImVec2(0.f, 0.f), tabindex == SKIN_TAB))
					tabindex = SKIN_TAB;
			}
			ImGui::EndChild();

			if (tabindex == VISUAL_TAB) {

				static int selectedESP = 0;
				static int selectedMode = 0;
				ImGui::BeginChild("##entitypicker", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y / 6), true, ImGuiWindowFlags_None);
				{
					static const char* item[] = { "enemy", "team", "local" };
					ImGui::Combo("player type", &selectedESP, item, IM_ARRAYSIZE(item));

					if (selectedESP == ENEMY)
						ImGui::Checkbox("enable", &cfg::visual::enemyEsp);
					else if (selectedESP == TEAM)
						ImGui::Checkbox("enable", &cfg::visual::teamEsp);
					else if (selectedESP == LOCAL)
						ImGui::Checkbox("enable", &cfg::visual::localEsp);
				}
				ImGui::EndChild();

				//ImGui::SameLine();

				if (selectedESP == ENEMY) {

					ImVec2 desiredPlace = ImGui::GetCursorPos();

					ImGui::SameLine();
					float otherX = ImGui::GetCursorPosX();
					float rightbotX = ImGui::GetContentRegionAvail().x - 30;
					float rightboxY = ImGui::GetContentRegionAvail().y / 3 - 45;
					ImGui::BeginChild("##enemyright", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 3), true);
					{
						static const char* chamsType[] = { "default", "flat" };
						ImGui::Combo("##enemyType", &cfg::model::enemyType, chamsType, IM_ARRAYSIZE(chamsType));

						ImGui::Checkbox("enable", &cfg::model::enemy); ImGui::SameLine(); ImGui::SetCursorPosX(rightbotX);
						ImGui::ColorEdit4("##enemyColor", cfg::model::enemyColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);
					
						if (cfg::model::enemy) {

							ImGui::Checkbox("through wall", &cfg::model::enemyXQZ); ImGui::SameLine(); ImGui::SetCursorPosX(rightbotX);
							ImGui::ColorEdit4("##enemyXQZColor", cfg::model::enemyXQZColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);
						}

						ImGui::SetCursorPosY(rightboxY);

						if (cfg::model::enemy)
							ImGui::Checkbox("wireframe", &cfg::model::enemyXhair);

						if (cfg::model::enemyXQZ && cfg::model::enemy)
							ImGui::Checkbox("wireframe trough wall", &cfg::model::enemyXQZXhair);
					}
					ImGui::EndChild();

					ImGui::SetCursorPosX(otherX);
					ImVec2 enemyrightbotRegion = ImGui::GetContentRegionAvail();
					ImGui::BeginChild("##enemyrightbot", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true);
					{
						static int selectedOverlay = 0;
						static const char* glowType[] = { "glow", "thin glow", "animated"};
						ImGui::Combo("##glowtype", &selectedOverlay, glowType, IM_ARRAYSIZE(glowType));

						if (selectedOverlay == 0) {

							ImGui::Checkbox("enable", &cfg::model::enemyOverlay); ImGui::SameLine(); ImGui::SetCursorPosX(enemyrightbotRegion.x - 30);
							ImGui::ColorEdit4("##enemyOverlayColor", cfg::model::enemyOverlayColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

							if (cfg::model::enemyOverlay) {

								ImGui::Checkbox("through wall", &cfg::model::enemyOverlayXQZ); ImGui::SameLine(); ImGui::SetCursorPosX(enemyrightbotRegion.x - 30);
								ImGui::ColorEdit4("##enemyOverlayXQZColor", cfg::model::enemyOverlayXQZColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);
							}

							ImGui::SetCursorPosY(enemyrightbotRegion.y - 45);

							if (cfg::model::enemyOverlay)
								ImGui::Checkbox("wireframe", &cfg::model::enemyOverlayXhair);

							if (cfg::model::enemyOverlayXQZ && cfg::model::enemyOverlay)
								ImGui::Checkbox("wireframe trough wall", &cfg::model::enemyOverlayXQZXhair);
						}
						else if (selectedOverlay == 1) {

							ImGui::Checkbox("enable", &cfg::model::enemyThinOverlay); ImGui::SameLine(); ImGui::SetCursorPosX(enemyrightbotRegion.x - 30);
							ImGui::ColorEdit4("##enemyThinOverlayColor", cfg::model::enemyThinOverlayColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

							if (cfg::model::enemyThinOverlay) {

								ImGui::Checkbox("through wall", &cfg::model::enemyThinOverlayXQZ); ImGui::SameLine(); ImGui::SetCursorPosX(enemyrightbotRegion.x - 30);
								ImGui::ColorEdit4("##enemyThinOverlayXQZColor", cfg::model::enemyThinOverlayXQZColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);
							}

							ImGui::SetCursorPosY(enemyrightbotRegion.y - 45);

							if (cfg::model::enemyThinOverlay)
								ImGui::Checkbox("wireframe", &cfg::model::enemyThinOverlayXhair);

							if (cfg::model::enemyThinOverlayXQZ && cfg::model::enemyThinOverlay)
								ImGui::Checkbox("wireframe trough wall", &cfg::model::enemyThinOverlayXQZXhair);
						}
						else if (selectedOverlay == 2) {

							ImGui::Checkbox("enable", &cfg::model::enemyAnimOverlay); ImGui::SameLine(); ImGui::SetCursorPosX(enemyrightbotRegion.x - 30);
							ImGui::ColorEdit4("##enemyAnimOverlayColor", cfg::model::enemyAnimOverlayColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

							if (cfg::model::enemyAnimOverlay) {

								ImGui::Checkbox("through wall", &cfg::model::enemyAnimOverlayXQZ); ImGui::SameLine(); ImGui::SetCursorPosX(enemyrightbotRegion.x - 30);
								ImGui::ColorEdit4("##enemyAnimOverlayXQZColor", cfg::model::enemyAnimOverlayXQZColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);
							}

							ImGui::SetCursorPosY(enemyrightbotRegion.y - 45);

							if (cfg::model::enemyAnimOverlay)
								ImGui::Checkbox("wireframe", &cfg::model::enemyAnimOverlayXhair);

							if (cfg::model::enemyAnimOverlayXQZ && cfg::model::enemyAnimOverlay)
								ImGui::Checkbox("wireframe trough wall", &cfg::model::enemyAnimOverlayXQZXhair);
						}
					}
					ImGui::EndChild();

					ImGui::SetCursorPos(desiredPlace);

					float childLeft = (ImGui::GetContentRegionAvail().x / 2) - 30;
					ImGui::BeginChild("##enemyleft", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y), true);
					{
						ImGui::Checkbox("bounding box", &cfg::visual::enemyBox); ImGui::SameLine(); ImGui::SetCursorPosX(childLeft);
						ImGui::ColorEdit4("##enemyBoxColor", cfg::visual::enemyBoxColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

						ImGui::Checkbox("glow", &cfg::visual::enemyGlow); ImGui::SameLine(); ImGui::SetCursorPosX(childLeft);
						ImGui::ColorEdit4("##enemyGlowColor", cfg::visual::enemyGlowColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

						ImGui::Checkbox("name", &cfg::visual::enemyName); ImGui::SameLine(); ImGui::SetCursorPosX(childLeft);
						ImGui::ColorEdit4("##enemyNameColor", cfg::visual::enemyNameColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

						ImGui::Checkbox("health", &cfg::visual::enemyHealth); ImGui::SameLine(); ImGui::SetCursorPosX(childLeft);
						ImGui::ColorEdit4("##enemyHealthColor", cfg::visual::enemyHealthColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

						ImGui::Checkbox("armor", &cfg::visual::enemyArmor); ImGui::SameLine(); ImGui::SetCursorPosX(childLeft);
						ImGui::ColorEdit4("##enemyArmorColor", cfg::visual::enemyArmorColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

						ImGui::Checkbox("weapon", &cfg::visual::enemyWeapon); ImGui::SameLine(); ImGui::SetCursorPosX(childLeft);
						ImGui::ColorEdit4("##enemyWeaponColor", cfg::visual::enemyWeaponColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

						//ImGui::Checkbox( "breaking lagcomp", &cfg::visual::enemyBreakLC );

						ImGui::Checkbox("ammo", &cfg::visual::enemyAmmo); ImGui::SameLine(); ImGui::SetCursorPosX(childLeft);
						ImGui::ColorEdit4("##enemyAmmoColor", cfg::visual::enemyAmmoColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

						ImGui::Checkbox("money", &cfg::visual::enemyMoney); ImGui::SameLine(); ImGui::SetCursorPosX(childLeft);
						ImGui::ColorEdit4("##enemyMoneyColor", cfg::visual::enemyMoneyColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);
					}
					ImGui::EndChild();
				}
				if (selectedESP == TEAM) {

					ImVec2 desiredPlace = ImGui::GetCursorPos();

					ImGui::SameLine();
					float otherX = ImGui::GetCursorPosX();
					float rightbotX = ImGui::GetContentRegionAvail().x - 30;
					float rightboxY = ImGui::GetContentRegionAvail().y / 3 - 45;
					ImGui::BeginChild("##teamright", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 3), true);
					{
						static const char* chamsType[] = { "default", "flat" };
						ImGui::Combo("##teamType", &cfg::model::teamType, chamsType, IM_ARRAYSIZE(chamsType));

						ImGui::Checkbox("enable", &cfg::model::team); ImGui::SameLine(); ImGui::SetCursorPosX(rightbotX);
						ImGui::ColorEdit4("##teamColor", cfg::model::teamColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

						if (cfg::model::team) {

							ImGui::Checkbox("through wall", &cfg::model::teamXQZ); ImGui::SameLine(); ImGui::SetCursorPosX(rightbotX);
							ImGui::ColorEdit4("##teamXQZColor", cfg::model::teamXQZColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);
						}

						ImGui::SetCursorPosY(rightboxY);

						if (cfg::model::team)
							ImGui::Checkbox("wireframe", &cfg::model::teamXhair);

						if (cfg::model::teamXQZ && cfg::model::team)
							ImGui::Checkbox("wireframe trough wall", &cfg::model::teamXQZXhair);
					}
					ImGui::EndChild();

					ImGui::SetCursorPosX(otherX);
					ImVec2 teamrightbotRegion = ImGui::GetContentRegionAvail();
					ImGui::BeginChild("##teamrightbot", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true);
					{
						static int selectedOverlay = 0;
						static const char* glowType[] = { "glow", "thin glow", "animated" };
						ImGui::Combo("##glowtype", &selectedOverlay, glowType, IM_ARRAYSIZE(glowType));

						if (selectedOverlay == 0) {

							ImGui::Checkbox("enable", &cfg::model::teamOverlay); ImGui::SameLine(); ImGui::SetCursorPosX(teamrightbotRegion.x - 30);
							ImGui::ColorEdit4("##teamOverlayColor", cfg::model::teamOverlayColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

							if (cfg::model::teamOverlay) {

								ImGui::Checkbox("through wall", &cfg::model::teamOverlayXQZ); ImGui::SameLine(); ImGui::SetCursorPosX(teamrightbotRegion.x - 30);
								ImGui::ColorEdit4("##teamOverlayXQZColor", cfg::model::teamOverlayXQZColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);
							}

							ImGui::SetCursorPosY(teamrightbotRegion.y - 45);

							if (cfg::model::teamOverlay)
								ImGui::Checkbox("wireframe", &cfg::model::teamOverlayXhair);

							if (cfg::model::teamOverlayXQZ && cfg::model::teamOverlay)
								ImGui::Checkbox("wireframe trough wall", &cfg::model::teamOverlayXQZXhair);
						}
						else if (selectedOverlay == 1) {

							ImGui::Checkbox("enable", &cfg::model::teamThinOverlay); ImGui::SameLine(); ImGui::SetCursorPosX(teamrightbotRegion.x - 30);
							ImGui::ColorEdit4("##teamThinOverlayColor", cfg::model::teamThinOverlayColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

							if (cfg::model::teamThinOverlay) {

								ImGui::Checkbox("through wall", &cfg::model::teamThinOverlayXQZ); ImGui::SameLine(); ImGui::SetCursorPosX(teamrightbotRegion.x - 30);
								ImGui::ColorEdit4("##teamThinOverlayXQZColor", cfg::model::teamThinOverlayXQZColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);
							}

							ImGui::SetCursorPosY(teamrightbotRegion.y - 45);

							if (cfg::model::teamThinOverlay)
								ImGui::Checkbox("wireframe", &cfg::model::teamThinOverlayXhair);

							if (cfg::model::teamThinOverlayXQZ && cfg::model::teamThinOverlay)
								ImGui::Checkbox("wireframe trough wall", &cfg::model::teamThinOverlayXQZXhair);
						}
						else if (selectedOverlay == 2) {

							ImGui::Checkbox("enable", &cfg::model::teamAnimOverlay); ImGui::SameLine(); ImGui::SetCursorPosX(teamrightbotRegion.x - 30);
							ImGui::ColorEdit4("##teamAnimOverlayColor", cfg::model::teamAnimOverlayColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

							if (cfg::model::teamAnimOverlay) {

								ImGui::Checkbox("through wall", &cfg::model::teamAnimOverlayXQZ); ImGui::SameLine(); ImGui::SetCursorPosX(teamrightbotRegion.x - 30);
								ImGui::ColorEdit4("##teamAnimOverlayXQZColor", cfg::model::teamAnimOverlayXQZColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);
							}

							ImGui::SetCursorPosY(teamrightbotRegion.y - 45);

							if (cfg::model::teamAnimOverlay)
								ImGui::Checkbox("wireframe", &cfg::model::teamAnimOverlayXhair);

							if (cfg::model::teamAnimOverlayXQZ && cfg::model::teamAnimOverlay)
								ImGui::Checkbox("wireframe trough wall", &cfg::model::teamAnimOverlayXQZXhair);
						}
					}
					ImGui::EndChild();

					ImGui::SetCursorPos(desiredPlace);

					float childLeft = (ImGui::GetContentRegionAvail().x / 2) - 30;
					ImGui::BeginChild("##teamleft", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y), true);
					{
						ImGui::Checkbox("bounding box", &cfg::visual::teamBox); ImGui::SameLine(); ImGui::SetCursorPosX(childLeft);
						ImGui::ColorEdit4("##teamBoxColor", cfg::visual::teamBoxColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

						ImGui::Checkbox("glow", &cfg::visual::teamGlow); ImGui::SameLine(); ImGui::SetCursorPosX(childLeft);
						ImGui::ColorEdit4("##teamGlowColor", cfg::visual::teamGlowColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

						ImGui::Checkbox("name", &cfg::visual::teamName); ImGui::SameLine(); ImGui::SetCursorPosX(childLeft);
						ImGui::ColorEdit4("##teamNameColor", cfg::visual::teamNameColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

						ImGui::Checkbox("health", &cfg::visual::teamHealth); ImGui::SameLine(); ImGui::SetCursorPosX(childLeft);
						ImGui::ColorEdit4("##teamHealthColor", cfg::visual::teamHealthColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

						ImGui::Checkbox("armor", &cfg::visual::teamArmor); ImGui::SameLine(); ImGui::SetCursorPosX(childLeft);
						ImGui::ColorEdit4("##teamArmorColor", cfg::visual::teamArmorColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

						// weapon here lmao

						ImGui::Checkbox("ammo", &cfg::visual::teamAmmo); ImGui::SameLine(); ImGui::SetCursorPosX(childLeft);
						ImGui::ColorEdit4("##teamAmmoColor", cfg::visual::teamAmmoColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

						ImGui::Checkbox("money", &cfg::visual::teamMoney); ImGui::SameLine(); ImGui::SetCursorPosX(childLeft);
						ImGui::ColorEdit4("##teamMoneyColor", cfg::visual::teamMoneyColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);
					}
					ImGui::EndChild();
				}
				if (selectedESP == LOCAL) {

					ImVec2 desiredPlace = ImGui::GetCursorPos();

					ImGui::SameLine();
					float otherX = ImGui::GetCursorPosX();
					float rightbotX = ImGui::GetContentRegionAvail().x - 30;
					float rightboxY = ImGui::GetContentRegionAvail().y / 3 - 45;
					ImGui::BeginChild("##localright", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 3), true);
					{
						static const char* chamsType[] = { "default", "flat" };
						ImGui::Combo("##localType", &cfg::model::localType, chamsType, IM_ARRAYSIZE(chamsType));

						ImGui::Checkbox("enable", &cfg::model::local); ImGui::SameLine(); ImGui::SetCursorPosX(rightbotX);
						ImGui::ColorEdit4("##localColor", cfg::model::localColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

						if (cfg::model::local) {

							ImGui::Checkbox("through wall", &cfg::model::localXQZ); ImGui::SameLine(); ImGui::SetCursorPosX(rightbotX);
							ImGui::ColorEdit4("##localXQZColor", cfg::model::localXQZColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);
						}

						ImGui::SetCursorPosY(rightboxY);

						if (cfg::model::local)
							ImGui::Checkbox("wireframe", &cfg::model::localXhair);

						if (cfg::model::localXQZ && cfg::model::local)
							ImGui::Checkbox("wireframe trough wall", &cfg::model::localXQZXhair);
					}
					ImGui::EndChild();

					ImGui::SetCursorPosX(otherX);
					ImVec2 localrightbotRegion = ImGui::GetContentRegionAvail();
					ImGui::BeginChild("##localrightbot", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true);
					{
						ImGui::Checkbox("localDesync", &cfg::model::localDesync);
						if (cfg::model::localDesync) {

							static const char* chamsType[] = { "default", "flat", "glow", "thin glow", "animated" };
							ImGui::Combo("##localDesyncType", &cfg::model::localDesyncType, chamsType, IM_ARRAYSIZE(chamsType)); ImGui::SameLine(); ImGui::SetCursorPosX(localrightbotRegion.x - 30);
							ImGui::ColorEdit4("##localDesyncColor", cfg::model::localDesyncColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);
							ImGui::Checkbox("wireframe", &cfg::model::localDesyncXhair);
						}

						static int selectedOverlay = 0;
						static const char* glowType[] = { "glow", "thin glow", "animated", "ideal tick"};
						ImGui::Combo("##glowtype", &selectedOverlay, glowType, IM_ARRAYSIZE(glowType));

						if (selectedOverlay == 0) {

							ImGui::Checkbox("enable", &cfg::model::localOverlay); ImGui::SameLine(); ImGui::SetCursorPosX(localrightbotRegion.x - 30);
							ImGui::ColorEdit4("##localOverlayColor", cfg::model::localOverlayColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

							if (cfg::model::localOverlay) {

								ImGui::Checkbox("through wall", &cfg::model::localOverlayXQZ); ImGui::SameLine(); ImGui::SetCursorPosX(localrightbotRegion.x - 30);
								ImGui::ColorEdit4("##localOverlayXQZColor", cfg::model::localOverlayXQZColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);
							}

							ImGui::SetCursorPosY(localrightbotRegion.y - 45);

							if (cfg::model::localOverlay)
								ImGui::Checkbox("wireframe", &cfg::model::localOverlayXhair);

							if (cfg::model::localOverlayXQZ && cfg::model::localOverlay)
								ImGui::Checkbox("wireframe trough wall", &cfg::model::localOverlayXQZXhair);
						}
						else if (selectedOverlay == 1) {

							ImGui::Checkbox("enable", &cfg::model::localThinOverlay); ImGui::SameLine(); ImGui::SetCursorPosX(localrightbotRegion.x - 30);
							ImGui::ColorEdit4("##localThinOverlayColor", cfg::model::localThinOverlayColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

							if (cfg::model::localThinOverlay) {

								ImGui::Checkbox("through wall", &cfg::model::localThinOverlayXQZ); ImGui::SameLine(); ImGui::SetCursorPosX(localrightbotRegion.x - 30);
								ImGui::ColorEdit4("##localThinOverlayXQZColor", cfg::model::localThinOverlayXQZColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);
							}

							ImGui::SetCursorPosY(localrightbotRegion.y - 45);

							if (cfg::model::localThinOverlay)
								ImGui::Checkbox("wireframe", &cfg::model::localThinOverlayXhair);

							if (cfg::model::localThinOverlayXQZ && cfg::model::localThinOverlay)
								ImGui::Checkbox("wireframe trough wall", &cfg::model::localThinOverlayXQZXhair);
						}
						else if (selectedOverlay == 2) {

							ImGui::Checkbox("enable", &cfg::model::localAnimOverlay); ImGui::SameLine(); ImGui::SetCursorPosX(localrightbotRegion.x - 30);
							ImGui::ColorEdit4("##localAnimOverlayColor", cfg::model::localAnimOverlayColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

							if (cfg::model::localAnimOverlay) {

								ImGui::Checkbox("through wall", &cfg::model::localAnimOverlayXQZ); ImGui::SameLine(); ImGui::SetCursorPosX(localrightbotRegion.x - 30);
								ImGui::ColorEdit4("##localAnimOverlayXQZColor", cfg::model::localAnimOverlayXQZColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);
							}

							ImGui::SetCursorPosY(localrightbotRegion.y - 45);

							if (cfg::model::localAnimOverlay)
								ImGui::Checkbox("wireframe", &cfg::model::localAnimOverlayXhair);

							if (cfg::model::localAnimOverlayXQZ && cfg::model::localAnimOverlay)
								ImGui::Checkbox("wireframe trough wall", &cfg::model::localAnimOverlayXQZXhair);
						}
						else if (selectedOverlay == 3) {

							ImGui::Checkbox("enable", &cfg::model::localIdealTick);
							ImGui::ColorEdit4("before", cfg::model::localIdealTickColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);
							ImGui::ColorEdit4("after", cfg::model::localIdealTickColor2, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);
						}
					}
					ImGui::EndChild();

					ImGui::SetCursorPos(desiredPlace);

					float childLeft = (ImGui::GetContentRegionAvail().x / 2) - 30;
					ImGui::BeginChild("##localleft", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y), true);
					{
						ImGui::Checkbox("bounding box", &cfg::visual::localBox); ImGui::SameLine(); ImGui::SetCursorPosX(childLeft);
						ImGui::ColorEdit4("##localBoxColor", cfg::visual::localBoxColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

						ImGui::Checkbox("glow", &cfg::visual::localGlow); ImGui::SameLine(); ImGui::SetCursorPosX(childLeft);
						ImGui::ColorEdit4("##localGlowColor", cfg::visual::localGlowColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

						ImGui::Checkbox("name", &cfg::visual::localName); ImGui::SameLine(); ImGui::SetCursorPosX(childLeft);
						ImGui::ColorEdit4("##localNameColor", cfg::visual::localNameColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

						ImGui::Checkbox("health", &cfg::visual::localHealth); ImGui::SameLine(); ImGui::SetCursorPosX(childLeft);
						ImGui::ColorEdit4("##localHealthColor", cfg::visual::localHealthColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

						ImGui::Checkbox("armor", &cfg::visual::localArmor); ImGui::SameLine(); ImGui::SetCursorPosX(childLeft);
						ImGui::ColorEdit4("##localArmorColor", cfg::visual::localArmorColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

						// weapon here lmao

						ImGui::Checkbox("ammo", &cfg::visual::localAmmo); ImGui::SameLine(); ImGui::SetCursorPosX(childLeft);
						ImGui::ColorEdit4("##localAmmoColor", cfg::visual::localAmmoColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);

						ImGui::Checkbox("money", &cfg::visual::localMoney); ImGui::SameLine(); ImGui::SetCursorPosX(childLeft);
						ImGui::ColorEdit4("##localMoneyColor", cfg::visual::localMoneyColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);
					}
					ImGui::EndChild();
				}
				
			}
			else if (tabindex == MISC_TAB) {
				ImGui::BeginChild("##misc", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y), true);
				{
					ImGui::Checkbox("bhop", &cfg::misc::bunnyhop);
					ImGui::Checkbox("auto strafe", &cfg::misc::autoStrafe);
					ImGui::Checkbox("fast stop", &cfg::misc::faststop);
					ImGui::Checkbox("thirdperson", &cfg::misc::thirdperson);
					if (cfg::misc::thirdperson) {
						ImGui::SameLine();
						ImGui::Keybind("##tpkey", &cfg::misc::thirdpersonbind);
					}
					
					ImGui::Checkbox( "draw server hitbox", &cfg::misc::m_bDrawServerHitbox);
					if ( cfg::misc::m_bDrawServerHitbox );
						ImGui::Checkbox( "draw server hitbox on all entities", &cfg::misc::m_bDrawServerHitboxOnAllEntities );
					
					ImGui::Checkbox("nightmode", &cfg::misc::nightmode); ImGui::SameLine();
					ImGui::ColorEdit4("##nightmodecolor", cfg::misc::nightmodeColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_AlphaBar);
					ImGui::Checkbox("preserve killfeed", &cfg::misc::preserveKillfeed);
					ImGui::Checkbox("bullet impact", &cfg::misc::bulletImpact);
					ImGui::Checkbox("cheat logs only", &cfg::misc::onlyCheatLogs);
					ImGui::Checkbox("aspect ratio", &cfg::misc::aspectRatio);
					if (cfg::misc::aspectRatio)
						ImGui::SliderInt("##ratio", &cfg::misc::aspectRatioValue, 0, 100);
					ImGui::SliderInt("viewmodel fov", &cfg::misc::viewmodelFov, 65, 140);
					ImGui::SliderInt("fov", &cfg::misc::fov, 80, 140);

					ImGui::Checkbox( "fakeping", &cfg::misc::fakePing );
					if ( cfg::misc::fakePing )
						ImGui::SliderFloat( "##fakepingFactor", &cfg::misc::fakePingFactor, 0.f, 200.f, "%.1f ms" );

					static const char* items[] = { "smoke", "flash", "recoil", "zoom" };
					static std::string selectedPreview = "";
					if (ImGui::BeginCombo("removals", selectedPreview.c_str())) {

						selectedPreview = "";
						for (size_t i = 0; i < IM_ARRAYSIZE(items); i++) {

							ImGui::Selectable(items[i], &cfg::misc::removals[i], ImGuiSelectableFlags_DontClosePopups);
						}
						for (size_t i = 0; i < IM_ARRAYSIZE(items); i++) {

							if (cfg::misc::removals[i]) {
								selectedPreview += items[i];
								selectedPreview += " ";
							}
						}
						ImGui::EndCombo();
					}
				}
				ImGui::EndChild();

				ImGui::SameLine();

				ImGui::BeginChild("##config", ImGui::GetContentRegionAvail(), true);
				{
					static const char* item[] = { "semi-rage", "hvh", "baim", "headshot" };
					ImGui::Combo("configs", &cfg::configID, item, IM_ARRAYSIZE(item));

					if (ImGui::ButtonCenteredOnLine("save", 0.5f, true))
						Config2->Save(item[cfg::configID]);

					ImGui::Spacing();

					if (ImGui::ButtonCenteredOnLine("load", 0.5f, true))
						Config2->Load(item[cfg::configID]);

					ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); 
					ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
					ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
					ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
					ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
					ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
					ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

					if (g::pLocal) {
						if (g::pLocal->GetTeam() == TEAM_CT) {
							static const char* pistolsCT[] = { "none", "usp-s / p2000", "dual beretta", "p250", "five-seven / cz-auto", "desert eagle / revolver"};
							static const char* riflesCT[] = { "none", "famas", "m4a1-s / m4a4", "aug"};
							static const char* snipersCT[] = { "none", "ssg08", "awp", "scar-20" };

							static const char* equipmentsCT[] = { "kevlar + helmet", "zeus", "defuse kit"};
							static const char* grenadesCT[] = { "incendiary grenade", "decoy grenade", "flashbang", "he grenade", "smoke grenade"};

							ImGui::Combo("pistols", &cfg::misc::pistols, pistolsCT, IM_ARRAYSIZE(pistolsCT));
							ImGui::Combo("snipers", &cfg::misc::snipers, snipersCT, IM_ARRAYSIZE(snipersCT));

							static std::string selectedequipments = "";
							if (ImGui::BeginCombo("equipments", selectedequipments.c_str())) {

								selectedequipments = "";
								for (size_t i = 0; i < IM_ARRAYSIZE(equipmentsCT); i++) {

									ImGui::Selectable(equipmentsCT[i], &cfg::misc::equipments[i], ImGuiSelectableFlags_DontClosePopups);
								}
								for (size_t i = 0; i < IM_ARRAYSIZE(equipmentsCT); i++) {

									if (cfg::misc::equipments[i]) {
										selectedequipments += equipmentsCT[i];
										selectedequipments += " ";
									}
								}
								ImGui::EndCombo();
							}
							static std::string selectedgrenades = "";
							if (ImGui::BeginCombo("grenades", selectedgrenades.c_str())) {

								selectedgrenades = "";
								for (size_t i = 0; i < IM_ARRAYSIZE(grenadesCT); i++) {

									ImGui::Selectable(grenadesCT[i], &cfg::misc::grenades[i], ImGuiSelectableFlags_DontClosePopups);
								}
								for (size_t i = 0; i < IM_ARRAYSIZE(grenadesCT); i++) {

									if (cfg::misc::grenades[i]) {
										selectedgrenades += grenadesCT[i];
										selectedgrenades += " ";
									}
								}
								ImGui::EndCombo();
							}

						}
						if (g::pLocal->GetTeam() == TEAM_TT) {
							static const char* pistols[] = { "none", "glock", "dual beretta", "p250", "tec9 / cz-auto", "desert eagle / revolver" };
							static const char* rifles[] = { "none", "galil ar", "ak47", "sg 553" };
							static const char* snipers[] = { "none", "ssg08", "awp", "g3sg1" };

							static const char* equipments[] = { "kevlar + helmet", "zeus" };
							static const char* grenades[] = { "molotov", "decoy grenade", "flashbang", "he grenade", "smoke grenade" };

							ImGui::Combo("pistols", &cfg::misc::pistols, pistols, IM_ARRAYSIZE(pistols));
							ImGui::Combo("snipers", &cfg::misc::snipers, snipers, IM_ARRAYSIZE(snipers));

							static std::string selectedequipments = "";
							if (ImGui::BeginCombo("equipments", selectedequipments.c_str())) {

								selectedequipments = "";
								for (size_t i = 0; i < IM_ARRAYSIZE(equipments); i++) {

									ImGui::Selectable(equipments[i], &cfg::misc::equipments[i], ImGuiSelectableFlags_DontClosePopups);
								}
								for (size_t i = 0; i < IM_ARRAYSIZE(equipments); i++) {

									if (cfg::misc::equipments[i]) {
										selectedequipments += equipments[i];
										selectedequipments += " ";
									}
								}
								ImGui::EndCombo();
							}
							static std::string selectedgrenades = "";
							if (ImGui::BeginCombo("grenades", selectedgrenades.c_str())) {

								selectedgrenades = "";
								for (size_t i = 0; i < IM_ARRAYSIZE(grenades); i++) {

									ImGui::Selectable(grenades[i], &cfg::misc::grenades[i], ImGuiSelectableFlags_DontClosePopups);
								}
								for (size_t i = 0; i < IM_ARRAYSIZE(grenades); i++) {

									if (cfg::misc::grenades[i]) {
										selectedgrenades += grenades[i];
										selectedgrenades += " ";
									}
								}
								ImGui::EndCombo();
							}
						}
						else {

						}
					}
					
				}
				ImGui::EndChild();
			}
			else if (tabindex == ANTIAIM_TAB) {

				ImGui::BeginChild("##antiaimleft", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y / 2), true);
				{
					ImGui::Checkbox("enable", &cfg::antiaim::enabled);

					static const char* yaw[] = { "forward", "backward" };
					static const char* pitch[] = { "up", "zero" ,"down" };
					static const char* desync[] = { "off", "static", "jitter" };

					ImGui::Combo("##pitch", &cfg::antiaim::pitch, pitch, IM_ARRAYSIZE(pitch));

					ImGui::Checkbox("at target", &cfg::antiaim::atTarget);

					ImGui::Combo("##yaw", &cfg::antiaim::yaw, yaw, IM_ARRAYSIZE(yaw));

					ImGui::Combo("##desync", &cfg::antiaim::desynctype, desync, IM_ARRAYSIZE(desync));

					if (cfg::antiaim::desynctype) {
						ImGui::SameLine();
						ImGui::Keybind("##inverter", &cfg::antiaim::desyncinverter);

						ImGui::SliderFloat("range", &cfg::antiaim::desyncvalue, 0.f, 58.f, std::to_string((int)cfg::antiaim::desyncvalue).c_str());
					}
				}
				ImGui::EndChild();

				ImVec2 vecChildPosition = ImGui::GetCursorPos();

				ImGui::SameLine();

				ImGui::BeginChild("##antiaimright", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true);
				{
					ImGui::SliderInt("jitter", &cfg::antiaim::jittervalue, 0, 90, std::to_string((int)cfg::antiaim::jittervalue).c_str());
					ImGui::Checkbox("fakeduck", &cfg::antiaim::fakeduck);
					if (cfg::antiaim::fakeduck) {
						ImGui::SameLine();
						ImGui::Keybind("##fakeduckbind", &cfg::antiaim::fakeduckbind);
					}

					ImGui::Checkbox("ideal tick", &cfg::antiaim::idealTick);
					ImGui::Keybind("##idealtickbind", &cfg::antiaim::idealTickBind);
				}
				ImGui::EndChild();

				ImGui::SetCursorPos(vecChildPosition);

				ImGui::BeginChild("##antiaimleftbot", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y), true);
				{
					static const char* fakelagtype[] = { "maximum", "adaptive", "jitter" };

					ImGui::Combo("##fakelagtype", &cfg::antiaim::fakelagType, fakelagtype, IM_ARRAYSIZE(fakelagtype));
					ImGui::SliderInt("base", &cfg::antiaim::fakelag, 0, GetAsyncKeyState(VK_RSHIFT) ? 64 : 14, std::to_string(cfg::antiaim::fakelag).c_str());
					ImGui::SliderInt("minimum", &cfg::antiaim::fakelagmin, 0, GetAsyncKeyState(VK_RSHIFT) ? 64 : 14, std::to_string(cfg::antiaim::fakelagmin).c_str());
					ImGui::SliderInt("maximum", &cfg::antiaim::fakelagmax, 0, GetAsyncKeyState(VK_RSHIFT) ? 64 : 14, std::to_string(cfg::antiaim::fakelagmax).c_str());
					ImGui::Spacing(); ImGui::Spacing();
					ImGui::Checkbox("defensive", &cfg::antiaim::defensive);
				}
				ImGui::EndChild();
			}
			else if (tabindex == RAGE_TAB) {

				ImGui::BeginChild("##rageleft", ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y), true);
				{
					ImGui::Checkbox("enable", &cfg::rage::enable);
					ImGui::Checkbox("auto stop", &cfg::rage::autostop);
					if (cfg::rage::autostop) {
						ImGui::SameLine();
						ImGui::Checkbox("always", &cfg::rage::betweenshots);
						ImGui::Checkbox( "in air", &cfg::rage::m_bAutoStopInAir );
					}
					ImGui::Checkbox("resolver", &cfg::rage::resolver);
					//ImGui::Checkbox("aimstep", &cfg::rage::aimstep);
					ImGui::Checkbox("doubletap", &cfg::rage::doubletap);
					ImGui::Keybind("##doubletapkey", &cfg::rage::doubletapkey);

					static const char* item[] = { "auto", "scout", "awp", "pistol", "heavy pistol", "other" };
					static int selectedWeapon = 0;
					ImGui::Combo("##e", &selectedWeapon, item, IM_ARRAYSIZE(item));

					static const char* items[] = { "head", "upper chest", "lower chest", "stomach", "arms", "legs" };

					if (selectedWeapon == AUTO) {

						static std::string selectedPreview = "";
						if (ImGui::BeginCombo("hitboxes", selectedPreview.c_str())) {

							selectedPreview = "";
							for (size_t i = 0; i < IM_ARRAYSIZE(items); i++) {

								ImGui::Selectable(items[i], &cfg::rage::autoHitboxes[i], ImGuiSelectableFlags_DontClosePopups);
							}
							for (size_t i = 0; i < IM_ARRAYSIZE(items); i++) {

								if (cfg::rage::autoHitboxes[i]) {
									selectedPreview += items[i];
									selectedPreview += " ";
								}
							}
							ImGui::EndCombo();
						}

						ImGui::SliderInt("hitchance", &cfg::rage::autoHitchance, 0, 100);
						ImGui::SliderInt("damage", &cfg::rage::autoMindmg, 0, 110);

						ImGui::SliderInt("head scale", &cfg::rage::autoHeadPoints, 0, 100);
						ImGui::SliderInt("body scale", &cfg::rage::autoBodyPoints, 0, 100);

						ImGui::Checkbox( "auto-scope", &cfg::rage::autoscope[ AUTO ] );
					}
					else if (selectedWeapon == SCOUT) {

						static std::string selectedPreview = "";
						if (ImGui::BeginCombo("hitboxes", selectedPreview.c_str())) {

							selectedPreview = "";
							for (size_t i = 0; i < IM_ARRAYSIZE(items); i++) {

								ImGui::Selectable(items[i], &cfg::rage::scoutHitboxes[i], ImGuiSelectableFlags_DontClosePopups);
							}
							for (size_t i = 0; i < IM_ARRAYSIZE(items); i++) {

								if (cfg::rage::scoutHitboxes[i]) {
									selectedPreview += items[i];
									selectedPreview += " ";
								}
							}
							ImGui::EndCombo();
						}

						ImGui::SliderInt("hitchance", &cfg::rage::scoutHitchance, 0, 100);
						ImGui::SliderInt("damage", &cfg::rage::scoutMindmg, 0, 110);

						ImGui::SliderInt("head scale", &cfg::rage::scoutHeadPoints, 0, 100);
						ImGui::SliderInt("body scale", &cfg::rage::scoutBodyPoints, 0, 100);

						ImGui::Checkbox( "auto-scope", &cfg::rage::autoscope[ SCOUT ] );
					}
					else if (selectedWeapon == AWP) {

						static std::string selectedPreview = "";
						if (ImGui::BeginCombo("hitboxes", selectedPreview.c_str())) {

							selectedPreview = "";
							for (size_t i = 0; i < IM_ARRAYSIZE(items); i++) {

								ImGui::Selectable(items[i], &cfg::rage::awpHitboxes[i], ImGuiSelectableFlags_DontClosePopups);
							}
							for (size_t i = 0; i < IM_ARRAYSIZE(items); i++) {

								if (cfg::rage::awpHitboxes[i]) {
									selectedPreview += items[i];
									selectedPreview += " ";
								}
							}
							ImGui::EndCombo();
						}

						ImGui::SliderInt("hitchance", &cfg::rage::awpHitchance, 0, 100);
						ImGui::SliderInt("damage", &cfg::rage::awpMindmg, 0, 110);

						ImGui::SliderInt("head scale", &cfg::rage::awpHeadPoints, 0, 100);
						ImGui::SliderInt("body scale", &cfg::rage::awpBodyPoints, 0, 100);

						ImGui::Checkbox( "auto-scope", &cfg::rage::autoscope[ AWP ] );
					}
					else if (selectedWeapon == PISTOL) {

						static std::string selectedPreview = "";
						if (ImGui::BeginCombo("hitboxes", selectedPreview.c_str())) {

							selectedPreview = "";
							for (size_t i = 0; i < IM_ARRAYSIZE(items); i++) {

								ImGui::Selectable(items[i], &cfg::rage::pistolHitboxes[i], ImGuiSelectableFlags_DontClosePopups);
							}
							for (size_t i = 0; i < IM_ARRAYSIZE(items); i++) {

								if (cfg::rage::pistolHitboxes[i]) {
									selectedPreview += items[i];
									selectedPreview += " ";
								}
							}
							ImGui::EndCombo();
						}

						ImGui::SliderInt("hitchance", &cfg::rage::pistolHitchance, 0, 100);
						ImGui::SliderInt("damage", &cfg::rage::pistolMindmg, 0, 110);

						ImGui::SliderInt("head scale", &cfg::rage::pistolHeadPoints, 0, 100);
						ImGui::SliderInt("body scale", &cfg::rage::pistolBodyPoints, 0, 100);
					}
					else if (selectedWeapon == HEAVY_PISTOL) {

						static std::string selectedPreview = "";
						if (ImGui::BeginCombo("hitboxes", selectedPreview.c_str())) {

							selectedPreview = "";
							for (size_t i = 0; i < IM_ARRAYSIZE(items); i++) {

								ImGui::Selectable(items[i], &cfg::rage::heavypistolHitboxes[i], ImGuiSelectableFlags_DontClosePopups);
							}
							for (size_t i = 0; i < IM_ARRAYSIZE(items); i++) {

								if (cfg::rage::heavypistolHitboxes[i]) {
									selectedPreview += items[i];
									selectedPreview += " ";
								}
							}
							ImGui::EndCombo();
						}

						ImGui::SliderInt("hitchance", &cfg::rage::heavypistolHitchance, 0, 100);
						ImGui::SliderInt("damage", &cfg::rage::heavypistolMindmg, 0, 110);

						ImGui::SliderInt("head scale", &cfg::rage::heavypistolHeadPoints, 0, 100);
						ImGui::SliderInt("body scale", &cfg::rage::heavypistolBodyPoints, 0, 100);
					}
					else if (selectedWeapon == OTHER) {

						static std::string selectedPreview = "";
						if (ImGui::BeginCombo("hitboxes", selectedPreview.c_str())) {

							selectedPreview = "";
							for (size_t i = 0; i < IM_ARRAYSIZE(items); i++) {

								ImGui::Selectable(items[i], &cfg::rage::etcHitboxes[i], ImGuiSelectableFlags_DontClosePopups);
							}
							for (size_t i = 0; i < IM_ARRAYSIZE(items); i++) {

								if (cfg::rage::etcHitboxes[i]) {
									selectedPreview += items[i];
									selectedPreview += " ";
								}
							}
							ImGui::EndCombo();
						}

						ImGui::SliderInt("hitchance", &cfg::rage::etcHitchance, 0, 100);
						ImGui::SliderInt("damage", &cfg::rage::etcMindmg, 0, 110);

						ImGui::SliderInt("head scale", &cfg::rage::etcHeadPoints, 0, 100);
						ImGui::SliderInt("body scale", &cfg::rage::etcBodyPoints, 0, 100);
					}
				}
				ImGui::EndChild();

				ImGui::SameLine();

				ImGui::BeginChild("##rageright", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true);
				{

				}
				ImGui::EndChild();
			}

		}
		ImGui::End();
	}

	static int height = 30;
	if (GetKeyState(cfg::misc::thirdpersonbind) || GetKeyState(cfg::antiaim::desyncinverter) || (GetAsyncKeyState(cfg::antiaim::fakeduckbind) && cfg::antiaim::fakeduck) || (cfg::rage::doubletap && GetKeyState(cfg::rage::doubletapkey))) {
		ImGui::SetNextWindowSizeConstraints(ImVec2(130, height), ImVec2(130, height));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(130, height));
		ImGui::Begin("##keystuff", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
		{
			height = 30;
			ImGui::PopStyleVar();

			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(222.f / 255.f, 153.f / 255.f, 42.f / 255.f, 1.f));
			ImGui::BeginChild("##binds", ImGui::GetContentRegionAvail(), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			{
				ImGui::PopStyleColor();
				// cfg::misc::thirdpersonbind
				// cfg::antiaim::desyncinverter
				// cfg::antiaim::fakeduckbind
				// cfg::rage::doubletapkey
				if (GetKeyState(cfg::misc::thirdpersonbind)) {
					ImGui::Text("thirdperson");
					height += 20;
				}
				if (GetKeyState(cfg::antiaim::desyncinverter)) {
					ImGui::Text("invert");
					height += 20;
				}
				if (GetAsyncKeyState(cfg::antiaim::fakeduckbind) && cfg::antiaim::fakeduck) {
					ImGui::Text("fakeduck");
					height += 20;
				}
				if (cfg::rage::doubletap && GetKeyState(cfg::rage::doubletapkey)) {
					ImGui::Text("shifting");
					height += 20;
				}
			}
			ImGui::EndChild();
		}
		ImGui::End();
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

bool menu::SetupWindowClass(const char* szWindowClassName) noexcept {

	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW;
	windowClass.lpfnWndProc = DefWindowProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandle(NULL);
	windowClass.hIcon = NULL;
	windowClass.hCursor = NULL;
	windowClass.hbrBackground = NULL;
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = szWindowClassName;
	windowClass.hIconSm = NULL;

	if (!RegisterClassEx(&windowClass))
		return false;

	return true;
}

void menu::DestroyWindowClass() noexcept {

	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

bool menu::SetupWindow(const char* szWindowName) noexcept {

	//                    lpClassName,				lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam
	window = CreateWindow(windowClass.lpszClassName, szWindowName, WS_OVERLAPPEDWINDOW, 0, 0, 500, 700, 0, 0, windowClass.hInstance, 0);

	if (!window)
		return false;

	return true;
}

void menu::DestroyWindow() noexcept {

	if (window)
		DestroyWindow(window);
}

bool menu::SetupDirectX() noexcept {

	const auto handle = GetModuleHandle("d3d9.dll");

	if (!handle)
		return false;

	using Fn = LPDIRECT3D9(__stdcall*)(UINT);
	const auto create = reinterpret_cast<Fn>(GetProcAddress(handle, "Direct3DCreate9"));

	if (!create)
		return false;

	d3d9 = create(D3D_SDK_VERSION);

	if (!d3d9)
		return false;

	D3DPRESENT_PARAMETERS parameters = {};
	parameters.BackBufferWidth = 0;
	parameters.BackBufferHeight = 0;
	parameters.BackBufferFormat = D3DFMT_UNKNOWN;
	parameters.BackBufferCount = 0;
	parameters.MultiSampleType = D3DMULTISAMPLE_NONE;
	parameters.MultiSampleQuality = NULL;
	parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	parameters.hDeviceWindow = window;
	parameters.Windowed = 1;
	parameters.EnableAutoDepthStencil = 0;
	parameters.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
	parameters.Flags = NULL;
	parameters.FullScreen_RefreshRateInHz = 0;
	parameters.PresentationInterval = 0;

	if (d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, window, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT, &parameters, &device) < 0)
		return false;

	return true;
}

void menu::DestroyDirectX() noexcept {

	if (device) {
		device->Release();
		device = NULL;
	}

	if (d3d9) {
		d3d9->Release();
		d3d9 = NULL;
	}
}

void menu::Setup() {

	if (!SetupWindowClass("CoolHack001"))
		throw std::runtime_error("Failed to create window class");

	if (!SetupWindow("CoolHack Window"))
		throw std::runtime_error("Failed to create window");

	if (!SetupDirectX())
		throw std::runtime_error("Failed to create directx");

	DestroyWindow();
	DestroyWindowClass();
}

void menu::SetupMenu(LPDIRECT3DDEVICE9 device) noexcept {

	auto parameters = D3DDEVICE_CREATION_PARAMETERS{};
	device->GetCreationParameters(&parameters);

	window = parameters.hFocusWindow;

	originalWindowProcess = reinterpret_cast<WNDPROC>(SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProcess)));

	ImGui::CreateContext();
	//ImGui::StyleColorsClassic();

	Style();
	//NewStyle();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);

	setup = true;
}

void menu::Destroy() noexcept {

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(originalWindowProcess));

	DestroyDirectX();
}
