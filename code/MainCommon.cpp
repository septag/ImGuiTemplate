#include "Core.h"
#include "Main.h"

#include "ImGui/imgui.h"

struct ShortcutItem
{
    char name[16];
    ImGuiKey keys[2];
    int modKeys;    // modifier keys, ImGuiKey combination
    ShortcutCallback callback;
    void* user;
};

static Settings gSettings;
static Array<ShortcutItem> gShortcuts;

struct SettingsImpl : SettingsCustomCallbacks
{
    inline static const char* kCats[] = {
        "Layout"
    };

    enum Category 
    {
        Layout = 0,
        Count
    };

    uint32 GetCategoryCount() const override    { return uint32(Category::Count); }
    const char* GetCategory(uint32 id) const override { ASSERT(id < uint32(Category::Count)); return kCats[id]; }

    bool ParseSetting(uint32 categoryId, const char* key, const char* value) override
    {
        ASSERT(categoryId < uint32(Category::Count));
        switch (Category(categoryId)) {
        case Category::Layout:
            if (strIsEqualNoCase(key, "WindowWidth")) {
                gSettings.windowWidth = uint16(strToInt(value));
                return true;
            }
            else if (strIsEqualNoCase(key, "WindowHeight")) {
                gSettings.windowHeight = uint16(strToInt(value));
                return true;
            }
            else if (strIsEqualNoCase(key, "WindowX")) {
                gSettings.windowX = uint16(strToInt(value));
                return true;
            }
            else if (strIsEqualNoCase(key, "WindowY")) {
                gSettings.windowY = uint16(strToInt(value));
                return true;
            }
        default:
            break;
        }
        return false;
    }

    void SaveCategory(uint32 categoryId, Array<SettingsKeyValue>& items) override
    {
        char num[32];
        auto ToStr = [&num](int n)->const char* { strPrintFmt(num, sizeof(num), "%d", n); return num; };

        ASSERT(categoryId < uint32(Category::Count));
        switch (Category(categoryId)) {
        case Category::Layout:
            items.Push(SettingsKeyValue { "WindowWidth", ToStr(gSettings.windowWidth) });
            items.Push(SettingsKeyValue { "WindowHeight", ToStr(gSettings.windowHeight) });
            items.Push(SettingsKeyValue { "WindowX", ToStr(gSettings.windowX) });
            items.Push(SettingsKeyValue { "WindowY", ToStr(gSettings.windowY) });
            break;
        default:
            break;
        }
    }
};

static Path GetSettingsFilePath()
{
    Path myDir;
    pathGetMyPath(myDir.Ptr(), myDir.Capacity());
    myDir = myDir.GetDirectory();
    ASSERT(myDir.IsDir());
    return Path::Join(myDir, CONFIG_APP_NAME ".ini");
}

bool InitializeCommon()
{
    static SettingsImpl settingsImpl;

    settingsAddCustomCallbacks(&settingsImpl);
    settingsInitializeFromINI(GetSettingsFilePath().CStr());

    logSetSettings(LogLevel::Debug, false, false);
    jobsInitialize({});
    
    return true;
}

void ReleaseCommon()
{
    settingsSaveToINI(GetSettingsFilePath().CStr());

    jobsRelease();
    settingsRelease();
}

const Settings& GetSettings()
{
    return gSettings;
}

void SetWindowPos(uint16 x, uint16 y)
{
    gSettings.windowX = x;
    gSettings.windowY = y;
}

void SetWindowRect(uint16 x, uint16 y, uint16 width, uint16 height)
{
    gSettings.windowX = x;
    gSettings.windowY = y;
    gSettings.windowWidth = width;
    gSettings.windowHeight = height;
}

// shortcut string example:
// "mod1+mod2+key1+key2+key3"
// "SHIFT+CTRL+K"
static ShortcutItem ParseShortcutKeys(const char* shortcut)
{
    shortcut = strSkipWhitespace(shortcut);

    ShortcutItem item {};
    uint32 numKeys = 0;
    const char* plus;
    char keystr[32];

    auto ParseSingleKey = [&item, &numKeys](const char* keystr) {
        uint32 len = strLen(keystr);

        bool isFn = 
            (len == 2 || len == 3) && 
            strToUpper(keystr[0]) == 'F' && 
            ((len == 2 && strIsNumber(keystr[1])) || (len == 3 && strIsNumber(keystr[1]) && strIsNumber(keystr[2])));
        if (isFn && numKeys < 2) {
            char numstr[3] = {keystr[1], keystr[2], 0};
            int fnum = strToInt(numstr) - 1;
            if (fnum >= 0 && fnum < 12)
                item.keys[numKeys++] = ImGuiKey(ImGuiKey_F1 + fnum);
        }
        else if (len > 1) {
            char modstr[32];
            strToUpper(modstr, sizeof(modstr), keystr);
            if (strIsEqual(modstr, "ALT"))
                item.modKeys |= ImGuiKey_ModAlt;
            else if (strIsEqual(modstr, "CTRL"))
                item.modKeys |= ImGuiKey_ModCtrl;
            else if (strIsEqual(modstr, "SHIFT"))
                item.modKeys |= ImGuiKey_ModShift;
        } 
        else if (len == 1 && numKeys < 2) {
            if (keystr[0] > 32) {
                switch (strToUpper(keystr[0])) {
                case '0': item.keys[numKeys++] = ImGuiKey_0; break;
                case '1': item.keys[numKeys++] = ImGuiKey_1; break;
                case '2': item.keys[numKeys++] = ImGuiKey_2; break;
                case '3': item.keys[numKeys++] = ImGuiKey_3; break;
                case '4': item.keys[numKeys++] = ImGuiKey_4; break;
                case '5': item.keys[numKeys++] = ImGuiKey_5; break;
                case '6': item.keys[numKeys++] = ImGuiKey_6; break;
                case '7': item.keys[numKeys++] = ImGuiKey_7; break;
                case '8': item.keys[numKeys++] = ImGuiKey_8; break;
                case '9': item.keys[numKeys++] = ImGuiKey_9; break;
                case 'A': item.keys[numKeys++] = ImGuiKey_A; break;
                case 'B': item.keys[numKeys++] = ImGuiKey_B; break;
                case 'C': item.keys[numKeys++] = ImGuiKey_C; break;
                case 'D': item.keys[numKeys++] = ImGuiKey_D; break;
                case 'E': item.keys[numKeys++] = ImGuiKey_E; break;
                case 'F': item.keys[numKeys++] = ImGuiKey_F; break;
                case 'G': item.keys[numKeys++] = ImGuiKey_G; break;
                case 'H': item.keys[numKeys++] = ImGuiKey_H; break;
                case 'I': item.keys[numKeys++] = ImGuiKey_I; break;
                case 'J': item.keys[numKeys++] = ImGuiKey_J; break;
                case 'K': item.keys[numKeys++] = ImGuiKey_K; break;
                case 'L': item.keys[numKeys++] = ImGuiKey_L; break;
                case 'M': item.keys[numKeys++] = ImGuiKey_M; break;
                case 'N': item.keys[numKeys++] = ImGuiKey_N; break;
                case 'O': item.keys[numKeys++] = ImGuiKey_O; break;
                case 'P': item.keys[numKeys++] = ImGuiKey_P; break;
                case 'Q': item.keys[numKeys++] = ImGuiKey_Q; break;
                case 'R': item.keys[numKeys++] = ImGuiKey_R; break;
                case 'S': item.keys[numKeys++] = ImGuiKey_S; break;
                case 'T': item.keys[numKeys++] = ImGuiKey_T; break;
                case 'U': item.keys[numKeys++] = ImGuiKey_U; break;
                case 'V': item.keys[numKeys++] = ImGuiKey_V; break;
                case 'W': item.keys[numKeys++] = ImGuiKey_W; break;
                case 'X': item.keys[numKeys++] = ImGuiKey_X; break;
                case 'Y': item.keys[numKeys++] = ImGuiKey_Y; break;
                case 'Z': item.keys[numKeys++] = ImGuiKey_Z; break;
                case '-': item.keys[numKeys++] = ImGuiKey_Minus; break;
                case '=': item.keys[numKeys++] = ImGuiKey_Equal; break;
                case '[': item.keys[numKeys++] = ImGuiKey_LeftBracket; break;
                case ']': item.keys[numKeys++] = ImGuiKey_RightBracket; break;
                case ';': item.keys[numKeys++] = ImGuiKey_Semicolon; break;
                case '\'': item.keys[numKeys++] = ImGuiKey_Apostrophe; break;
                case '`': item.keys[numKeys++] = ImGuiKey_GraveAccent; break;
                case ',': item.keys[numKeys++] = ImGuiKey_Comma; break;
                case '.': item.keys[numKeys++] = ImGuiKey_Period; break;
                case '/': item.keys[numKeys++] = ImGuiKey_Slash; break;
                case '\\': item.keys[numKeys++] = ImGuiKey_Backslash; break;
                default: break;
                }
            }
        }
    };


    while (shortcut[0]) {
        plus = strFindChar(shortcut, '+');
        if (!plus)
            break;

        strCopyCount(keystr, sizeof(keystr), shortcut, PtrToInt<uint32>((void*)(plus - shortcut)));
        ParseSingleKey(keystr);
        shortcut = strSkipWhitespace(plus + 1);
    }

    // read the last one
    if (shortcut[0]) {
        strCopy(keystr, sizeof(keystr), shortcut);
        ParseSingleKey(keystr);
    }

    return item;
}

bool RegisterShortcut(const char* shortcut, ShortcutCallback shortcutFn, void* userData)
{
    ASSERT(shortcut);
    ASSERT(shortcutFn);

    // strip whitespace and search for duplicates
    char name[32];
    if (strLen(shortcut) >= sizeof(name)) {
        ASSERT(0);
        return false;
    }

    strRemoveWhitespace(name, sizeof(name), shortcut);
    strToUpper(name, sizeof(name), name);
    for (const ShortcutItem& item : gShortcuts) {
        if (strIsEqual(name, item.name)) {
            ASSERT_MSG(0, "Shortcut already registered '%s'", shortcut);
            return false;
        }
    }

    ShortcutItem item = ParseShortcutKeys(name);
    if (item.keys[0]) {
        strCopy(item.name, sizeof(item.name), name);
        item.callback = shortcutFn;
        item.user = userData;
        gShortcuts.Push(item);
        return true;
    }
    else {
        return false;
    }
}

void UnregisterShortcut(const char* shortcut)
{
    char name[32];
    strRemoveWhitespace(name, sizeof(name), shortcut);
    strToUpper(name, sizeof(name), name);
    for (uint32 i = 0; i < gShortcuts.Count(); i++) {
        const ShortcutItem& item = gShortcuts[i];
        if (strIsEqual(name, item.name)) {
            gShortcuts.RemoveAndSwap(i);
            return;
        }
    }
}

