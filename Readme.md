# nxvfx VST3

## Description

A highly configurable and modular Video Effects Engine for midi events. It VST3 that comes with a standalone test playground application. 

## Features

* Ability to run in Standalone and VST3 Plugin
  * In Standalone (testing only, no audio)
    * Midi Generator that pushes events on different threads in order to
      simulate DAW Processor threads and stress testing
* Multichannel support
  * Route midi output to independent VFX chains
  * Infinite shader and modifier chaining per channel
* Each effect can be assigned user-specified midi notes for triggers
* JSON serialization for importing/exporting (via clipboard at the moment)
* Real-time VFX Engine that synchronize to midi events
  * Effects
  * Particle generator
  * Particle modifiers
  * Easings (for time decays)
  * Triggers at multiple stages of a pipeline (for time synchronization)

## Dependencies

* C++20
* SFML v3 (graphics: small patch required)
* ImGui (menus)
* nlohmann json (serialization and state management)
* spdlog (logging)
* VST3 SDK

The VST3 SDK can be downloaded from here https://www.steinberg.net/vst3sdk

## Feature Roadmap

* Save and load to/from files instead of copy/paste
* Add parameters for VST3 for automation
* Add OS support for Linux and Mac
* Better UI design
  * Better layout
  * More information/intuitive control naming
  * Controls work directly with BPM
* Export video output directly

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

# Design

```text
+---------------------+
|  EventFacadeVST     |
+---------------------+
       │ │ │ │          <---  multichannel provider 
       ▼ ▼ ▼ ▼
+---------------------+
|  ChannelPipeline    |
+---------------------+
        │
        ▼
+---------------------+      +--------------------+
|  ParticlePipeline   | ---> |  Behavior Pipeline |  
+---------------------+      +--------------------+
        │
        ▼
+---------------------+
|  ModifierPipeline   |
+---------------------+
        │
        ▼
+--------------------+      +--------------------+
|  ShaderPipeline    | ---> |      Easings       +
+--------------------+      +--------------------+
        │
        ▼
+------------------+
|   RenderWindow   |
+------------------+
```

🔁 = manages multiple instances of a type, e.g., ShaderPipeline manages multiple shaders

## EventFacadeVST

Created by the Win32View, which is an implementation of the VST View interface
    
    Handles the event frame, i.e., everything required in a UI loop
    Forwards events from the VST Controller
    Manages ImGui and SFML high-level coordination 

## Channel Pipeline

The ChannelPipeline is the top-level coordinator for a single MIDI channel. It handles:

    Receiving MIDI events
    Updating particle logic
    Running the full modifier and shader pipelines
    Rendering the final composited result to the screen

## Particle Pipeline

Manages the particle layout (initial placement & creation) and passes particles to the ModifierPipeline.

    Owns IParticleLayout (e.g., SpiralLayout, RandomBurst)
    Maintains a container of TimedParticle_t objects
    Delegates visual transformation to modifiers
    Sends particle data to the modifier pipeline via std::deque<TimedParticle_t*>

## Particle Behavior Pipeline

Manages behaviors that change particles directly (useful for making adjustments on a single particle basis),
e.g., Jitter, Gravity, Spread. Cannot be used for adding or removing particles.

    Alters particles on a per-particle basis
    Does not own any memory

## Modifier Pipeline

Processes a stack of IParticleModifier objects sequentially. It modifies the particles or adds objects to draw.

    Operates on std::deque<TimedParticle_t*> from the IParticleLayout implementation
    Applies transformations (e.g., line connections, Perlin deformation) 
    Those transformations can directly alter the Particles in a deque or add std::deque<sf::Drawable*>
    Renders output to an internal sf::RenderTexture that gets handed off to the shader pipeline 

## ShaderPipeline

Applies post-processing shaders to the result of the modifier stack.

    Accepts the RenderTexture from the ModifierPipeline
    Applies one or more chained fragment shaders (e.g., glow, blur, ripple, glitch)
    Applies many different types of easings
    Produces the final visual output for the sf::RenderWindow

## GlobalInfo

A shared read-only context passed throughout all components that help:

```c++
struct GlobalInfo_t {
  sf::Vector2u windowSize;
  sf::View windowView;
  sf::Vector2f windowHalfSize;
  bool hideMenu = false;
  double bpm = 0.0;
  float elapsedTimeSeconds = 0.0f;
  std::uint64_t frameCount = 0;
};
```

Used for:

    Time-based animation
    Rhythm-driven effects (via BPM)
    Window information

## Serialization

Each ChannelPipeline supports full JSON serialization:

    Layout configuration
    Modifier stack
    Shader chain
    UI state

This enables saving and restoring complex visual setups per channel.

## Easing Functions

Easing functions shape how visual effects change over time — they control intensity, scale, opacity, distortion, or any other effect parameter in a way that feels natural, expressive, or rhythmic. Instead of effects snapping on or fading linearly, easings let us inject emotion, timing, and energy into each animation.

Each easing function takes a normalized time value t in the range [0.0, 1.0] and returns a value that modulates the strength or visibility of an effect at that point in time.

### Use Cases:

    Controlling glow or blur intensity over time
    Adding bounce or flicker to ripple pulses
    Modulating brightness synced to beat pulses
    Fading particles in or out with style

Available Easing Types
```text 
Name	          Description
-----------------------------------------------------------------------------------------------
Linear	          Constant rate of change. No curve — useful for mechanical or unstyled fades.
Quadratic	  Accelerates or decelerates quickly. Good for soft ease-ins/outs.
Cubic	          More dramatic curve than quadratic. Smoother transitions with more tension.
Quartic	          Very sharp ease-in or ease-out. Great for dramatic builds or drops.
Sine	          Uses a sine wave for smooth natural motion — feels organic.
Expo	          Starts slow, then rapidly ramps up. Perfect for builds and bursts.
Bounce	          Simulates physical bouncing — springy and fun for reactive animations.
Back	          Overshoots slightly then settles — mimics recoil or elastic movement.
Impulse	          Sudden burst with a rapid decay. Ideal for one-shot flashes.
Pulse Sine	  Oscillating sine wave. Creates rhythmic pulsing synced to time or BPM.
Pulse Ping	  Soft ping-pong effect — ramps up and back down smoothly.
Sparkle Flicker	  High-frequency noise-like flicker — adds chaotic shimmer or sparkle.
Smooth Pulse	  Curved rise and fall — good for glow or aura-like pulsing.
```

Additionally, there is a Cumulative Easing, that can combine multiple easings, but it's currently only applied to the LayeredGlitch Effect.