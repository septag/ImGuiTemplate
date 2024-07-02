#include "Core.h"
#include "CommonLogView.h"

#include "ImGui/ImGuiMain.h"

static TextContent gLogContents;
static GuiTextView gLogView;
static bool gLogIsOpen = true;

static void CommonLogCallback(const LogEntry& entry, void* userData)
{
    TextContent* contents = (TextContent*)userData;

    // TODO: improve this part. it sucks
    if (contents->mBlob.Size())
        contents->mBlob.SetSize(contents->mBlob.Size() - 1);    // Remove the last null-terminator

    switch(entry.type) {
    case LogLevel::Error:
        contents->WriteData("Error: ", strLen("Error: "));
        break;
    case LogLevel::Warning:
        contents->WriteData("Warning: ", strLen("Warning: "));
        break;
    case LogLevel::Debug:
        contents->WriteData("[DBG]: ", strLen("[DBG]: "));
        break;
    default:
        break;
    }

    contents->WriteData(entry.text, entry.textLen);
    contents->WriteData<char>('\n');

    contents->ParseLines();
    contents->WriteData<char>('\n');
}

bool InitializeLogView()
{
    if (!gLogContents.Initialize(64*kKB, 64*kKB)) {
        gLogIsOpen = false;
        return false;
    }

    logRegisterCallback(CommonLogCallback, &gLogContents);

    return true;
}

void ReleaseLogView()
{
    logUnregisterCallback(CommonLogCallback);
    gLogContents.Release();
}

void RenderLogView()
{
    if (gLogIsOpen) 
        gLogView.Render(&gLogContents, "Log", &gLogIsOpen);
}

bool IsLogViewShown()
{
    return gLogIsOpen;
}

void ToggleLogView(bool toggle)
{
    gLogIsOpen = toggle;
}

bool TextContent::Initialize(size_t reserveSize, size_t pageSize)
{
    mAlloc.Initialize(reserveSize, pageSize);
    mBlob.SetAllocator(&mAlloc);
    mBlob.SetGrowPolicy(Blob::GrowPolicy::Linear, 32*kKB);
    return true;
}

void TextContent::Release()
{
    mBlob.Free();
    mLines.Free();
}
    
void TextContent::ParseLines()
{
    if (!mLastLinePtr)
    mLastLinePtr = (const char*)mBlob.Data();
    ASSERT(mLastLinePtr);

    const char* startPtr = (const char*)mBlob.Data();
    const char* str = mLastLinePtr;
    const char* line = str;
    while (uintptr_t(str - startPtr) < mBlob.Size()) {
        if (*str == '\n' || *str == '\0') {
            uint32 end = (uint32)uintptr_t(str - startPtr);
            if (*(str - 1) == '\r')
                --end;
            uint32 start = (uint32)uintptr_t(line - startPtr);

            SpinLockMutexScope lock(mLock);
            mLines.Push({start, end});

            if (*str == '\0')
                break;

            line = str + 1;
            mLastLinePtr = line;
        }
        ++str;
    }

    if (mRedirectContent)
        mRedirectContent->ParseLines();
}

void TextContent::WriteData(const void* src, size_t size)
{
    mBlob.Write(src, size);
    if (mRedirectContent) {
        SpinLockMutexScope redirectLock(mRedirectContent->mLock);
        mRedirectContent->mBlob.Write(src, size);
    }
}

void TextContent::Reset()
{
    mLines.Clear();
    mBlob.ResetRead();
    mBlob.SetSize(0);
    mLastLinePtr = nullptr;
    atomicExchange32Explicit(&mResetFlag, 1, AtomicMemoryOrder::Release);
}

GuiTextView::~GuiTextView()
{
    mLines.Free();
    memFree(mEditableText);
}

void GuiTextView::Reset()
{
    mLines.Clear();
    memFree(mEditableText);
    mEditableText = nullptr;
    mLastUpdateLineCount = 0;
    mLastUpdateContentWidth = 0;
    mEditableLine = 0;
    mEditableLineCount = 0;
    mEditableTextSize = 0;
}

void GuiTextView::Render(TextContent* content, const char* windowId, bool* pOpen)
{
    auto CreateEditable = [this](const char* str, int lineIndex, uint32 lineNo) {
        if (mEditableText) {
            memFree(mEditableText);
            mEditableText = nullptr;
        }

        for (int i = lineIndex; i >= 0; --i) {
            if (mLines[i].lineNo == lineNo)
                lineIndex = i;
            else 
                break;
        }

        uint32 totalTextSize = 0;
        mEditableLineCount = 0;
        for (int i = lineIndex; i < int(mLines.Count()); i++) {
            const GuiTextView::Line& line = mLines[i];
            if (line.lineNo == lineNo) {
                uint32 textSize = line.text.end - line.text.begin;
                mEditableText = memReallocTyped<char>(mEditableText, totalTextSize + textSize + 1);
                memcpy(mEditableText + totalTextSize, str + line.text.begin, textSize);
                mEditableText[totalTextSize + textSize] = '\n';
                totalTextSize += (textSize + 1);
                ++mEditableLineCount;
            }
            else {
                mEditableText = memReallocTyped<char>(mEditableText, totalTextSize + 1);
                if (totalTextSize) {
                    if (mEditableText[totalTextSize-1] == '\n')
                    mEditableText[totalTextSize-1] = 0;
                }
                mEditableText[totalTextSize] = '\0';
                break;
            }
        }

        mEditableLine = lineNo;        
        mEditableTextSize = totalTextSize;
    };

    ImGui::SetNextWindowSizeConstraints(ImVec2(320, 100), ImVec2(-1, -1));
    if (ImGui::Begin(windowId, pOpen, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
        bool updated = false;
        float contentWidth = ImGui::GetContentRegionAvail().x;        
        bool widgetResized = mathAbs(contentWidth - mLastUpdateContentWidth) >= 1.0f;
        bool contentChanged = mLastUpdateLineCount != content->mLines.Count();
        if ((widgetResized || contentChanged) && content->mLock.TryEnter()) 
        {
            uint32 startIndex = mLastUpdateLineCount;
            mLastUpdateLineCount = content->mLines.Count();
            mLastUpdateContentWidth = contentWidth;

            MemTempAllocator tempAlloc;
            Array<TextSegment> tempContentLines(&tempAlloc);
            content->mLines.CopyTo(&tempContentLines);
            content->mLock.Exit();
            
            atomicUint32 kOne = 1;
            if (atomicCompareExchange32Weak(&content->mResetFlag, &kOne, 0) || widgetResized) {
                mLines.Clear();
                startIndex = 0;
            }

            ImFont* font = ImGui::GetFonts().monoFont;
            float fontScale = ImGui::GetIO().FontGlobalScale;
            const char* strPtr = (const char*)content->mBlob.Data();
        
            uint32 lineNo = startIndex + 1;
            
            for (uint32 i = startIndex; i < tempContentLines.Count(); i++) {
                const TextSegment& line = tempContentLines[i];
                uint32 beginPos = line.begin;
                while (true) {
                    const char* wrapped = font->CalcWordWrapPositionA(fontScale, strPtr + beginPos, strPtr + line.end, contentWidth);
                    uint32 wrappedPos = (uint32)uintptr_t(wrapped - strPtr);
                    mLines.Push(GuiTextView::Line {
                        .lineNo = lineNo,
                        .text = {beginPos, wrappedPos}
                    });

                    beginPos = wrappedPos;
                    if (wrappedPos == line.end)
                        break;                
                }

                lineNo++;
            }

            mAutoScroll = true;
            updated = true;
        }

        ImGui::PushFont(ImGui::GetFonts().monoFont);

        // TEMP: We need to be able to select multiple lines with Shift pressed
        //       We also can have a max number of lines selected like 64
        static uint32 selectedLine = 0;
        static uint32 selectedLine2 = 0;
        const char* strPtr = (const char*)content->mBlob.Data();
        char id[32];
        float lineHeight = ImGui::GetTextLineHeightWithSpacing();

        ImGuiListClipper clipper;
        clipper.Begin((int)mLines.Count());
        while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                const GuiTextView::Line& line = mLines[i];
                strPrintFmt(id, sizeof(id), "##Line_%d", i);

                if (mEditableLine != line.lineNo) {
                    if (ImGui::Selectable(id, line.lineNo == selectedLine || line.lineNo == selectedLine2, ImGuiSelectableFlags_AllowDoubleClick)) {
                        if (ImGui::GetIO().KeyCtrl) {
                            if (selectedLine == 0)
                            selectedLine = line.lineNo;
                            else if (selectedLine2 == 0)
                            selectedLine2 = line.lineNo;
                        }
                        else {
                            selectedLine = 0;
                            selectedLine2 = 0;
                        }
    

                        if (ImGui::IsMouseDoubleClicked(0))
                        CreateEditable(strPtr, i, line.lineNo);
                        else
                        mEditableLine = 0;
                    }
                    ImGui::SameLine();
                    ImGui::TextUnformatted(strPtr + line.text.begin, strPtr + line.text.end);
                }
                else {
                    ASSERT(mEditableText);
                    ImGui::InputTextMultiline(id, mEditableText, mEditableTextSize,
                                              ImVec2(-1, mEditableLineCount*lineHeight + 5), 
                                              ImGuiInputTextFlags_ReadOnly|ImGuiInputTextFlags_AutoSelectAll|
                                              ImGuiInputTextFlags_NoHorizontalScroll);
                    i += mEditableLineCount - 1;
                }
            }
        }

        ImGui::PopFont();

        if (mAutoScroll)
            ImGui::SetScrollY(ImGui::GetScrollMaxY());
        if (!updated && mAutoScroll)
            mAutoScroll = false;
    }

    ImGui::End();
}
