#include "Core.h"
#include "Main.h"

#include "ImGui/ImGuiMain.h"

bool Initialize()
{
    // TODO: implement
     return false;
}

void Release()
{
    // TODO: implement
}

void Update()
{
    // TODO: implement

    if (ImGui::Begin("Hello")) {
        ImGui::Text("Fps: %.1f", 1.0f/ImGui::GetIO().DeltaTime);
    }
    ImGui::End();
}

void QuitRequested(void(*closeCallback)())
{
    // TODO: implement
    closeCallback();
}

bool HasUnsavedChanges()
{
    // TODO: implement
    return false;
}

