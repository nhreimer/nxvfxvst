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

if ( BUILD_PLUGIN )

  message( STATUS "BUILDING VST3 PLUGIN" )

  project(nxvfxvst
    # This is your plug-in version number. Change it here only.
    # Version number symbols usable in C++ can be found in
    # source/version.h and ${PROJECT_BINARY_DIR}/projectversion.h.
    VERSION 1.0.1.1
    DESCRIPTION "nxvfxvst VST 3 Plug-in"
  )

#  set(SFML_STATIC_LIBRARIES TRUE)
#  find_package(SFML 3.0.0 COMPONENTS Graphics Window REQUIRED)
#  find_package(spdlog CONFIG REQUIRED)
  find_package(ImGui-SFML REQUIRED)
  find_package(nlohmann_json CONFIG REQUIRED)

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
  )

  target_include_directories( ${PROJECT_NAME}
    PRIVATE

    # must used the patched version
    C:/Precision/projects/SFML/include
  )

  if( CMAKE_BUILD_TYPE STREQUAL "Debug" )

    find_package(spdlog CONFIG REQUIRED)

    target_link_libraries( ${PROJECT_NAME}
      PRIVATE

      sdk
      Rpcrt4.lib

      # must use the patched version
      C:/Precision/projects/SFML/cmake-build-debug/lib/sfml-system-s-d.lib
      C:/Precision/projects/SFML/cmake-build-debug/lib/sfml-window-s-d.lib
      C:/Precision/projects/SFML/cmake-build-debug/lib/sfml-graphics-s-d.lib
  #    SFML::Window
  #    SFML::Graphics
      ImGui-SFML::ImGui-SFML
      spdlog::spdlog_header_only
      nlohmann_json::nlohmann_json
    )

  else()

    target_link_libraries( ${PROJECT_NAME}
      PRIVATE

      sdk
      Rpcrt4.lib

      nlohmann_json::nlohmann_json

      # must use the patched version
      C:/Precision/projects/SFML/cmake-build-release/lib/sfml-system-s.lib
      C:/Precision/projects/SFML/cmake-build-release/lib/sfml-window-s.lib
      C:/Precision/projects/SFML/cmake-build-release/lib/sfml-graphics-s.lib
      #    SFML::Window
      #    SFML::Graphics
      ImGui-SFML::ImGui-SFML
#      spdlog::spdlog_header_only
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
  # TODO: this should be set on the CLI
  # set(vst3sdk_SOURCE_DIR "C:/Precision/projects/VST_SDKs/VST_SDK_3.7.12/vst3sdk")

  set(SFML_STATIC_LIBRARIES TRUE)
  find_package(SFML 3.0.0 COMPONENTS Graphics Window REQUIRED)
  find_package(spdlog CONFIG REQUIRED)
  find_package(ImGui-SFML REQUIRED)
  find_package(nlohmann_json CONFIG REQUIRED)

  add_executable(${PROJECT_NAME} main.cpp)

  target_link_libraries( ${PROJECT_NAME}
    PRIVATE

    #  sdk
    Rpcrt4.lib

    SFML::Window
    SFML::Graphics
    ImGui-SFML::ImGui-SFML
    spdlog::spdlog_header_only
    nlohmann_json::nlohmann_json
  )

endif()

target_include_directories( ${PROJECT_NAME}
  PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${vst3sdk_SOURCE_DIR}
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