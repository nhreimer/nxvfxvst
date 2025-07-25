cmake_minimum_required(VERSION 3.30)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(GitVersion)

set(CMAKE_CXX_STANDARD 20)

message( "BUILD: ${CMAKE_BUILD_TYPE}" )

if ( NX_LOG_FILE )
  message(STATUS "Logging directory set to: ${NX_LOG_FILE}")
  set( NX_LOG_FILE_DISABLED FALSE )
else()
  set( NX_LOG_FILE_DISABLED TRUE )
endif()

if(NOT vst3sdk_SOURCE_DIR)
  message(FATAL_ERROR "Path to VST3 SDK is empty!")
else()
  message(STATUS "Path to VST3 SDK found")
endif()

# required for fmt and msvc to force utf-8
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set( NX_CPP_FILES

  utils/LazyTexture.cpp

  shapes/CurvedLine.cpp

#  models/encoder/FFMpegEncoder.cpp
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

if ( BUILD_PLUGIN )

  message( STATUS "BUILDING VST3 PLUGIN" )

  project(nxvfxvst
    # This is your plug-in version number. Change it here only.
    # Version number symbols usable in C++ can be found in
    # source/version.h and ${PROJECT_BINARY_DIR}/projectversion.h.
    VERSION 1.1.3.0
    DESCRIPTION "nxvfxvst VST 3 Plug-in"
  )

  set(SFML_STATIC_LIBRARIES TRUE)
  find_package(SFML 3.0.0 COMPONENTS Graphics Window REQUIRED)
#  find_package(spdlog CONFIG REQUIRED)
  find_package(ImGui-SFML REQUIRED)
  find_package(nlohmann_json CONFIG REQUIRED)
  find_package(unofficial-concurrentqueue CONFIG REQUIRED)
  find_package(kissfft CONFIG REQUIRED COMPONENTS float)

  option(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES "Enable VST 3 Plug-in Examples" OFF)
  option(SMTG_ENABLE_VST3_HOSTING_EXAMPLES "Enable VST 3 Hosting Examples" OFF)

  #add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
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

  target_include_directories( ${PROJECT_NAME}
    PRIVATE

    # must used the patched version
#    C:/Precision/projects/SFML/include
  )

  if( CMAKE_BUILD_TYPE STREQUAL "Debug" )

    find_package(spdlog CONFIG REQUIRED)
    target_link_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARY_DIRS})
    target_link_libraries( ${PROJECT_NAME}
      PRIVATE

      sdk
      Rpcrt4.lib
      dwmapi.lib

      # you should use the patched version in the README.MD file
#      C:/Precision/projects/SFML/cmake-build-debug/lib/sfml-system-s-d.lib
#      C:/Precision/projects/SFML/cmake-build-debug/lib/sfml-window-s-d.lib
#      C:/Precision/projects/SFML/cmake-build-debug/lib/sfml-graphics-s-d.lib
      SFML::Window
      SFML::Graphics
      ImGui-SFML::ImGui-SFML
      spdlog::spdlog_header_only
      nlohmann_json::nlohmann_json

      unofficial::concurrentqueue::concurrentqueue
      kissfft::kissfft
    )

  else()

    target_link_libraries( ${PROJECT_NAME}
      PRIVATE

      sdk
      Rpcrt4.lib
      dwmapi.lib

      nlohmann_json::nlohmann_json
      SFML::Window
      SFML::Graphics
      ImGui-SFML::ImGui-SFML

      unofficial::concurrentqueue::concurrentqueue
      kissfft::kissfft
    )

  endif()

  smtg_target_configure_version_file(nxvfxvst)

  if(SMTG_MAC)
    smtg_target_set_bundle(nxvfxvst
      BUNDLE_IDENTIFIER org.nx.nxvfxvst
      COMPANY_NAME "nx"
    )
    smtg_target_set_debug_executable(nxvfxvst
      "/Applications/VST3PluginTestHost.app"
      "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
    )
  elseif(SMTG_WIN)
    if(MSVC)
      set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

      smtg_target_set_debug_executable(${PROJECT_NAME}
        "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
        "--pluginfolder \"$(OutDir)/\""
      )
    endif()
  endif(SMTG_MAC)

else()

  message( STATUS "BUILDING STANDALONE APPLICATION" )

  project(RVTestPlatform)

  # keep this set no matter what because we rely on some Vst constructs even in the
  # application version.

  set(SFML_STATIC_LIBRARIES TRUE)
  find_package(SFML 3.0.0 COMPONENTS Graphics Window REQUIRED)
  find_package(spdlog CONFIG REQUIRED)
  find_package(ImGui-SFML REQUIRED)
  find_package(nlohmann_json CONFIG REQUIRED)
  find_package(unofficial-concurrentqueue CONFIG REQUIRED)
  find_package(kissfft CONFIG REQUIRED COMPONENTS float)

  add_executable(${PROJECT_NAME}
    main.cpp

    app/EventFacadeApp.cpp

    ${NX_CPP_FILES}
  )
  target_link_directories( ${PROJECT_NAME}
    PRIVATE
    unofficial::concurrentqueue::concurrentqueue
  )

  target_link_libraries( ${PROJECT_NAME}
    PRIVATE

    #  sdk
    Rpcrt4.lib
    dwmapi.lib

    SFML::Window
    SFML::Graphics
    ImGui-SFML::ImGui-SFML
    spdlog::spdlog_header_only
    nlohmann_json::nlohmann_json
    kissfft::kissfft
  )

endif()

target_include_directories( ${PROJECT_NAME}
  PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${vst3sdk_SOURCE_DIR}
  ${FFMPEG_INCLUDE_DIRS}
  ${FFNVCODEC_INCLUDE_DIRS}
)

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )

  set( COMPILE_DEFS
    -DNOMINMAX
    -DWINDOWS_LEAN_AND_MEAN
    -DWIN32
    -D_WINDOWS
    -D_UNICODE
    -DUNICODE
    -DSPDLOG_USE_STD_FORMAT
    -DDEBUG
    -DBUILD_NUMBER="d${BUILD_NUMBER}"
  )

  if ( BUILD_PLUGIN )
    list( APPEND COMPILE_DEFS -DBUILD_PLUGIN )
  endif()

  add_definitions( ${COMPILE_DEFS} )

else()

  set( COMPILE_DEFS
    -DNOMINMAX
    -DWINDOWS_LEAN_AND_MEAN
    -DWIN32
    -D_WINDOWS
    -D_UNICODE
    -DUNICODE
    -DSPDLOG_USE_STD_FORMAT
    -DBUILD_NUMBER="r${BUILD_NUMBER}"
  )

  if ( BUILD_PLUGIN )
    list( APPEND COMPILE_DEFS -DBUILD_PLUGIN )
  endif()

  add_definitions( ${COMPILE_DEFS} )

endif()

target_precompile_headers( ${PROJECT_NAME}
  PRIVATE
  helpers/CommonHeaders.hpp
)

if ( NX_LOG_FILE_DISABLED )
  add_compile_definitions( NX_LOG_FILE_DISABLED=1 )
else()
  add_compile_definitions( NX_LOG_FILE="${NX_LOG_FILE}" )
  add_compile_definitions( NX_LOG_FILE_DISABLED=0 )
endif()