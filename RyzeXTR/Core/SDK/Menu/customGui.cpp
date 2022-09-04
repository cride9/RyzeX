#include "customGui.h"

bool customGUI::Checkbox(const char* label, bool* v) {

	ImGuiWindow* pImGuiWindow = ImGui::GetCurrentWindow();
	if (pImGuiWindow->SkipItems)
		return false;

	ImGuiContext& g = *GImGui; // apparently YOU CANNOT RENAME THIS

	const ImGuiStyle& rStyle = g.Style;

	const ImGuiID ID = pImGuiWindow->GetID(label);
	const ImVec2 vecLabelSize = ImGui::CalcTextSize(label, NULL, true);
	const float flFrameHeight = ImGui::GetFrameHeight();
	const ImVec2 vecPosition = pImGuiWindow->DC.CursorPos;

	const ImRect vecTotalBB(vecPosition, vecPosition + ImVec2(flFrameHeight + (vecLabelSize.x > 0.0f ? rStyle.ItemInnerSpacing.x + vecLabelSize.x : 0.0f), vecLabelSize.y + rStyle.FramePadding.y * 2.0f));
	
	ImGui::ItemSize(vecTotalBB, rStyle.FramePadding.y);

	if (!ImGui::ItemAdd(vecTotalBB, ID))
	{
		IMGUI_TEST_ENGINE_ITEM_INFO(ID, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_::ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_::ImGuiItemStatusFlags_Checked : 0));
		return false;
	}

	bool bHovered, bHeld;
	bool bPressed = ImGui::ButtonBehavior(vecTotalBB, ID, &bHovered, &bHeld);

	if (bPressed)
	{
		*v = !(*v);
		ImGui::MarkItemEdited(ID);
	}

	const ImRect vecCheckBB(vecPosition, vecPosition + ImVec2(flFrameHeight, flFrameHeight));
	
	ImGui::RenderNavHighlight(vecTotalBB, ID);
	ImGui::RenderFrame(vecCheckBB.Min, vecCheckBB.Max, ImGui::GetColorU32((bHeld && bHovered) ? ImGuiCol_FrameBgActive : bHovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), true, rStyle.FrameRounding);
	
	ImU32 colColor = ImGui::GetColorU32(ImGuiCol_CheckMark);

	bool bMixedValue = (g.LastItemData.InFlags & ImGuiItemFlags_MixedValue) != 0;

	if (bMixedValue) {

		ImVec2 pad(ImMax(1.0f, IM_FLOOR(flFrameHeight / 3.6f)), ImMax(1.0f, IM_FLOOR(flFrameHeight / 3.6f)));
		pImGuiWindow->DrawList->AddRectFilled(vecCheckBB.Min + pad, vecCheckBB.Max - pad, colColor, rStyle.FrameRounding);
	}
	else if (*v) {

		const float flCheckSize = ImMin(vecCheckBB.GetWidth(), vecCheckBB.GetHeight());
		const float flPad = ImMax(1.0f, (float)(int)(flCheckSize));

		pImGuiWindow->DrawList->AddRectFilled(vecCheckBB.Min + ImVec2(flPad - 2, flPad - 2), vecCheckBB.Max - ImVec2(flPad - 2, flPad - 2), ImGui::GetColorU32(ImGuiCol_CheckMark), rStyle.FrameRounding);

		//const float pad = ImMax(1.0f, IM_FLOOR(square_sz / 6.0f));
		//RenderCheckMark(window->DrawList, check_bb.Min + ImVec2(pad, pad), check_col, square_sz - pad * 2.0f);
	}

	ImVec2 vecLabelPos = ImVec2(vecCheckBB.Max.x + rStyle.ItemInnerSpacing.x, vecCheckBB.Min.y + rStyle.FramePadding.y);

	if (g.LogEnabled)
		ImGui::LogRenderedText(&vecLabelPos, bMixedValue ? "[~]" : *v ? "[x]" : "[ ]");

	if (vecLabelSize.x > 0.0f)
		ImGui::RenderText(vecLabelPos, label);

	IMGUI_TEST_ENGINE_ITEM_INFO(ID, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
	return bPressed;
}