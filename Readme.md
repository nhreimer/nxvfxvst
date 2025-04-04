# nxvfxvst (Nick's VFX VST)

### Description

A header-only VFX VST (and standalone playground) for Midi events. 

This is a highly configurable and modular graphics plugin for implementing your own algorithms or shaders 
that react to midi events.

### Features

* Midi Generator that pushes events on a different thread in order to 
simulate VST's Processor thread. 
* Ability to run in Standalone (for testing only) and VST3 Plugin
* Infinite shader chaining
* JSON serialization for importing/exporting (via clipboard at the moment)
* Real-time VFX that synchronizes to music

### Dependencies

* C++20
* SFML v2.6.2 (graphics: v3 has a bug)
* ImGui (menus)
* nlohmann json (serialization)
* spdlog (logging)
* VST3 SDK

The VST3 SDK can be downloaded from here https://www.steinberg.net/vst3sdk

### Roadmap

* Save and load from files
* Multichannel

# Getting Started

Regardless of which version you build, you will need to specify where the 
VST3 SDK lives and where your dependency manager resides:

```bash
-Dvst3sdk_SOURCE_DIR=C:/path/to/vst3sdk
-DVCPKG_TARGET_TRIPLET=x64-windows-static-md
-DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

## Standalone Application

## VST3 Plugin

Using logging when debugging. You'll want to log to a file 
and the plugin may crash if the directory cannot be found or 
there are permissions issues. Set the directory in CMakeLists.txt
or use -DNX_LOG_FILE=C:/path/to/log/file.log