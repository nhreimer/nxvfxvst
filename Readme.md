# nxvfx VST3

### Description

A midi event VFX engine for VST3 that comes with a standalone test playground application. 

This is a highly configurable and modular graphics plugin for implementing your own algorithms or shaders 
that react to midi events.

### Features

* Ability to run in Standalone and VST3 Plugin
  * In Standalone (testing only, no audio)
    * Midi Generator that pushes events on different threads in order to
      simulate DAW Processor threads.
* Multichannel support
  * Route midi output to independent VFX chains
  * Infinite shader chaining per channel
* Each effect can be assigned user-specified midi notes for triggers
* JSON serialization for importing/exporting (via clipboard at the moment)
* Real-time VFX Engine that synchronize to midi events
  * Effects
    * Blur (Gaussian -- directional, strength, brighten, adjustable blur granularity)
    * Glitch (chroma flicker, noise, scanlines, pixel jumps, band counts, strobe)
    * Kaleidoscope (segments, rotation, centering)
    * Ripple (position, decay, speed, frequency, amplitude)
    * Rumble (noise, color desync, decay, direction, frequency, strength)
    * Strobe (decay, strength)
  * Particle generator
    * Random (entirely random)
    * Spiral (spiral positions around the screen based on midi note)
    * None (good for strobing without particles)
  * Particle connectors
    * Sequential (connects one particle to the next particle)
    * Mesh (connects one particle to all other particles)
    * None (no lines)
  * Easings (for time decays)
    * many! (for better or worse)
    * Independent layered easings for smooth visuals

### Dependencies

* C++20
* SFML v3 (graphics: small patch required)
* ImGui (menus)
* nlohmann json (serialization and state management)
* spdlog (logging)
* VST3 SDK

The VST3 SDK can be downloaded from here https://www.steinberg.net/vst3sdk

### Feature Roadmap

* Save and load to/from files instead of copy/paste
* Add parameters for VST3 for automation
* Add OS support for Linux and Mac
* More VFX, more particles, more particle modifiers!
* Better UI

# Getting Started

Regardless of which version you build, you will need to specify where the 
VST3 SDK lives and where your dependency manager resides:

```bash
-Dvst3sdk_SOURCE_DIR=C:/path/to/vst3sdk
-DVCPKG_TARGET_TRIPLET=x64-windows-static-md
-DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
-DNX_LOG_FILE=C:/path/to/log/file.log
-DBUILD_PLUGIN=OFF
```

## Standalone Application

Set BUILD_PLUGIN to OFF or FALSE. You do not need to make any SFML 
code adjustments for the standalone application. 

```bash
-DBUILD_PLUGIN=OFF
```

## VST3 Plugin

You'll need to enable BUILD_PLUGIN.

You'll want to log to a file when debugging 
and the plugin may crash if the directory cannot be found or
there are permissions issues. Set the directory in CMakeLists.txt
or use -DNX_LOG_FILE=C:/path/to/log/file.log

```bash
-Dvst3sdk_SOURCE_DIR=C:/path/to/vst3sdk
-DVCPKG_TARGET_TRIPLET=x64-windows-static-md
-DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
-DNX_LOG_FILE=C:/path/to/log/file.log
-DBUILD_PLUGIN=ON
```

WARNING: If you want to build the VST plugin version then there's a problem in
SFML's RenderWindow.cpp that MUST be fixed or SFML will crash.
glCheck() is the culprit, but I haven't dug into more than this.

```cpp
void RenderWindow::onCreate()
{
    if (priv::RenderTextureImplFBO::isAvailable())
    {
        // Retrieve the framebuffer ID we have to bind when targeting the window for rendering
        // We assume that this window's context is still active at this point
        
        // NX: something in glCheck causes this to crash when as a child window
        // NX: and embedded, e.g., VST plugin
        // glCheck(glGetIntegerv(GLEXT_GL_FRAMEBUFFER_BINDING, reinterpret_cast<GLint*>(&m_defaultFrameBuffer)));
        glGetIntegerv(GLEXT_GL_FRAMEBUFFER_BINDING, reinterpret_cast<GLint*>(&m_defaultFrameBuffer));
    }

    // Just initialize the render target part
    RenderTarget::initialize();
}
```

clone the SFML repo and checkout the 3.0.0 branch. Make the abovementioned 
change, build both release and debug versions of system, window, and 
graphics. Then update CMakeLists.txt file:

```cmake
target_include_directories( ${PROJECT_NAME}
  PRIVATE

  # must used the patched version
  C:/Path/to/SFML/include
)

target_link_libraries( ${PROJECT_NAME}
  PRIVATE

  sdk
  Rpcrt4.lib

  # must use the patched version (DEBUG libs -- don't forget RELEASE too)
  C:/Path/to/SFML/cmake-build-debug/lib/sfml-system-s-d.lib
  C:/Path/to/SFML/cmake-build-debug/lib/sfml-window-s-d.lib
  C:/Path/to/SFML/cmake-build-debug/lib/sfml-graphics-s-d.lib
  #    SFML::Window
  #    SFML::Graphics
  ImGui-SFML::ImGui-SFML
  spdlog::spdlog_header_only
  nlohmann_json::nlohmann_json
)
```

## Contributing

Contributions, ideas, and suggestions are welcome! 

## Media

### Reaper v7.28. nxvfxvst v1.0.1.0a. 1 channel, 2 effects
![Alt Text](https://media1.giphy.com/media/v1.Y2lkPTc5MGI3NjExNWd1ZXEwMHl0NDJlczVmYnQ3YzAzY2w3bGU3dDE1M3QwZjloY3JiNSZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/xJvrv2p5RFCdWlpItl/giphy.gif)