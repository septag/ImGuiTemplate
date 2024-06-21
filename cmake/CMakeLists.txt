cmake_minimum_required(VERSION 3.10)

set(APPNAME "" CACHE STRING "App name")
if (NOT APPNAME)
    message(FATAL_ERROR "APPNAME is not defined. -DAPPNAME=[Name]")
endif()

project(${APPNAME})

if (MSVC)
    option(MSVC_STATIC_RUNTIME "Link with MSVC static runtime. Only sets for 'Release' build" OFF)
    option(MSVC_MULTITHREADED_COMPILE "Multi-threaded compilation in MSVC." ON)
endif()

if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}")
include(Utils)

add_definitions(-D__STDC_LIMIT_MACROS)
add_definitions(-D__STDC_FORMAT_MACROS)
add_definitions(-D__STDC_CONSTANT_MACROS)
if(WIN32)
    add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)
    add_definitions(-D_SECURE_SCL=0)
    add_definitions(-D_HAS_EXCEPTIONS=0)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS=0)
endif()

# Common C++ flags
set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    utils_remove_compile_options(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" "/EHsc /GR")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
    if (MSVC_MULTITHREADED_COMPILE AND NOT CMAKE_MAKE_PROGRAM MATCHES "ninja.exe")
        add_compile_options(/MP)    # multiprocessor compilation
    endif()

    if (MSVC_STATIC_RUNTIME) 
        utils_remove_compile_options(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}" "/MD")
        utils_remove_compile_options(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}" "/MD")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")

        utils_remove_compile_options(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}" "/MDd")
        utils_remove_compile_options(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}" "/MDd")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
    endif()

    if (${MSVC_VERSION} GREATER_EQUAL 1928)
        message(STATUS "MSVC compiler supports C17 standard")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /std:c17")
    endif()

    utils_remove_compile_options(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}" "/Ob0")
    utils_remove_compile_options(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}" "/Ob0")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Ob1")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Ob1")
else()
    if (NOT WIN32)
        add_compile_options(-fPIC)
    endif()
    add_compile_options(-Wno-deprecated-declarations)
    add_compile_options("$<$<CONFIG:Debug>:-D_DEBUG>")
    add_compile_options("$<$<CONFIG:Release>:-DNDEBUG>")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")
    utils_remove_compile_options(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" "-fexceptions -frtti")

    if (CMAKE_C_COMPILER_ID MATCHES "GNU")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format-truncation")
    endif()
endif()

# Source files
set(SOURCE_FILES 
    ../code/Core.cpp
    ../code/Core.h
    ../code/Main.cpp 
    ../code/Main.h
    ../code/MainWin.cpp 
    ../code/MainCommon.cpp 
    ../code/ImGui/ImGuiMain.cpp
    ../code/ImGui/ImGuiMain.h
    ../code/ImGui/imgui.cpp
    ../code/ImGui/imgui_draw.cpp
    ../code/ImGui/imgui_freetype.cpp
    ../code/ImGui/imgui_tables.cpp
    ../code/ImGui/imgui_widgets.cpp
    ../code/ImGui/imgui.h
    ../code/ImGui/imconfig.h
    ../code/ImGui/imgui_impl_dx11.cpp
    ../code/ImGui/imgui_impl_dx11.h 
    ../code/ImGui/imgui_impl_win32.cpp
    ../code/ImGui/imgui_impl_win32.h)

add_executable(${APPNAME} ${SOURCE_FILES})
if (WIN32)
    set_target_properties(${APPNAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
target_link_options(${APPNAME} PUBLIC /DEBUG:FULL)
target_precompile_headers(${APPNAME} PRIVATE ../code/Core.h)
target_include_directories(${APPNAME} PRIVATE ../code/ImGui/freetype)
target_link_directories(${APPNAME} PRIVATE ../code/ImGui/freetype)
target_compile_definitions(${APPNAME} PRIVATE COMPILE_DEFINITIONS -DCONFIG_APP_NAME="${APPNAME}")
target_link_libraries(${APPNAME} "d3d11.lib" "freetype.lib")
target_link_options(${APPNAME} PRIVATE /ignore:4099)   # warning LNK4099: PDB '' was not found with 'freetype.lib'
