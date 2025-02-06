#include "../Core.h"
#include "../Main.h"

#include "ImGuiMain.h"
#include "FontAwesome.h"
#include "imgui_internal.h"

#include <time.h>

struct ImGuiContextExtra
{
    Fonts fonts;
    void* fontTexture;
    bool init;
};

static ImGuiContextExtra gImGuiExtra;

#if PLATFORM_OSX
static constexpr const char* kOsxFontsDir = "/System/Library/Fonts";
#endif

static void SetImGuiTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsDark(&style);
    
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    
    style.ScrollbarSize = 14;
    style.GrabMinSize = 14;
    style.WindowBorderSize = 1;
    style.ChildBorderSize = 0;
    style.PopupBorderSize = 0;
    style.FrameBorderSize = 0;
    style.TabBorderSize = 0;
    
    style.WindowRounding = 0;
    style.ChildRounding = 3;
    style.FrameRounding = 3;
    style.PopupRounding = 3;
    style.ScrollbarRounding = 1;
    style.GrabRounding = 1;
    style.TabRounding = 2;
    
    style.AntiAliasedFill = true;
    style.AntiAliasedLines = true;
    
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 0.89f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(1.00f, 1.00f, 1.00f, 0.39f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_Border]                 = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(1.00f, 1.00f, 1.00f, 0.08f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(1.00f, 1.00f, 1.00f, 0.12f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.25f, 0.43f, 0.76f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.26f, 0.45f, 0.80f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(1.00f, 1.00f, 1.00f, 0.55f);
    colors[ImGuiCol_Header]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.06f, 0.06f, 0.06f, 0.39f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(1.00f, 1.00f, 1.00f, 0.16f);
    colors[ImGuiCol_Separator]              = ImVec4(1.00f, 1.00f, 1.00f, 0.15f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.25f, 0.43f, 0.76f, 1.00f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.25f, 0.43f, 0.76f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.49f, 0.49f, 0.49f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.95f, 0.95f, 0.95f, 0.31f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.25f, 0.43f, 0.76f, 1.00f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.15f, 0.26f, 0.47f, 1.00f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.25f, 0.43f, 0.76f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_DockingPreview]         = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(0.80f, 0.47f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.80f, 0.47f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.89f, 0.62f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.16f, 0.27f, 0.49f, 1.00f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 0.86f, 0.00f, 0.86f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.80f, 0.47f, 0.00f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.71f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

static Path GetImGuiSettingsFilepath()
{
    Path myDir;
    OS::GetMyPath(myDir.Ptr(), myDir.Capacity());
    myDir = myDir.GetDirectory();
    ASSERT(myDir.IsDir());
    return Path::Join(myDir, CONFIG_IMGUI_SETTINGS_FILENAME);
}

bool ImGui::MyInitialize()
{
    ImGui::SetAllocatorFunctions(
        [](size_t size, void*)->void* { return Mem::Alloc(size); },
        [](void* ptr, void*) { Mem::Free(ptr); });

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;
    //io.ConfigViewportsNoDefaultParent = true;
    //io.ConfigDockingAlwaysTabBar = true;
    //io.ConfigDockingTransparentPayload = true;
    //io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;     // FIXME-DPI: Experimental. THIS CURRENTLY DOESN'T WORK AS EXPECTED. DON'T USE IN USER APP!
    io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports; // FIXME-DPI: Experimental.

    // Setup Dear ImGui context
    io.IniFilename = nullptr;

    ImGui::LoadIniSettingsFromDisk(GetImGuiSettingsFilepath().CStr());

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();
    SetImGuiTheme();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    gImGuiExtra.init = true;

    return true;
}

void ImGui::MyRelease()
{
    DestroyTexture(gImGuiExtra.fontTexture);
}

const Fonts& ImGui::GetFonts()
{
    return gImGuiExtra.fonts;
}

bool ImGui::LoadFonts(float dpiScale)
{
    #if PLATFORM_WINDOWS
        Path fontsDir;
        OS::Win32GetFolder(OSWin32Folder::Fonts, fontsDir.Ptr(), sizeof(fontsDir));
        Path defaultFontPath = Path::Join(fontsDir, "Verdana.ttf");
        Path defaultMonoFontPath = Path::Join(fontsDir, "consola.ttf");
        float fontSize = 15.0f;
        float monoFontSize = 14.0f;
    #elif PLATFORM_OSX
        Path fontsDir(kOsxFontsDir);
        Path defaultFontPath = Path::Join(fontsDir, "Geneva.ttf");
        Path defaultMonoFontPath = Path::Join(fontsDir, "SFNSMono.ttf");
        float fontSize = 15.0f;
        float monoFontSize = 15.0f;
    #else
        #error "Not implemented"
    #endif

    // TODO: for now, we rescale fonts to match the DPI
    //       But moving to another monitor currently, doesn't work as expected

    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig fontConfig;

    {
        gImGuiExtra.fonts.uiFont = io.Fonts->AddFontFromFileTTF(defaultFontPath.CStr(), fontSize*dpiScale, &fontConfig);
        gImGuiExtra.fonts.uiFontSize = fontSize;
        
        fontConfig.MergeMode = true;
        // fontConfig.GlyphMinAdvanceX = fontSize;
        static const ImWchar iconRanges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
        io.Fonts->AddFontFromMemoryCompressedTTF(FontAwesome_compressed_data, FontAwesome_compressed_size, fontSize*dpiScale, &fontConfig, iconRanges);
        io.Fonts->Build();
    }

    {
        fontConfig = {};
        gImGuiExtra.fonts.uiLargeFont = io.Fonts->AddFontFromFileTTF(defaultFontPath.CStr(), fontSize*dpiScale*1.5f, &fontConfig);
        gImGuiExtra.fonts.uiLargeFontSize = fontSize*1.5f;
    }
     
    {
        fontConfig = {};
        // fontConfig.GlyphMinAdvanceX = kDefaultMonoFontSize;
        gImGuiExtra.fonts.monoFont = io.Fonts->AddFontFromFileTTF(defaultMonoFontPath.CStr(), monoFontSize*dpiScale, &fontConfig);
        gImGuiExtra.fonts.monoFontSize = monoFontSize;
    }

    uint8* fontPixels;
    int fontWidth, fontHeight, fontBpp;
    io.Fonts->GetTexDataAsRGBA32(&fontPixels, &fontWidth, &fontHeight, &fontBpp);
    
    void* texture = CreateRGBATexture(fontWidth, fontHeight, fontPixels);
    io.Fonts->SetTexID((ImTextureID)texture);

    gImGuiExtra.fontTexture = texture;

    return true;
}

void ImGui::BeginFrame()
{
    if (!gImGuiExtra.init)
        return;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    if (!viewport)
        return;

    ImGuiID id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    
    static bool firstTime = true;
    if (firstTime) {

        ImGui::DockBuilderRemoveNode(id);
        ImGui::DockBuilderAddNode(id, ImGuiDockNodeFlags_PassthruCentralNode);

        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::DockBuilderSetNodeSize(id, viewport->WorkSize);
        ImGui::DockBuilderSetNodePos(id, viewport->WorkPos);

        // Add additional docks and assign them to windows based on this example
        // ImGuiID dock1 = ImGui::DockBuilderSplitNode(id, ImGuiDir_Up, 0.5f, nullptr, &id);
        // ImGuiID dock2 = ImGui::DockBuilderSplitNode(id, ImGuiDir_Down, 0.5f, nullptr, &id);
        // ImGui::DockBuilderDockWindow("Preview", dock1);
        // ImGui::DockBuilderDockWindow("Log", dock2);

        firstTime = false;
    }
}

void ImGui::SaveState()
{
    ImGui::SaveIniSettingsToDisk(GetImGuiSettingsFilepath().CStr());
}

// imspinner.h
#define PI_2 M_PI2

inline float CalcDampedGravity(float limtime)
{
    float time = 0.0f, initialHeight = 10.f, height = initialHeight, prtime = 0.0f;

    while (height >= 0.0) {
        if (prtime >= limtime) { return height / 10.f; }
        time += 0.01f; prtime += 0.01f;
        height = initialHeight - 0.5f * 9.81f * time * time;
        if (height < 0.0) { initialHeight = 0.0; time = 0.0; }
    }
    return 0.f;
}

inline ImColor MakeColorAlpha(ImColor c, float alpha) { c.Value.w *= alpha * ImGui::GetStyle().Alpha; return c; }

template <typename _PF>
inline void SpinnerCircle(_PF point_func, ImU32 dbc, float dth, ImVec2 centre, int num_segments) 
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    window->DrawList->PathClear();
    for (int i = 0; i < num_segments; i++) {
        ImVec2 p = point_func(i);
        window->DrawList->PathLineTo(ImVec2(centre.x + p.x, centre.y + p.y));
    }
    window->DrawList->PathStroke(dbc, 0, dth);
}

inline bool SpinnerBegin(const char *label, float radius, ImVec2 &pos, ImVec2 &size, ImVec2 &centre, int &num_segments) 
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext &g = *GImGui;
    const ImGuiStyle &style = g.Style;
    const ImGuiID id = window->GetID(label);

    pos = window->DC.CursorPos;
    // The size of the spinner is set to twice the radius, plus some padding based on the style
    size = ImVec2((radius) * 2, (radius + style.FramePadding.y) * 2);

    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(bb, style.FramePadding.y);

    num_segments = window->DrawList->_CalcCircleAutoSegmentCount(radius);

    centre = bb.GetCenter();
    // If the item cannot be added to the window, return false
    if (!ImGui::ItemAdd(bb, id))
        return false;

    return true;
}

void ImGui::SpinnerAng(const char *label, float radius, float thickness, const ImColor &color, const ImColor &bg, float speed, float angle, int mode)
{
    // SPINNER_HEADER
    // #define SPINNER_HEADER(pos, size, centre, num_segments)
    ImVec2 pos, size, centre; int num_segments;
    if (!SpinnerBegin(label, radius, pos, size, centre, num_segments)) 
        return;
    // END: SPINNER_HEADER

    float start = (float)ImGui::GetTime() * speed;
    radius = (mode == 2) ? (0.8f + ImCos(start) * 0.2f) * radius : radius;

    SpinnerCircle([&] (int i)->ImVec2 {
        const float a = start + (i * (PI_2 / (num_segments - 1)));
        return ImVec2(ImCos(a) * radius, ImSin(a) * radius);
    }, MakeColorAlpha(bg, 1.f), thickness, centre, num_segments);

    const float b = (mode == 1) ? CalcDampedGravity(ImSin(start * 1.1f)) * angle : 0.f;
    SpinnerCircle([&] (int i)->ImVec2 {
        const float a = start - b + (i * angle / num_segments);
        return ImVec2(ImCos(a) * radius, ImSin(a) * radius);
    }, MakeColorAlpha(color, 1.f), thickness, centre, num_segments);
}

bool ImGui::BeginMainToolbar(float height)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;

    if (height == 0)
        height = ImGui::GetFrameHeight();

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
    if (!ImGui::BeginViewportSideBar("##MainToolbar", viewport, ImGuiDir_Up, height, windowFlags))
        return false;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;
    if (!(window->Flags & ImGuiWindowFlags_MenuBar))
        return false;

    BeginGroup(); // Backup position on layer 0 // FIXME: Misleading to use a group for that backup/restore
    PushID("##ToolbarChild");

    ImRect bar_rect = window->MenuBarRect();
    bar_rect.Max.y = bar_rect.Min.y + height;
    
    ImRect clip_rect(IM_ROUND(bar_rect.Min.x + window->WindowBorderSize), 
                     IM_ROUND(bar_rect.Min.y), 
                     IM_ROUND(ImMax(bar_rect.Min.x, bar_rect.Max.x - ImMax(window->WindowRounding, window->WindowBorderSize))), 
                     IM_ROUND(bar_rect.Max.y));
    clip_rect.ClipWith(window->OuterRectClipped);
    PushClipRect(clip_rect.Min, clip_rect.Max, false);

    window->DC.CursorPos = window->DC.CursorMaxPos = ImVec2(bar_rect.Min.x + window->DC.MenuBarOffset.x, bar_rect.Min.y + window->DC.MenuBarOffset.y);
    window->DC.LayoutType = ImGuiLayoutType_Horizontal;
    window->DC.IsSameLine = false;
    window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;
    window->DC.MenuBarAppending = true;
    // AlignTextToFramePadding();

    return true;
}

void ImGui::internal::DisableSkipItems()
{
    ImGui::GetCurrentWindow()->SkipItems = false;
}

void ImGui::EndMainToolbar()
{
    ImGuiWindow* window = GetCurrentWindow();
    PopClipRect();
    PopID();
    window->DC.MenuBarOffset.x = window->DC.CursorPos.x - window->Pos.x; // Save horizontal position so next append can reuse it. This is kinda equivalent to a per-layer CursorPos.
    EndGroup(); 
    //GImGui->GroupStack.back().EmitItem = false;
    window->DC.LayoutType = ImGuiLayoutType_Vertical;
    window->DC.IsSameLine = false;
    window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
    window->DC.MenuBarAppending = false;

    ImGui::End();
    ImGui::PopStyleColor();
}

void ImGui::SeparatorVertical(float thickness)
{
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical, thickness);
}

bool ImGui::ToggleButton(const char* label, bool* toggled, const ImVec2& size_arg)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 labelSize = CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(size_arg, labelSize.x + style.FramePadding.x * 2.0f, labelSize.y + style.FramePadding.y * 2.0f);
    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);

    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, 0);

    if (pressed)
        *toggled = !*toggled;

    {
        const ImU32 col = GetColorU32(*toggled ? ImGuiCol_ButtonActive : ImGuiCol_Button);
        RenderNavHighlight(bb, id);
        RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
        RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &labelSize, style.ButtonTextAlign, &bb);
    }

    return pressed;
}