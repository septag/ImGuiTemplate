#include "Core.h"

#if PLATFORM_WINDOWS

#include "Main.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/IconsFontAwesome4.h"
#include "ImGui/ImGuiMain.h"

#include "Resources/resource.h"

#include <stdio.h>

#define COM_NO_WINDOWS_H
#include <d3d11.h>
#undef COM_NO_WINDOWS_H

#if USE_LIVEPP
#include <LPP_API_x64_CPP.h>
#include <LPP_API_Options.h>
#endif

struct GraphicsContext
{
    ID3D11Device* device;
    ID3D11DeviceContext* deviceContext;
    IDXGISwapChain* swapchain;
    ID3D11RenderTargetView* mainRenderTargetView;
    ID3D11ShaderResourceView* fontTextureView;
};

struct MainWindowContext
{
    HWND hwnd;
    uint32 resizeWidth;
    uint32 resizeHeight;
    bool isFocused;
    bool disableIdleWait;
    bool quit;
};

// Forward declarations of helper functions
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static GraphicsContext gGfx;
static MainWindowContext gWindow;

static bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    UINT createDeviceFlags = 0;
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &gGfx.swapchain, &gGfx.device, &featureLevel, &gGfx.deviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &gGfx.swapchain, &gGfx.device, &featureLevel, &gGfx.deviceContext);
    if (res != S_OK)
        return false;

    // Create Render target
    {
        ID3D11Texture2D* pBackBuffer;
        gGfx.swapchain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        gGfx.device->CreateRenderTargetView(pBackBuffer, nullptr, &gGfx.mainRenderTargetView);
        pBackBuffer->Release();
    }

    return true;
}

static void CleanupDeviceD3D()
{
    if (gGfx.mainRenderTargetView) { gGfx.mainRenderTargetView->Release(); gGfx.mainRenderTargetView = nullptr; }
    if (gGfx.fontTextureView) { gGfx.fontTextureView->Release(); gGfx.fontTextureView = nullptr; }
    if (gGfx.swapchain) { gGfx.swapchain->Release(); gGfx.swapchain = nullptr; }
    if (gGfx.deviceContext) { gGfx.deviceContext->Release(); gGfx.deviceContext = nullptr; }
    if (gGfx.device) { gGfx.device->Release(); gGfx.device = nullptr; }
}

static void ResizeBuffers(uint32 width, uint32 height)
{
    ASSERT(gGfx.swapchain);

    if (gGfx.mainRenderTargetView) 
        gGfx.mainRenderTargetView->Release();

    gGfx.swapchain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

    ID3D11Texture2D* pBackBuffer;
    gGfx.swapchain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    gGfx.device->CreateRenderTargetView(pBackBuffer, nullptr, &gGfx.mainRenderTargetView);
    pBackBuffer->Release();
}

static void BeginDraw(ImVec4 clearColor)
{
    const float clearColorWithAlpha[4] = { 
        clearColor.x * clearColor.w, 
        clearColor.y * clearColor.w, 
        clearColor.z * clearColor.w, 
        clearColor.w };

    gGfx.deviceContext->OMSetRenderTargets(1, &gGfx.mainRenderTargetView, nullptr);
    gGfx.deviceContext->ClearRenderTargetView(gGfx.mainRenderTargetView, clearColorWithAlpha);
}

// Main code
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    #if USE_LIVEPP
    lpp::LppSynchronizedAgent lppAgent = lpp::LppCreateSynchronizedAgentANSI(nullptr, STRINGIZE(LIVEPP_PATH));
    lppAgent.EnableModuleANSI(lpp::LppGetCurrentModulePathANSI(), lpp::LPP_MODULES_OPTION_NONE, nullptr, nullptr);
    if (!lpp::LppIsValidSynchronizedAgent(&lppAgent)) {
        ASSERT_MSG(0, "LivePP initialization failed. Make sure cwd is the root directory of the project");
        return 1;
    }
    #endif

    [[maybe_unused]] bool r = InitializeCommon();
    ASSERT_ALWAYS(r, "InitializeCommon() failed");

    wchar_t appNameW[512];
    Str::Utf8ToWide(CONFIG_APP_NAME, appNameW, sizeof(appNameW));
    // Create application window
    ImGui_ImplWin32_EnableDpiAwareness();

    HICON appIcon = LoadIcon(GetModuleHandleA(nullptr), MAKEINTRESOURCE(IDI_APP_ICON));
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), appIcon, nullptr, nullptr, nullptr, appNameW, nullptr };
    RegisterClassExW(&wc);

    const AppSettings& settings = GetSettings();
    RectInt displayRect = GetWindowDesktopRect();
    RectInt myRect(settings.windowX, settings.windowY, settings.windowX + settings.windowWidth, settings.windowY + settings.windowHeight);
    if (!RectInt::Test(myRect, displayRect))
        SetWindowPos(0, 0);

    HWND hwnd = CreateWindowW(wc.lpszClassName, appNameW, WS_OVERLAPPEDWINDOW, 
                              settings.windowX, settings.windowY, 
                              Clamp<uint16>(settings.windowWidth, 500, displayRect.Width()), Clamp<uint16>(settings.windowHeight, 500, displayRect.Height()), 
                              nullptr, nullptr, wc.hInstance, nullptr);

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);
    gWindow.hwnd = hwnd;

    // Setup Platform/Renderer backends
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        return -1;
    }

    if (!ImGui::MyInitialize()) {
        CleanupDeviceD3D();
        LOG_ERROR("ImGui initialization failed");
        return -1;
    }
    ImGui::LoadFonts(ImGui_ImplWin32_GetDpiScaleForHwnd(hwnd));

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(gGfx.device, gGfx.deviceContext);

    Initialize();

    ImGuiIO& io = ImGui::GetIO();

    // Our state
    bool showAnotherWindow = false;
    ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done && !gWindow.quit)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;

        if (!gWindow.disableIdleWait) {
            constexpr uint32 MSG_WAIT_TIME = USE_LIVEPP ? 64 : INFINITE;
            MsgWaitForMultipleObjects(0, nullptr, FALSE, MSG_WAIT_TIME, QS_ALLINPUT);
        }

        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }

        if (msg.message == WM_QUIT)
            done = true;
        if (done)
            break;

        if (gWindow.quit)
            continue;

        #if USE_LIVEPP
        if (lppAgent.WantsReload(lpp::LPP_RELOAD_OPTION_SYNCHRONIZE_WITH_RELOAD))
            lppAgent.Reload(lpp::LPP_RELOAD_BEHAVIOUR_WAIT_UNTIL_CHANGES_ARE_APPLIED);
        if (lppAgent.WantsRestart())
            lppAgent.Restart(lpp::LPP_RESTART_BEHAVIOUR_INSTANT_TERMINATION, 0u, nullptr);
        #endif // USE_LIVEPP

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (gWindow.resizeWidth != 0 && gWindow.resizeHeight != 0)
        {
            ResizeBuffers(gWindow.resizeWidth, gWindow.resizeHeight);

            RECT windowRect;
            GetWindowRect(hwnd, &windowRect);
            SetWindowRect(uint16(windowRect.left), uint16(windowRect.top), 
                          uint16(windowRect.right - windowRect.left), 
                          uint16(windowRect.bottom - windowRect.top));
            gWindow.resizeWidth = gWindow.resizeHeight = 0;
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGui::BeginFrame();       
        UpdateCommon();
        Update();

        // Rendering
        BeginDraw(clearColor);
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        gGfx.swapchain->Present(gWindow.isFocused ? 0 : 2, 0); // Present with vsync
        MemTempAllocator::Reset();
    }

    ImGui::MyRelease();
    Release();
    ReleaseCommon();

    CleanupDeviceD3D();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();

    ImGui::DestroyContext();

    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);

#if USE_LIVEPP
    lppAgent.DisableModuleANSI(lpp::LppGetCurrentModulePathANSI(), lpp::LPP_MODULES_OPTION_NONE, nullptr, nullptr);
    lpp::LppDestroySynchronizedAgent(&lppAgent);
#endif 

    return 0;
}

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    #ifndef WM_DPICHANGED
    #define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
    #endif

    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED) {
            gWindow.resizeWidth = (UINT)LOWORD(lParam); // Queue resize
            gWindow.resizeHeight = (UINT)HIWORD(lParam);
        }
        else {
            gWindow.isFocused = false;
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;

    case WM_ACTIVATEAPP:
        gWindow.isFocused = wParam ? true : false;
        break;

    case WM_MOVE: {
        RECT windowRect;
        GetWindowRect(hWnd, &windowRect);
        SetWindowPos(uint16(windowRect.left), uint16(windowRect.top));
        break;
    }

    case WM_DPICHANGED:
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
        {
            //const int dpi = HIWORD(wParam);
            //printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
            const RECT* suggested_rect = (RECT*)lParam;
            ::SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        break;

    case WM_CLOSE:
        if (HasUnsavedChanges()) {
            QuitRequested([]() { ImGui::SaveState(); gWindow.quit = true;});
            return 0;
        }
        else {
            ImGui::SaveState();
            gWindow.quit = true;
        }
        break;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

void* CreateRGBATexture(uint32 width, uint32 height, const void* data)
{
    ID3D11Texture2D* texture;
    D3D11_TEXTURE2D_DESC textureDesc {
        .Width = width,
        .Height = height,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .SampleDesc = {1, 0},
        .Usage = D3D11_USAGE_DEFAULT,
        .BindFlags = D3D11_BIND_SHADER_RESOURCE

    };
    [[maybe_unused]] HRESULT hr = gGfx.device->CreateTexture2D(&textureDesc, nullptr, &texture);
    ASSERT(SUCCEEDED(hr));

    return texture;
}

void  DestroyTexture(void* handle)
{
    if (handle)
        ((ID3D11Texture2D*)handle)->Release();
}

bool SetClipboardString(const char* text)
{
    ASSERT(text);

    uint32 textLen = Str::Len(text);
    wchar_t* wcharBuff = 0;
    const size_t wcharBuffSize = (textLen + 1) * sizeof(wchar_t);
    HANDLE object = GlobalAlloc(GMEM_MOVEABLE, wcharBuffSize);
    if (!object)
        goto error;

    wcharBuff = (wchar_t*)GlobalLock(object);
    if (!wcharBuff)
        goto error;

    if (!Str::Utf8ToWide(text, wcharBuff, wcharBuffSize))
        goto error;

    GlobalUnlock(wcharBuff);
    wcharBuff = 0;
    if (!OpenClipboard(gWindow.hwnd))
        goto error;

    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, object);
    CloseClipboard();

    return true;
    
    error:
        if (wcharBuff)
            GlobalUnlock(object);
    if (object) 
        GlobalFree(object);
    return false;
}

bool GetClipboardString(char* textOut, uint32 textSize)
{
    ASSERT(gWindow.hwnd);
    
    if (!OpenClipboard(gWindow.hwnd))
        return false;
    
    HANDLE object = GetClipboardData(CF_UNICODETEXT);
    if (!object) {
        CloseClipboard();
        return false;
    }
    
    wchar_t* wcharBuff = (wchar_t*)GlobalLock(object);
    if (!wcharBuff) {
        CloseClipboard();
        return false;
    }
    
    Str::WideToUtf8(wcharBuff, textOut, textSize);
    GlobalUnlock(object);
    CloseClipboard();
    
    return true;
}

void ToggleIdleWait(bool wait)
{
    gWindow.disableIdleWait = !wait;
}

RectInt GetWindowDesktopRect()
{
    HMONITOR hmon = MonitorFromWindow(gWindow.hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(MONITORINFO) };
    if (GetMonitorInfoA(hmon, &mi))
        return RectInt(mi.rcWork.left, mi.rcWork.top, mi.rcWork.right, mi.rcWork.bottom);
    else
        return RECTINT_EMPTY;
}

void* GetGraphicsDevice()
{
    return gGfx.device;
}

void SetWindowTitle(const char* title)
{
    if (title && title[0])
        SetWindowTextA(gWindow.hwnd, String<512>::Format("%s [%s]", CONFIG_APP_NAME, title).CStr());
    else
        SetWindowTextA(gWindow.hwnd, CONFIG_APP_NAME);
}

#endif // PLATFORM_WINDOWS