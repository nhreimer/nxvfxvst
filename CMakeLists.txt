cmake_minimum_required(VERSION 3.28)


project(nxvfxvst
  # This is your plug-in version number. Change it here only.
  # Version number symbols usable in C++ can be found in
  # source/version.h and ${PROJECT_BINARY_DIR}/projectversion.h.
  VERSION 1.1.4.0
  DESCRIPTION "nxvfxvst VST 3 Plug-in"
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(GitVersion)

set(CMAKE_CXX_STANDARD 20)

message( "BUILD: ${CMAKE_BUILD_TYPE}" )

set(SFML_STATIC_LIBRARIES TRUE)
find_package(SFML 3.0.0 COMPONENTS Graphics Window REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(ImGui-SFML REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-concurrentqueue CONFIG REQUIRED)
find_package(kissfft CONFIG REQUIRED COMPONENTS float)

add_executable(${PROJECT_NAME}
  test_main.cpp
)

target_include_directories( ${PROJECT_NAME}
  PRIVATE
)

target_link_libraries( ${PROJECT_NAME}
  PRIVATE

  # sdk

  nlohmann_json::nlohmann_json
  SFML::Window
  SFML::Graphics
  ImGui-SFML::ImGui-SFML

  unofficial::concurrentqueue::concurrentqueue
  kissfft::kissfft
)

