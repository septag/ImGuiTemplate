#pragma once

#include <time.h>

#include "imgui.h"
#include "IconsFontAwesome4.h"

struct Fonts
{
    ImFont* uiFont;
    ImFont* uiLargeFont;
    ImFont* monoFont;
    float uiFontSize;
    float uiLargeFontSize;
    float monoFontSize;
};

namespace ImGui
{
    bool MyInitialize();
    void MyRelease();

    bool LoadFonts(float dpiScale);
    const Fonts& GetFonts();
    void SaveState();
    void BeginFrame();

    bool BeginMainToolbar(float height);
    void EndMainToolbar();

    namespace internal
    {
        void DisableSkipItems();
    }

    template <typename F> void Align(float align, const F& f) 
    {
        const ImVec2 containerSize = ImGui::GetContentRegionAvail();
        const ImVec2 cp = ImGui::GetCursorScreenPos();
        ImGuiStyle& style = ImGui::GetStyle();
        float alpha_backup = style.DisabledAlpha;
        style.DisabledAlpha = 0;
        ImGui::BeginDisabled();
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking;
        const char* id = "imgui_measure__";
        ImGui::Begin(id, nullptr, flags);
        ImGui::internal::DisableSkipItems();
        
        //
        ImGui::BeginGroup();
        f();
        ImGui::EndGroup();
        const ImVec2 size = ImGui::GetItemRectSize();
        ImGui::End();
        ImGui::EndDisabled();
        style.DisabledAlpha = alpha_backup;
        ImGui::SetCursorScreenPos(ImVec2(cp.x + (containerSize.x - size.x) * align, cp.y));
        f();
    }

    template <typename F> void AlignRight(const F& f) { Align(1, f); }
    template <typename F> void AlignCenter(const F& f) { Align(0.5f, f); }

    void SpinnerAng(const char *label, float radius, float thickness, const ImColor &color, const ImColor &bg, float speed, float angle, int mode);

    void SeparatorVertical(float thickness = 1.0f);
    bool ToggleButton(const char* label, bool* toggled, const ImVec2& size_arg = ImVec2(0, 0));
}
