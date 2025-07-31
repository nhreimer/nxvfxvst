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

# this gets set in the CMakePresets.json (from the VST3_SDK_ROOT environment variable)
if(NOT DEFINED vst3sdk_SOURCE_DIR OR "${vst3sdk_SOURCE_DIR}" STREQUAL "")
  message(FATAL_ERROR "vst3sdk_SOURCE_DIR environment variable is not set or empty")
else()
  message(STATUS "found VST3 SDK directory: ${vst3sdk_SOURCE_DIR}")
endif()

set(SFML_STATIC_LIBRARIES TRUE)

find_package(SFML COMPONENTS Graphics Window REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(ImGui-SFML REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-concurrentqueue CONFIG REQUIRED)
find_package(kissfft CONFIG REQUIRED COMPONENTS float)

if ( ${CMAKE_BUILD_VST3} )
  message( STATUS "BUILDING VST3 PLUGIN" )

  option(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES "Enable VST 3 Plug-in Examples" OFF)
  option(SMTG_ENABLE_VST3_HOSTING_EXAMPLES "Enable VST 3 Hosting Examples" OFF)
  option(SMTG_ENABLE_VSTGUI_SUPPORT "Enable VSTGUI Support" OFF)

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
#    ${NX_CPP_FILES}
  )

  smtg_target_configure_version_file(${PROJECT_NAME})

else()
  message( STATUS "BUILDING APPLICATION" )

  add_executable(${PROJECT_NAME}
    test_main.cpp
  )
endif()

target_include_directories( ${PROJECT_NAME}
  PRIVATE

  ${vst3sdk_SOURCE_DIR}
)

target_link_libraries( ${PROJECT_NAME}
  PRIVATE

  if ( WIN32 )
    Rpcrt4.lib
    dwmapi.lib
  endif()

  nlohmann_json::nlohmann_json
  SFML::Window
  SFML::Graphics
  ImGui-SFML::ImGui-SFML

  unofficial::concurrentqueue::concurrentqueue
  kissfft::kissfft

  if ( ${CMAKE_BUILD_VST3} )
    sdk
  endif()
)

