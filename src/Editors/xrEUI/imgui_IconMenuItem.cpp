#include "stdafx.h"
#include "imgui_internal.h"
#include "imgui_IconMenuItem.h"

bool IXIconMenuItem(const char* label, ImTextureID icon, const ImVec2& imageSize, const char* shortcut, bool selected, bool enabled)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    ImGuiStyle& style = g.Style;

    //
    const ImVec4& bg_col = ImVec4(0, 0, 0, 0);
    const int frame_padding = -1;
    const ImVec2& uv0 = ImVec2(0, 0);
    const ImVec2& uv1 = ImVec2(1, 1);
    const ImVec4& tint_col = ImVec4(1, 1, 1, 1);
    //

    ImVec2 size = imageSize;
    if (size.x <= 0 && size.y <= 0) { size.x = size.y = ImGui::GetTextLineHeightWithSpacing(); }
    else {
        if (size.x <= 0)          size.x = size.y;
        else if (size.y <= 0)     size.y = size.x;
        size *= window->FontWindowScale * ImGui::GetIO().FontGlobalScale;
    }

    const ImVec2 textSize = ImGui::CalcTextSize(label, NULL, true);
    const bool hasText = textSize.x > 0;
    const float innerSpacing = hasText ? ((frame_padding >= 0) ? (float)frame_padding : (style.ItemInnerSpacing.x)) : 0.f;
    const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
    const ImVec2 totalSizeWithoutPadding(size.x + innerSpacing + textSize.x, size.y > textSize.y ? size.y : textSize.y);
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + totalSizeWithoutPadding + padding * 2);
    ImVec2 start(0, 0);
    start = window->DC.CursorPos + padding; if (size.y < textSize.y) start.y += (textSize.y - size.y) * .5f;
    const ImRect image_bb(start, start + size);
    start = window->DC.CursorPos + padding; start.x += size.x + innerSpacing; if (size.y > textSize.y) start.y += (size.y - textSize.y) * .5f;
    ImGui::ItemSize(bb);


    // We've been using the equivalent of ImGuiSelectableFlags_SetNavIdOnHover on all Selectable() since early Nav system days (commit 43ee5d73),
    // but I am unsure whether this should be kept at all. For now moved it to be an opt-in feature used by menus only.
    bool pressed;
    ImGui::PushID(label);
    if (!enabled)
        ImGui::BeginDisabled();

    // We use ImGuiSelectableFlags_NoSetKeyOwner to allow down on one menu item, move, up on another.
    const ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SelectOnRelease | ImGuiSelectableFlags_NoSetKeyOwner | ImGuiSelectableFlags_SetNavIdOnHover;
    const ImGuiMenuColumns* offsets = &window->DC.MenuColumns;

    {

        float w = textSize.x + image_bb.Max.x - image_bb.Min.x + padding.x;

        ImGui::SetCursorPosX(image_bb.Min.x);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x * 2.0f, style.ItemSpacing.y));
        pressed = ImGui::Selectable("", selected, selectable_flags, ImVec2(w, 0.0f));
        ImGui::PopStyleVar();
        if (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_Visible) {
            //ImGui::RenderText(text_pos, label);

            window->DrawList->AddImage(icon, image_bb.Min, image_bb.Max, uv0, uv1, ImGui::GetColorU32(tint_col));
            start.y = window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset;
            ImGui::RenderText(start, label);
        }
        window->DC.CursorPos.x += IM_TRUNC(style.ItemSpacing.x * (-1.0f + 0.5f)); // -1 spacing to compensate the spacing added when Selectable() did a SameLine(). It would also work to call SameLine() ourselves after the PopStyleVar().
    }

    IMGUI_TEST_ENGINE_ITEM_INFO(g.LastItemData.ID, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (selected ? ImGuiItemStatusFlags_Checked : 0));
    if (!enabled)
        ImGui::EndDisabled();
    ImGui::PopID();

    return pressed;
}

