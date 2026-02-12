#include "Core.h"
#include "Main.h"

#include "ImGui/ImGuiMain.h"

Array<Path> gDropFiles;
ImVec2 gDropPos;

static void OnDropFiles(uint32 count, const Path* paths, Int2 mousePos, void* userData)
{
	ImVec2 p(float(mousePos.x), float(mousePos.y));
	gDropPos = p;

	gDropFiles.Clear();
	gDropFiles.PushBatch(paths, count);
}

static bool AcceptDropFilesIfInCurrentWindow(ImVec2 dropPos, const Path* dropFiles, uint32 numDropFiles)
{
	if (numDropFiles == 0)
		return false;

	ASSERT(dropFiles);

	ImVec2 wndPos = ImGui::GetWindowPos();
	ImVec2 wndSize = ImGui::GetWindowSize();
	ImVec2 minPt = wndPos;
	ImVec2 maxPt = ImVec2(wndPos.x + wndSize.x, wndPos.y + wndSize.y);

	if (dropPos.x >= minPt.x && dropPos.x <= maxPt.x && dropPos.y >= minPt.y && dropPos.y <= maxPt.y) {
		for (uint32 i = 0; i < numDropFiles; i++)
			LOG_VERBOSE(dropFiles[i].CStr());

		return true;
	}
	
	return false;
}

bool Initialize()
{
    // TODO: implement
	RegisterDropFiles(OnDropFiles, nullptr);
    return true;
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

	if (ImGui::Begin("Drop Here")) {
		if (AcceptDropFilesIfInCurrentWindow(gDropPos, gDropFiles.Ptr(), gDropFiles.Count())) {
			gDropFiles.Clear();
		}
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

