# nxvfx (Nick's VFX)

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
* SFML v3 (graphics: small patch required)
* ImGui (menus)
* nlohmann json (serialization and state saving)
* spdlog (logging)
* VST3 SDK

The VST3 SDK can be downloaded from here https://www.steinberg.net/vst3sdk

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

### Roadmap

* Save and load to/from files instead of copy/paste
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