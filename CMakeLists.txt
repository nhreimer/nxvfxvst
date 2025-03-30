cmake_minimum_required(VERSION 3.30)
project(RVTestPlatform)

set(CMAKE_CXX_STANDARD 20)

set(vst3sdk_SOURCE_DIR "C:/Precision/projects/VST_SDKs/VST_SDK_3.7.12/vst3sdk")

set(SFML_STATIC_LIBRARIES TRUE)
find_package(SFML 3.0.0 COMPONENTS Graphics Window REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(ImGui-SFML REQUIRED)
#find_package(nlohmann_json CONFIG REQUIRED)
#find_package(EnTT CONFIG REQUIRED)

add_executable(RVTestPlatform main.cpp)

target_include_directories( RVTestPlatform
  PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${vst3sdk_SOURCE_DIR}
)

target_link_libraries( RVTestPlatform
  PRIVATE

#  sdk
  Rpcrt4.lib

  SFML::Window
  SFML::Graphics
  ImGui-SFML::ImGui-SFML
  spdlog::spdlog_header_only
#  nlohmann_json::nlohmann_json
#  EnTT::EnTT
)

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )

  message( "DEBUG BUILD" )

  set( COMPILE_DEFS
    -DNOMINMAX
    -DWINDOWS_LEAN_AND_MEAN
    -DWIN32
    -D_WINDOWS
    -D_UNICODE
    -DUNICODE
    -DSPDLOG_USE_STD_FORMAT
    -DDEBUG
  )
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
  )

  add_definitions( ${COMPILE_DEFS} )

endif()