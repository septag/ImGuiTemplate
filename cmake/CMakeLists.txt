cmake_minimum_required(VERSION 3.15)

set(APPNAME "" CACHE STRING "App name")
if (NOT APPNAME)
    message(FATAL_ERROR "APPNAME is not defined. -DAPPNAME=[Name]")
endif()

project(${APPNAME})

if (CMAKE_GENERATOR MATCHES "Visual Studio" OR CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    option(MSVC_MULTITHREADED_COMPILE "Multi-threaded compilation in MSVC." ON)
endif()

option(USE_LIVEPP "Use LivePP for windows debug builds. Requires LIVEPP_PATH" OFF)
if (USE_LIVEPP)
    set(LIVEPP_PATH "" CACHE PATH "Path to the root directory of LivePP")
    if (NOT EXISTS ${LIVEPP_PATH})
        message(FATAL_ERROR "LivePP path does not exist: ${LIVEPP_PATH}")
    endif()
endif()

if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../bin)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

# Remove harmful flags and set basic global flags
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:DebugDLL>)

    string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REPLACE "/GR" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

    if (MSVC_MULTITHREADED_COMPILE AND NOT CMAKE_MAKE_PROGRAM MATCHES "ninja.exe")
        add_compile_options(/MP)    # multiprocessor compilation
    endif()
else()
    string(REPLACE "-fexceptions" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REPLACE "-frtti" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_definitions(_DEBUG)
    endif()

    if (CMAKE_C_COMPILER_ID MATCHES "GNU")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format-truncation")
    endif()

    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        include_directories("/usr/lib/llvm-14/include/")        # This is hard coded, don't know how to fix this yet
    endif()

endif()

# Source files
set(SOURCE_FILES 
    ../code/Resources/App.rc
    ../code/Resources/AppIcon.ico
    ../code/Resources/resource.h
    ../code/Core.cpp
    ../code/Core.h
    ../code/Main.cpp 
    ../code/Main.h
    ../code/MainWin.cpp 
    ../code/MainCommon.cpp 
    ../code/CommonLogView.h 
    ../code/CommonLogView.cpp 
    ../code/ImGui/ImGuiMain.cpp
    ../code/ImGui/ImGuiMain.h
    ../code/ImGui/imgui.cpp
    ../code/ImGui/imgui_draw.cpp
    ../code/ImGui/imgui_freetype.cpp
    ../code/ImGui/imgui_tables.cpp
    ../code/ImGui/imgui_widgets.cpp
    ../code/ImGui/imgui.h
    ../code/ImGui/imconfig.h 
    ../code/External/osdialog/osdialog.h
    ../code/External/osdialog/osdialog.c)
set_source_files_properties(../code/External/osdialog/osdialog.c PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

if (WIN32)
    list(APPEND SOURCE_FILES 
        ../code/ImGui/imgui_impl_dx11.cpp
        ../code/ImGui/imgui_impl_dx11.h 
        ../code/ImGui/imgui_impl_win32.cpp
        ../code/ImGui/imgui_impl_win32.h
        ../code/External/osdialog/osdialog_win.c)
    set_source_files_properties(../code/External/osdialog/osdialog_win.c PROPERTIES SKIP_PRECOMPILE_HEADERS ON)        
    set_source_files_properties(../code/External/osdialog/osdialog_win.c PROPERTIES APPEND_STRING PROPERTY COMPILE_FLAGS " /wd4244")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND SOURCE_FILES 
        ../code/MainLinux.cpp 
        ../code/ImGui/imgui_impl_opengl3.cpp
        ../code/ImGui/imgui_impl_opengl3.h 
        ../code/ImGui/imgui_impl_glfw.cpp
        ../code/ImGui/imgui_impl_glfw.h
        ../code/External/osdialog/osdialog_gtk3.c)

    set_source_files_properties(../code/External/osdialog/osdialog_gtk3.c PROPERTIES SKIP_PRECOMPILE_HEADERS ON)        
endif()

add_executable(${APPNAME} ${SOURCE_FILES})

if (WIN32)
    set_target_properties(${APPNAME} PROPERTIES WIN32_EXECUTABLE TRUE)
    target_link_libraries(${APPNAME} "d3d11.lib" "freetype.lib")

    # warning LNK4099: PDB '' was not found with 'freetype.lib'
    # warning: static libs conflict
    target_link_options(${APPNAME} PRIVATE /ignore:4099 /NODEFAULTLIB:LIBCMT /DEBUG:FULL)

    target_compile_options(${APPNAME} PRIVATE /Zi /GR-)

    target_compile_definitions(${APPNAME} PRIVATE -D_ITERATOR_DEBUG_LEVEL=0 -D_SECURE_SCL=0 -D_HAS_EXCEPTIONS=0 -D_CRT_SECURE_NO_WARNINGS=0)

    if (USE_LIVEPP)
        target_include_directories(${APPNAME} PRIVATE ${LIVEPP_PATH}/API/x64)
        target_compile_definitions(${APPNAME} PRIVATE LIVEPP_PATH=${LIVEPP_PATH})
        target_compile_definitions(${APPNAME} PRIVATE USE_LIVEPP=1)
    else()
        target_compile_definitions(${APPNAME} PRIVATE USE_LIVEPP=0)
    endif()

    # LivePP
    # CL: Enable Minimal Rebuild (/Gm-), Function level linking (/Gy), Optimize Global Data (/Gw)
    # Link: NoReferences (/OPT:NOREF), Disable COMDAT Folding (/OPT:NOICF), Full Debug info (Already added), CreateHotpackableImage (/FUNCTIONPADMIN)
    target_compile_options(${APPNAME} PUBLIC /Gm- /Gy /Gw)
    target_link_options(${APPNAME} PUBLIC /OPT:NOREF /OPT:NOICF /FUNCTIONPADMIN) 
else()
    target_compile_options(${APPNAME} PRIVATE -fno-rtti -fno-exceptions -Wno-deprecated-declarations)

    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(FreeType REQUIRED freetype2)
        pkg_check_modules(GLFW REQUIRED glfw3)
        pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

        target_include_directories(${APPNAME} PRIVATE ${FreeType_INCLUDE_DIRS})
        target_include_directories(${APPNAME} PRIVATE ${GLFW_INCLUDE_DIRS})
        target_include_directories(${APPNAME} PRIVATE ${GTK3_INCLUDE_DIRS})

        target_link_directories(${APPNAME} PRIVATE ${FreeType_LIBRARY_DIRS})
        target_link_directories(${APPNAME} PRIVATE ${GLFW_LIBRARY_DIRS})
        target_link_directories(${APPNAME} PRIVATE ${GTK3_LIBRARY_DIRS})

        target_link_libraries(${APPNAME} PRIVATE ${FreeType_LIBRARIES})
        target_link_libraries(${APPNAME} PRIVATE ${GLFW_LIBRARIES})
        target_link_libraries(${APPNAME} PRIVATE ${GTK3_LIBRARIES})

        target_link_libraries(${APPNAME} PRIVATE uuid m c++abi GL)
    endif()

endif()

target_compile_definitions(${APPNAME} PRIVATE -D__STDC_LIMIT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_CONSTANT_MACROS)

target_precompile_headers(${APPNAME} PRIVATE ../code/Core.h)
target_include_directories(${APPNAME} PRIVATE ../code/ImGui/freetype)
target_link_directories(${APPNAME} PRIVATE ../code/ImGui/freetype)
target_compile_definitions(${APPNAME} PRIVATE -DCONFIG_APP_NAME="${APPNAME}")


