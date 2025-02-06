#pragma once

#define CONFIG_IMGUI_SETTINGS_FILENAME "imgui.ini"

struct AppSettings
{
    uint16 windowX = 0;
    uint16 windowY = 0;
    uint16 windowWidth = 1280;
    uint16 windowHeight = 800;
};

bool Initialize();
void Release();
void Update();

AppSettings& GetSettings();
void SetWindowPos(uint16 x, uint16 y);
void SetWindowRect(uint16 x, uint16 y, uint16 width, uint16 height);
void SetWindowTitle(const char* title);

void ToggleIdleWait(bool wait);

void* CreateRGBATexture(uint32 width, uint32 height, const void* data);
void  DestroyTexture(void* handle);
void* GetGraphicsDevice();

bool HasUnsavedChanges();
void QuitRequested(void(*closeCallback)());

using ShortcutCallback = void(*)(void* userData);
bool RegisterShortcut(const char* shortcut, ShortcutCallback shortcutFn, void* userData);
void UnregisterShortcut(const char* shortcut);

bool SetClipboardString(const char* text);
bool GetClipboardString(char* textOut, uint32 textSize);

bool InitializeCommon();
void ReleaseCommon();
void UpdateCommon();

RectInt GetWindowDesktopRect();

