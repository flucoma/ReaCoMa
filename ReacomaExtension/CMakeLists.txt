cmake_minimum_required(VERSION 3.16)

if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "MSVC Runtime Library" FORCE)
endif()

project(ReacomaExtension VERSION 1.0.0 LANGUAGES C CXX)

if(WIN32 AND NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "This project requires 64-bit compilation. Please configure with: cmake .. -A x64")
endif()

if(MSVC)
  foreach(flag_var CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELEASE)
    string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
  endforeach()
endif()

add_compile_definitions(NOMINMAX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/EHsc)
endif()

set(BINARY_NAME ReacomaExtension)

set(IPLUG2_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/iPlug2")
set(FLUCOMA_CORE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/flucoma-core")

set(DEPS_PATH "${IPLUG2_ROOT}/Dependencies")
set(WDL_PATH "${IPLUG2_ROOT}/WDL")
set(IPLUG_PATH "${IPLUG2_ROOT}/IPlug")
set(IGRAPHICS_PATH "${IPLUG2_ROOT}/IGraphics")
set(REAPER_SDK_PATH "${DEPS_PATH}/IPlug/REAPER_SDK")
set(SWELL_PATH "${WDL_PATH}/swell")

set(IPLUG_CORE_MAIN_SOURCES
    "../dependencies/iPlug2/IPlug/IPlug_include_in_plug_hdr.h"
    "../dependencies/iPlug2/IPlug/IPlug_include_in_plug_src.h"
    "../dependencies/iPlug2/IPlug/IPlugAPIBase.h"
    "../dependencies/iPlug2/IPlug/IPlugAPIBase.cpp"
    "../dependencies/iPlug2/IPlug/IPlugConstants.h"
    "../dependencies/iPlug2/IPlug/IPlugEditorDelegate.h"
    "../dependencies/iPlug2/IPlug/IPlugLogger.h"
    "../dependencies/iPlug2/IPlug/IPlugMidi.h"
    "../dependencies/iPlug2/IPlug/IPlugParameter.h"
    "../dependencies/iPlug2/IPlug/IPlugParameter.cpp"
    "../dependencies/iPlug2/IPlug/IPlugPaths.h"
    "../dependencies/iPlug2/IPlug/IPlugPlatform.h"
    "../dependencies/iPlug2/IPlug/IPlugPluginBase.h"
    "../dependencies/iPlug2/IPlug/IPlugPluginBase.cpp"
    "../dependencies/iPlug2/IPlug/IPlugProcessor.h"
    "../dependencies/iPlug2/IPlug/IPlugProcessor.cpp"
    "../dependencies/iPlug2/IPlug/IPlugQueue.h"
    "../dependencies/iPlug2/IPlug/IPlugStructs.h"
    "../dependencies/iPlug2/IPlug/IPlugTimer.h"
    "../dependencies/iPlug2/IPlug/IPlugTimer.cpp"
    "../dependencies/iPlug2/IPlug/IPlugUtilities.h"
    "../dependencies/iPlug2/IPlug/Extras/NChanDelay.h"
)

set(IGRAPHICS_MAIN_SOURCES
    "../dependencies/iPlug2/IGraphics/IControl.h"
    "../dependencies/iPlug2/IGraphics/IControl.cpp"
    "../dependencies/iPlug2/IGraphics/IGraphics_select.h"
    "../dependencies/iPlug2/IGraphics/IGraphics.h"
    "../dependencies/iPlug2/IGraphics/IGraphics.cpp"
    "../dependencies/iPlug2/IGraphics/IGraphicsConstants.h"
    "../dependencies/iPlug2/IGraphics/IGraphicsEditorDelegate.h"
    "../dependencies/iPlug2/IGraphics/IGraphicsEditorDelegate.cpp"
    "../dependencies/iPlug2/IGraphics/IGraphicsPopupMenu.h"
    "../dependencies/iPlug2/IGraphics/IGraphicsPrivate.h"
    "../dependencies/iPlug2/IGraphics/IGraphicsStructs.h"
    "../dependencies/iPlug2/IGraphics/IGraphicsUtilities.h"
    "../dependencies/iPlug2/IGraphics/Controls/IControls.h"
    "../dependencies/iPlug2/IGraphics/Controls/IControls.cpp"
    "../dependencies/iPlug2/IGraphics/Controls/IPopupMenuControl.h"
    "../dependencies/iPlug2/IGraphics/Controls/IPopupMenuControl.cpp"
    "../dependencies/iPlug2/IGraphics/Controls/ITextEntryControl.h"
    "../dependencies/iPlug2/IGraphics/Controls/ITextEntryControl.cpp"
    "../dependencies/iPlug2/IGraphics/Drawing/IGraphicsSkia.h"
)

if (APPLE)
  set(IPLUG_CORE_PATH_SOURCES "../dependencies/iPlug2/IPlug/IPlugPaths.mm")
  set(IGRAPHICS_PLATFORM_SOURCES 
    "../dependencies/iPlug2/IGraphics/Platforms/IGraphicsCoreText.h"
    "../dependencies/iPlug2/IGraphics/Platforms/IGraphicsCoreText.mm"
    "../dependencies/iPlug2/IGraphics/Platforms/IGraphicsMac_view.h"
    "../dependencies/iPlug2/IGraphics/Platforms/IGraphicsMac_view.mm"
    "../dependencies/iPlug2/IGraphics/Platforms/IGraphicsMac.h"
    "../dependencies/iPlug2/IGraphics/Platforms/IGraphicsMac.mm"
    "../dependencies/iPlug2/IGraphics/Drawing/IGraphicsNanoVG_src.m"
    )
elseif (WIN32)
  set(IPLUG_CORE_PATH_SOURCES "../dependencies/iPlug2/IPlug/IPlugPaths.cpp")
  set(IGRAPHICS_PLATFORM_SOURCES 
    "../dependencies/iPlug2/IGraphics/Platforms/IGraphicsWin_dnd.h"
    "../dependencies/iPlug2/IGraphics/Platforms/IGraphicsWin.h"
    "../dependencies/iPlug2/IGraphics/Platforms/IGraphicsWin.cpp"
    "../dependencies/iPlug2/IGraphics/Platforms/IGraphicsWinFonts.h"
    )
endif()

set(IPLUG_CORE_SOURCES
  ${IPLUG_CORE_MAIN_SOURCES} 
  ${IPLUG_CORE_PATH_SOURCES}
)

set(IGRAPHICS_SOURCES 
  ${IGRAPHICS_MAIN_SOURCES}
  ${IGRAPHICS_PLATFORM_SOURCES}
)

if(APPLE)
  set(SWELL_SOURCES "${SWELL_PATH}/swell-modstub.mm")
elseif(WIN32)
  set(SWELL_SOURCES "") # SWELL is not used on Windows
else() # Linux
  set(SWELL_SOURCES "${SWELL_PATH}/swell-modstub-generic.cpp")
endif()

set(REACOMA_SOURCES
    # Main project files
    "ReacomaExtension.cpp"
    "ReacomaExtension.h"
    "config.h"
    "VectorBufferAdaptor.cpp"
    "VectorBufferAdaptor.h"

    # iPlug2 sources
    ${IPLUG_CORE_SOURCES}
    ${IGRAPHICS_SOURCES}
    ${SWELL_SOURCES}

    # Algorithms
    "Algorithms/AmpGateAlgorithm.cpp"
    "Algorithms/AmpGateAlgorithm.h"
    "Algorithms/FlucomaAlgorithmBase.h"
    "Algorithms/HPPSAlgorithm.cpp"
    "Algorithms/HPSSAlgorithm.h"
    "Algorithms/IAlgorithm.cpp"
    "Algorithms/IAlgorithm.h"
    "Algorithms/NMFAlgorithm.cpp"
    "Algorithms/NMFAlgorithm.h"
    "Algorithms/NoveltySliceAlgorithm.cpp"
    "Algorithms/NoveltySliceAlgorithm.h"
    "Algorithms/OnsetSliceAlgorithm.cpp"
    "Algorithms/OnsetSliceAlgorithm.h"
    "Algorithms/ProcessingJob.cpp"
    "Algorithms/ProcessingJob.h"
    "Algorithms/TransientAlgorithm.cpp"
    "Algorithms/TransientAlgorithm.h"
    "Algorithms/TransientSliceAlgorithm.cpp"
    "Algorithms/TransientSliceAlgorithm.h"
    "Algorithms/AmpSliceAlgorithm.cpp"
    "Algorithms/AmpSliceAlgorithm.h"

    # UI Components
    "Components/ReacomaButton.cpp"
    "Components/ReacomaButton.h"
    "Components/ReacomaParamTextControl.cpp"
    "Components/ReacomaParamTextControl.h"
    "Components/ReacomaProgressBar.h"
    "Components/ReacomaProgressBar.cpp"
    "Components/ReacomaSegmented.cpp"
    "Components/ReacomaSegmented.h"
    "Components/ReacomaSlider.cpp"
    "Components/ReacomaSlider.h"
    "Components/ReacomaDropdown.cpp"
    "Components/ReacomaDropdown.h"

    # Resources
    "resources/resource.h"
    "resources/ibmplexmono.cpp"
    "resources/roboto.cpp"
)

if(WIN32)
    list(APPEND REACOMA_SOURCES "resources/main.rc")
endif()

add_library(${BINARY_NAME} SHARED ${REACOMA_SOURCES})

if(MSVC)
    set_property(TARGET ${BINARY_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Add flucoma-core subdirectory
# The static runtime library setting above will be inherited
add_subdirectory(${FLUCOMA_CORE_PATH} ${CMAKE_BINARY_DIR}/flucoma-core-build)

target_link_libraries(${BINARY_NAME} PUBLIC FLUID_DECOMPOSITION)

target_include_directories(${BINARY_NAME} PRIVATE
    # Project-specific paths
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/Algorithms"
    "${CMAKE_CURRENT_SOURCE_DIR}/Components"
    "${CMAKE_CURRENT_SOURCE_DIR}/resources"

    # iPlug2 core paths
    "${IPLUG2_ROOT}"
    "${IPLUG_PATH}"
    "${IPLUG_PATH}/ReaperExt"
    "${IPLUG_PATH}/Extras"
    "${WDL_PATH}"
    "${SWELL_PATH}"

    # IGraphics paths (as defined in IGRAPHICS_INC_PATHS)
    "${IGRAPHICS_PATH}"
    "${IGRAPHICS_PATH}/Controls"
    "${IGRAPHICS_PATH}/Platforms"
    "${IGRAPHICS_PATH}/Drawing"
    "${IGRAPHICS_PATH}/Extras"
    "${DEPS_PATH}/IGraphics/NanoSVG/src"
    "${DEPS_PATH}/IGraphics/NanoVG/src"
    "${DEPS_PATH}/IGraphics/MetalNanoVG/src"
    "${DEPS_PATH}/IGraphics/STB"
    "${DEPS_PATH}/IGraphics/yoga"
    "${DEPS_PATH}/IGraphics/yoga/yoga"
    "${DEPS_PATH}/IGraphics/glad_GL2/include"
    "${DEPS_PATH}/IGraphics/glad_GL2/src"

    # REAPER SDK path
    "${REAPER_SDK_PATH}"
)

if (APPLE)
    set_source_files_properties(
        "../dependencies/iPlug2/IGraphics/Drawing/IGraphicsNanoVG_src.m"
        PROPERTIES
        COMPILE_FLAGS -fobjc-arc
    )
endif()

target_compile_definitions(${BINARY_NAME} PRIVATE
    OBJC_PREFIX=vReacomaExtension
    SWELL_PROVIDED_BY_APP
    LICE_PROVIDED_BY_APP
    IGRAPHICS_NANOVG
)

if(APPLE)
    target_compile_definitions(${BINARY_NAME} PRIVATE IGRAPHICS_METAL)

    target_link_libraries(${BINARY_NAME} PRIVATE
        "-framework Metal"
        "-framework MetalKit"
        "-framework OpenGL"
        "-framework Accelerate"
        "-framework Cocoa"
        "-framework Carbon"
        "-framework CoreFoundation"
        "-framework QuartzCore"
    )
    
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS deployment version")
elseif(WIN32)
    target_compile_definitions(${BINARY_NAME} PRIVATE IGRAPHICS_GL2)

    target_link_libraries(${BINARY_NAME} PRIVATE
        gdi32
        user32
        Comctl32
        Shlwapi
        Wininet
        opengl32
    )
endif()

# Use appropriate optimization flags for MSVC vs. GCC/Clang
if(MSVC)
    target_compile_options(${BINARY_NAME} PRIVATE
        $<$<CONFIG:RELEASE>:/O2> # Max speed for Release
        $<$<CONFIG:DEBUG>:/Od>   # Disable optimization for Debug
    )
else()
    target_compile_options(${BINARY_NAME} PRIVATE
        $<$<CONFIG:RELEASE>:-O3>
        $<$<CONFIG:DEBUG>:-O0>
    )
endif()

# Apply compiler-specific warning suppression flags.
if(MSVC)
    # Set warning level to 0 for MSVC, which suppresses all warnings.
    target_compile_options(${BINARY_NAME} PRIVATE /W0)
else()
    # -w suppresses all warnings on GCC/Clang.
    target_compile_options(${BINARY_NAME} PRIVATE -w)
endif()


if(APPLE AND CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
    message(STATUS "Intel Mac detected. Adding x86-specific SSE flags.")
    target_compile_options(${BINARY_NAME} PRIVATE
        -mfpmath=sse
        -msse
        -msse2
        -msse3
    )
else()
    message(STATUS "Apple Silicon or non-Mac host detected. Skipping x86-specific SSE flags.")
endif()

set_target_properties(${BINARY_NAME} PROPERTIES OUTPUT_NAME "reaper_ReacomaExtension")
set_target_properties(${BINARY_NAME} PROPERTIES PREFIX "")

option(DEPLOYMENT_BUILD "Build for packaging/deployment instead of local REAPER installation" OFF)

set(OUTPUT_PATH "")

if(DEPLOYMENT_BUILD)
    set(OUTPUT_PATH "${CMAKE_BINARY_DIR}/dist")
    message(STATUS "Deployment build enabled. Output will be in: ${OUTPUT_PATH}")
else()
    if(APPLE)
        set(OUTPUT_PATH "$ENV{HOME}/Library/Application Support/REAPER/UserPlugins")
    elseif(WIN32)
        set(OUTPUT_PATH "$ENV{APPDATA}/REAPER/UserPlugins")
    else() # Linux
        set(OUTPUT_PATH "$ENV{HOME}/.config/REAPER/UserPlugins")
    endif()
    message(STATUS "Development build. Installing to REAPER UserPlugins: ${OUTPUT_PATH}")
endif()

if(APPLE)
    set_target_properties(${BINARY_NAME} PROPERTIES SUFFIX ".dylib")
elseif(WIN32)
    set_target_properties(${BINARY_NAME} PROPERTIES SUFFIX ".dll")
else() # Linux
    set_target_properties(${BINARY_NAME} PROPERTIES SUFFIX ".so")
endif()

set_target_properties(${BINARY_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_PATH}")
set_target_properties(${BINARY_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_PATH}")