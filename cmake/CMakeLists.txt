cmake_minimum_required(VERSION 3.15)

set(APPNAME "" CACHE STRING "App name")
if (NOT APPNAME)
    message(FATAL_ERROR "APPNAME is not defined. -DAPPNAME=[Name]")
endif()

project(${APPNAME})

if (CMAKE_GENERATOR MATCHES "Visual Studio" OR CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    option(MSVC_MULTITHREADED_COMPILE "Multi-threaded compilation in MSVC." ON)
endif()

if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)
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

    if (CMAKE_C_COMPILER_ID MATCHES "GNU")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format-truncation")
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

add_executable(${APPNAME} ${SOURCE_FILES})

if (WIN32)
    set_target_properties(${APPNAME} PROPERTIES WIN32_EXECUTABLE TRUE)
    target_link_libraries(${APPNAME} "d3d11.lib" "freetype.lib")

    # warning LNK4099: PDB '' was not found with 'freetype.lib'
    # warning: static libs conflict
    target_link_options(${APPNAME} PRIVATE /ignore:4099 /NODEFAULTLIB:LIBCMT /DEBUG:FULL)

    target_compile_options(${APPNAME} PRIVATE /Zi /GR-)

    target_compile_definitions(${APPNAME} PRIVATE -D_ITERATOR_DEBUG_LEVEL=0 -D_SECURE_SCL=0 -D_HAS_EXCEPTIONS=0 -D_CRT_SECURE_NO_WARNINGS=0)
else()
    target_compile_definitions(${APPNAME} PRIVATE "$<$<CONFIG:Debug>:-D_DEBUG>")
    target_compile_definitions(${APPNAME} PRIVATE "$<$<CONFIG:Release>:-DNDEBUG>")

    target_compile_options(${APPNAME} PRIVATE -fno-rtti -fno-exceptions -Wno-deprecated-declarations -fPIC)
endif()

target_compile_definitions(${APPNAME} PRIVATE -D__STDC_LIMIT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_CONSTANT_MACROS)

target_precompile_headers(${APPNAME} PRIVATE ../code/Core.h)
target_include_directories(${APPNAME} PRIVATE ../code/ImGui/freetype)
target_link_directories(${APPNAME} PRIVATE ../code/ImGui/freetype)
target_compile_definitions(${APPNAME} PRIVATE -DCONFIG_APP_NAME="${APPNAME}")
