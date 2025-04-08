#include "Core.h"
#include "ImGui/imgui.h"

#if PLATFORM_LINUX
#include "Main.h"

#include <GLFW/glfw3.h> 
#include "ImGui/ImGuiMain.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"

struct MainWindowContext
{
    GLFWwindow* window;
    Float2 dpiScale;
    bool idleWait = true;
};

MainWindowContext gMainWindowCtx;

static void _GlfwSizeCallback(GLFWwindow* window, int width, int height)
{
    AppSettings& settings = GetSettings();
    settings.windowWidth = uint16(float(width)/float(gMainWindowCtx.dpiScale.x));
    settings.windowHeight = uint16(float(height)/float(gMainWindowCtx.dpiScale.y));
}

static void _GlfwPosCallback(GLFWwindow* window, int xpos, int ypos)
{
    AppSettings& settings = GetSettings();
    settings.windowX = xpos;
    settings.windowY = ypos;
}

static void _GlfwContentScaleCallback(GLFWwindow* window, float xscale, float yscale)
{
    gMainWindowCtx.dpiScale = Float2(xscale, yscale);
}

static void _GlfwErrorCallback(int error, const char* description)
{
    LOG_ERROR("GLFW error %d: %s", error, description);
}


void* CreateRGBATexture(uint32 width, uint32 height, const void* data)
{
    GLuint textureId;
    GLint lastTextureId;

    glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTextureId);
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifdef GL_UNPACK_ROW_LENGTH // Not on WebGL/ES
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, lastTextureId);

    return IntToPtr(textureId);
}

void  DestroyTexture(void* handle)
{
    GLuint textureId = PtrToInt<GLuint>(handle);
    if (textureId)
        glDeleteTextures(1, &textureId);
}

void* GetGraphicsDevice()
{
    return glfwGetCurrentContext();
}

int main(int argc, const char * argv[])
{
    [[maybe_unused]] bool r = InitializeCommon();
    ASSERT_ALWAYS(r, "InitializeCommon() failed");
    
    glfwSetErrorCallback(_GlfwErrorCallback);
    if (!glfwInit())
        return -1;

    // GL 3.0 + GLSL 130
    const char* GLSL_VERSION = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    // Create window with graphics context
    const AppSettings& settings = GetSettings();
    GLFWwindow* window = glfwCreateWindow(settings.windowWidth, settings.windowHeight, CONFIG_APP_NAME, nullptr, nullptr);
    if (window  == nullptr)
        return 1;
    gMainWindowCtx.window = window;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    glfwSetWindowPos(window, settings.windowX, settings.windowY);
    glfwSetWindowSizeCallback(window, _GlfwSizeCallback);
    glfwSetWindowPosCallback(window, _GlfwPosCallback);

    // Setup Dear ImGui context
    if (!ImGui::MyInitialize()) {
        glfwDestroyWindow(window);
        glfwTerminate();
        LOG_ERROR("ImGui initialization failed");
        return -1;
    }

    float xscale, yscale;
    glfwGetWindowContentScale(window, &xscale, &yscale);
    glfwSetWindowContentScaleCallback(window, _GlfwContentScaleCallback);

    ImGui::LoadFonts(Max(xscale, yscale));
    gMainWindowCtx.dpiScale = Float2(xscale, yscale);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(GLSL_VERSION);

    if (!Initialize()) {
        glfwDestroyWindow(window);
        glfwTerminate();
        LOG_ERROR("%s initialization failed", CONFIG_APP_NAME);
        return -1;
    }

    ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImGuiIO& io = ImGui::GetIO();
    
    while (!glfwWindowShouldClose(window)) {
            // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        if (gMainWindowCtx.idleWait)
            glfwWaitEvents();
        else
            glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::BeginFrame();

        UpdateCommon();
        Update();

        int dispWidth, dispHeight;
        glfwGetFramebufferSize(window, &dispWidth, &dispHeight);
        glViewport(0, 0, dispWidth, dispHeight);
        glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backupCurrentCtx = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backupCurrentCtx);
        }

        glfwSwapBuffers(window);
        MemTempAllocator::Reset();
    }

    ImGui::SaveState();
    ImGui::MyRelease();
    Release();
    ReleaseCommon();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
   
    return 0;
}

bool SetClipboardString(const char* text)
{
    ASSERT(gMainWindowCtx.window);
    glfwSetClipboardString(gMainWindowCtx.window, text);
    return true;    
}

bool GetClipboardString(char* textOut, uint32 textSize)
{
    ASSERT(gMainWindowCtx.window);
    Str::Copy(textOut, textSize, glfwGetClipboardString(gMainWindowCtx.window));
    return true;
}

void ToggleIdleWait(bool wait)
{
    gMainWindowCtx.idleWait = wait;
}

RectInt GetWindowDesktopRect()
{
    ASSERT(gMainWindowCtx.window);

    int xpos, ypos, width, height;
    GLFWmonitor* mon = glfwGetWindowMonitor(gMainWindowCtx.window);
    glfwGetMonitorWorkarea(mon, &xpos, &ypos, &width, &height);
    return RectInt(xpos, ypos, xpos + width, ypos + height);
}

void SetWindowTitle(const char* title)
{
    ASSERT(gMainWindowCtx.window);
    glfwSetWindowTitle(gMainWindowCtx.window, title);
}



#endif // PLATFORM_LINUX