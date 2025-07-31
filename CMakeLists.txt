cmake_minimum_required(VERSION 3.28)

project(nxvfxvst
  VERSION 1.1.5.0
  DESCRIPTION "nxvfxvst VST 3 Plug-in"
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(GitVersion)

set(CMAKE_CXX_STANDARD 20)

message( STATUS "PROJECT: ${PROJECT_NAME}" )
message( STATUS "BUILD: ${CMAKE_BUILD_TYPE}" )
message( STATUS "CMAKE_BINARY_DIR is: ${CMAKE_BINARY_DIR}" )

if ( NX_LOG_FILE )
  message( STATUS "Logging file set to: `${NX_LOG_FILE}`" )
  set( NX_LOG_FILE_DISABLED FALSE )
else()
  set( NX_LOG_FILE_DISABLED TRUE )
endif()

# this gets set in the CMakePresets.json (from the VST3_SDK_ROOT environment variable)
if(NOT DEFINED vst3sdk_SOURCE_DIR OR "${vst3sdk_SOURCE_DIR}" STREQUAL "")
  message(FATAL_ERROR "vst3sdk_SOURCE_DIR environment variable is not set or empty")
else()
  message(STATUS "found VST3 SDK directory: ${vst3sdk_SOURCE_DIR}")
endif()

set(SFML_STATIC_LIBRARIES TRUE)

#-----------------------------------------------------------------------------#

find_package(SFML COMPONENTS Graphics Window REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(ImGui-SFML REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-concurrentqueue CONFIG REQUIRED)
find_package(kissfft CONFIG REQUIRED COMPONENTS float)

#-----------------------------------------------------------------------------#

set( NX_CPP_FILES

  utils/LazyTexture.cpp

  shapes/CurvedLine.cpp

  models/encoder/RawRGBAEncoder.cpp

  models/modifier/MirrorModifier.cpp
  models/modifier/ParticleFullMeshLineModifier.cpp
  models/modifier/ParticleSequentialLineModifier.cpp
  models/modifier/PerlinDeformerModifier.cpp
  models/modifier/RingZoneMeshModifier.cpp

  models/particle/behavior/FreeFallBehavior.cpp
  models/particle/behavior/JitterBehavior.cpp
  models/particle/behavior/MagneticBehavior.cpp

  models/particle/layout/EllipticalLayout.cpp
  models/particle/layout/FractalRingLayout.cpp
  models/particle/layout/GoldenSpiralLayout.cpp
  models/particle/layout/LissajousCurveLayout.cpp
  models/particle/layout/LSystemCurveLayout.cpp
  models/particle/layout/RandomParticleLayout.cpp
  models/particle/layout/SpiralParticleLayout.cpp

  models/channel/MidiChannelPipeline.cpp
  models/ModifierPipeline.cpp
  models/MultichannelPipeline.cpp
  models/ParticleBehaviorPipeline.cpp
  models/ParticleLayoutManager.cpp
  models/ShaderPipeline.cpp

  models/shader/BlenderShader.cpp
  models/shader/BlurShader.cpp
  models/shader/ColorShader.cpp
  models/shader/DensityHeatMapShader.cpp
  models/shader/DualKawaseBlurShader.cpp
  models/shader/FeedbackShader.cpp
  models/shader/KaleidoscopeShader.cpp
  models/shader/LayeredGlitchShader.cpp
  models/shader/RippleShader.cpp
  models/shader/RumbleShader.cpp
  models/shader/ShockBloomShader.cpp
  models/shader/SmearShader.cpp
  models/shader/StrobeShader.cpp
  models/shader/TransformShader.cpp
)

#-----------------------------------------------------------------------------#

if ( ${CMAKE_BUILD_VST3} )
  message( STATUS "BUILDING VST3 PLUGIN" )

  option(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES "Enable VST 3 Plug-in Examples" OFF)
  option(SMTG_ENABLE_VST3_HOSTING_EXAMPLES "Enable VST 3 Hosting Examples" OFF)
  option(SMTG_ENABLE_VSTGUI_SUPPORT "Enable VSTGUI Support" OFF)
  option(SMTG_MDA_VST3_VST2_COMPATIBLE "Build MDA Examples" OFF)
  option(SMTG_ADD_VST3_UTILITIES "Build VST Utilities" OFF)
  option(SMTG_CXX_STANDARD "CXX Standard" 20) # this might not be required since we inform CMake already
  if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    option(SMTG_ENABLE_TARGET_VARS_LOG "" ON)
  endif()
  # you can use this to turn on /MT instead of /MD (but it's NOT recommended!). you will also need to
  # go to the presets and remove this "CMAKE_MSVC_RUNTIME_LIBRARY": "MultiThreadedDLL",
  # if ( WIN32 )
  #   option(SMTG_USE_STATIC_CRT "Enable static CRuntime" OFF)
  # endif()

  add_subdirectory(${vst3sdk_SOURCE_DIR} ${vst3sdk_SOURCE_DIR})
  smtg_enable_vst3_sdk()

  smtg_add_vst3plugin(${PROJECT_NAME}
    vst/version.h
    vst/myplugincids.h
    vst/mypluginprocessor.h
    vst/mypluginprocessor.cpp
    vst/myplugincontroller.h
    vst/myplugincontroller.cpp
    vst/mypluginentry.cpp

    vst/EventFacadeVst.cpp
    ${NX_CPP_FILES}
  )

  smtg_target_configure_version_file(${PROJECT_NAME})

  if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

    smtg_target_set_debug_executable(${PROJECT_NAME}
      "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
      "--pluginfolder \"$(OutDir)/\""
    )
  endif()

else()
  message( STATUS "BUILDING APPLICATION" )

  add_executable(${PROJECT_NAME}
    main.cpp

    app/EventFacadeApp.cpp
    ${NX_CPP_FILES}
  )
endif()

#-----------------------------------------------------------------------------#

target_include_directories( ${PROJECT_NAME}
  PRIVATE

  ${CMAKE_SOURCE_DIR}
  ${vst3sdk_SOURCE_DIR}
)

#-----------------------------------------------------------------------------#

target_link_libraries( ${PROJECT_NAME}
  PRIVATE

  $<$<PLATFORM_ID:Windows>:dwmapi.lib>
  $<$<BOOL:${CMAKE_BUILD_VST3}>:sdk>

  nlohmann_json::nlohmann_json
  SFML::Window
  SFML::Graphics
  ImGui-SFML::ImGui-SFML

  unofficial::concurrentqueue::concurrentqueue
  kissfft::kissfft
)

#-----------------------------------------------------------------------------#

target_precompile_headers( ${PROJECT_NAME}
  PRIVATE
  helpers/CommonHeaders.hpp
)

#-----------------------------------------------------------------------------#

if ( NX_LOG_FILE_DISABLED )
  add_compile_definitions( NX_LOG_FILE_DISABLED=1 )
else()
  add_compile_definitions( NX_LOG_FILE="${NX_LOG_FILE}" )
  add_compile_definitions( NX_LOG_FILE_DISABLED=0 )
endif()

#-----------------------------------------------------------------------------#

# required for fmt and msvc to force utf-8
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

#-----------------------------------------------------------------------------#

# fdebug.h in the VST3 SDK requires _DEBUG and RELEASE to be defined
if ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
  add_definitions( -DBUILD_NUMBER="d${BUILD_NUMBER}" -D_DEBUG -DDEBUG )
else()
  add_definitions( -DBUILD_NUMBER="r${BUILD_NUMBER}" -DRELEASE )
endif()

#-----------------------------------------------------------------------------#