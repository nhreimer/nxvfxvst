# nxvfx VST3

## License

This software is licensed under the GNU AGPLv3 for non-commercial use only.  
See [LICENSE](./LICENSE) for full terms and third-party dependency notices.

## Description

A real-time, modular, multithreaded rendering VST3 Video Effects Engine Plugin for audio and midi events. 

## Goal

Synchronize midi events and audio data to highly customizable visuals.

## Features

### Right-click to hide/show menus!

* Ability to run in Standalone and VST3 Plugin
  * In Standalone (testing only, no audio)
    * Midi Generator that pushes events on different threads to
      simulate DAW Processor threads and stress testing
* Multichannel support
  * Route midi output to independent VFX chains
  * Infinite shader and modifier chaining per channel
  * Ability to prioritize rendering order for each channel
* Each effect can be assigned user-specified midi notes for triggers
* DAW Automation controls for effects
  * Dynamically names and resets names of parameters for DAW visibility
* JSON serialization for importing/exporting (via clipboard at the moment)
* Real-time VFX Engine that synchronizes to audio data and midi events
  * Effects
  * Particle generator
  * Particle modifiers
  * Easings (for time decays)
  * Triggers at multiple stages of a pipeline (for time synchronization)
* Real-time video encoder (Raw RGBA with Frame Rate Locking)
* Multithreaded rendering
  * Each channel renders on its own thread

---

# BUILDING

## Dependencies

* C++20
* [VST3 SDK](https://www.steinberg.net/vst3sdk)
* [SFML v3](https://www.sfml-dev.org/)
* [ImGui](https://github.com/ocornut/imgui)
* [ImGui-SFML](https://github.com/SFML/imgui-sfml)
* [nlohmann json](https://github.com/nlohmann/json)
* [spdlog](https://github.com/gabime/spdlog)
* [Moody Camel](https://github.com/cameron314/concurrentqueue)
* [KissFFT](https://github.com/mborgerding/kissfft)

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

Set BUILD_PLUGIN to OFF or FALSE. 

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

### VST3 Plugin and DEBUG Build

If you want to build the VST plugin version AND in DEBUG mode, then there's a problem in
SFML's RenderWindow.cpp that MUST be fixed or SFML will crash.
glCheck() is the culprit.

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

Clone the SFML repo and checkout the 3.0.0 branch. Make the abovementioned
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

  # must use the patched version (DEBUG libs)
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
---

# Architecture

```text
+---------------------+
|   nxvfx VST Plugin  |
|         0..n        |  Supports multiple plugin instantiations
+---------------------+


+---------------------+
|  EventFacadeVst     |
|         1           |  IVSTPlugin implementation forwards events here
+---------------------+
          │
          ▼
+------------------------+
|  MultichannelPipeline  |
|         1              |  Manages 1 audio channel and n midi channels
+------------------------+
          │
          ▼
+---------------------+
|  ChannelPipeline    |
|        1..n         |  Each channel is independent
+---------------------+        +--------------------+
          │                    | Particle Layout    |
          │                    |         1          |
          │                --> +--------------------+
          ▼               |    
+---------------------+   |    +--------------------+        +--------------------+
|                     | --     | Particle Generator |        |   Particle Type    |
|  ParticlePipeline   | -----> |         1          | -----> |          1         |
|         1           | --     +--------------------+        +--------------------+
+---------------------+   |    
          │               |    +--------------------+
          ▼                --> | Behavior Pipeline  |
+---------------------+        |       0..n         |
|  ModifierPipeline   |        +--------------------+
|       0..n          |
+---------------------+
          │
          ▼
+--------------------+      +--------------------+
|  ShaderPipeline    | ---> |      Easings       |
|       0..n         |      |         1          |
+--------------------+      +--------------------+
          │
          ▼
+------------------+
|   RenderWindow   |  All the graphical output from each channel output here
+------------------+
          │
          ▼
+------------------+
|   Video Encoder  |  Optional video encoder that locks to specified FPS
+------------------+


```

## VST3 Information and Limitations

### VST3 Refresh Rate (Windows)

__TL;DR__:

Windows Desktop Window Manager (DWM) cannot be bypassed (safely), and so the framerate will often 
be limited to approximately 60 FPS.

__DETAILS__:

Why VST3 Plugins Can't Escape DWM Chains:

    We're inside the DAW's process.
    We render into a child HWND.
    The DAW host owns the main thread, DPI context, compositor control, and event loop.
    We don’t get to create exclusive full-screen swap chains, custom WGL contexts, or bypass HWND parenting.

Even if you tried, most DAWs would reject the plugin (host rule: no global popups, no top-level windows, no full-screen hijacking).

Games and full-screen apps manage to bypass this by:

    Telling Windows to detach from DWM and hand the GPU fully to the app
    They create their own top-level HWND and control the swap chain directly

By default, the plugin will attempt to use 120 FPS, but it will match the reported refresh rate.
The logs will then report any frames that are dropped according to the matched refresh rate. If
the framerate drops by 10% then the Frame Diagnostics will appear in red.

### VST3 Design Overview: The Controller and the Processor

The VST3 API is composed of two components: the controller and the processor. 
Each one runs on a different thread and communication is handled through a message-passing 
interface.

The controller handles the user interface side (window creation, user controls, parameters)
while the processor handles signal processing (operates on audio and midi data).

Do __NOT__ create static variables unless you want all instantiations of the plugin to be identical.

### Communication pattern between the two

__TL;DR__:

Consumer-Producer pattern used for communication between the two components.

__DETAILS__:

For the purposes of this plugin, communication flows only from the Processor to the Controller, 
because we only need to inform the UI of a midi event or a BPM change.

Thread switching does not occur on a message event, i.e., the processor thread runs inside the 
controller side whenever the controller receives a message. 

so we utilize a simple producer-consumer pattern 
for immediately pushing events from the Processor thread into a concurrent queue in the controller, 
and then we allow the Controller thread to consume events on every frame.

### VST3 Parameters

__TL;DR__:

Parameters are limited to a number of slots on initialization. 
Parameter names in DAW show as "Param_N" until a control takes over and renames it. 
Parameters are limited to 0 - 1, but the real value is still provided to the DAW. 

Dynamic assignment of parameters are ephemeral by nature, so the order you add and 
remove modifiers, behaviors, or effects impacts the parameter assignment. 

__DETAILS__:

VST3 requires all parameters, i.e, user controls, to be defined during instantiation. The plugin, 
however, creates dynamic effects and each effect has a number of controls. There's no reliable way
(that I can tell) to change the VST parameter type or parameter name after creation. As a result, 
the plugin registers a block of parameters during plugin instantiation, which is why you'll see 
"Param_0" to "Param_255" appear as parameters. Those parameters get assigned dynamically whenever 
an effect is created, and the name will be updated.

Additionally, all parameters are of the same Ranged Type, where the values are normalized between 
0 and 1 because we cannot change the parameter type. The name and the "real" value of the parameter 
will be updated whenever using automation.

# NXVFX Components

## EventFacadeVST

Created by the Win32View, which is an implementation of the VST View interface by the VST Controller

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

### Particle Layout

Manages the particle layout (initial placement & creation) and passes particles to the ModifierPipeline.

    Owns IParticleLayout (e.g., SpiralLayout, RandomBurst)
    Maintains a container of TimedParticle_t objects
    Delegates visual transformation to modifiers
    Sends particle data to the modifier pipeline via std::deque<TimedParticle_t*>

| Layout Type     | Description                                    |
|-----------------|------------------------------------------------|
| Empty           | no particles are drawn at all                  |
| Random          | randomly generates positions                   |
| Spiral          | spirals from the center out based on midi note |
| Golden Spiral   | uses the golden ratio                          | 
| Lissajous Curve |                                                |
| L-System Curve  |                                                |
| Fractal Ring    |                                                |
| Elliptical      |                                                |

### Particle Generator

Generates particles 

| Particle Type | Description |
|---------------|-------------|
| Circles       |             |
| Rings         |             |

### Particle Behavior Pipeline

Manages behaviors that change particles directly (useful for making adjustments on a single particle basis),
e.g., Jitter, Gravity, Spread. Cannot be used for adding or removing particles.

    Alters particles on a per-particle basis
    Does not own any memory

| Behavior Name | Description |
|---------------|-------------|
| Color Morph   |             |
| Free Fall     |             |
| Jitter        |             |
| Magnetic      |             |
| Radial Spread |             |

## Modifier Pipeline

Processes a stack of IParticleModifier objects sequentially. It modifies the particles or adds objects to draw.

    Operates on std::deque<TimedParticle_t*> from the IParticleLayout implementation
    Applies transformations (e.g., line connections, Perlin deformation) 
    Those transformations can directly alter the Particles in a deque or add std::deque<sf::Drawable*>
    Renders output to an internal sf::RenderTexture that gets handed off to the shader pipeline 

| Modifier Type        | Description        |
|----------------------|--------------------|
| Full Mesh Lines      |                    |
| Sequential Lines     |                    |
| Ring Zone Mesh Lines |                    |
| Perlin Deformer      |                    |
| Mirror               | CPU-side mirroring |

## ShaderPipeline

Applies post-processing shaders to the result of the modifier stack.

    Accepts the RenderTexture from the ModifierPipeline
    Applies one or more chained fragment shaders (e.g., glow, blur, ripple, glitch)
    Applies many different types of easings
    Produces the final visual output for the sf::RenderWindow

| Shader Type      | Description |
|------------------|-------------|
| Gaussian Blur    |             |
| Color            |             |
| Cosmic-Kaleido   |             |
| Dual-Kawase Blur |             |
| Density Heat Map |             |
| Feedback         |             |
| Glitch           |             |
| Pulse            |             |
| Ripple           |             |
| Rumble           |             |
| Shock Bloom      |             |
| Smear            |             |
| Strobe           |             |
| Transform        |             |

In the code base, there's an additional shader called "BlenderShader" which is 
added to almost all shaders to provide mixing between the original input and the output.

## Video Encoder

At the moment, there is only one video encoder:

1. RawRGBA, which saves an image of every frame (MASSIVE file sizes). it will produce two json meta files.

You can use ffmpeg on the command line to convert the RawRGBA file, e.g.,

```bash
  ffmpeg.exe -f rawvideo -pix_fmt rgba -s 1280x768 -r 60 -i video_in.rbga -c:v libx264 -pix_fmt yuv420p video_out.mp4
````

Be sure to look at the JSON metadata file for the actual width and height.

## Pipeline Context

A pipeline context is passed down through every single component:

```c++
    const GlobalInfo_t& globalInfo; // read-only info related to window, view, and time
    VSTStateContext& vstContext; // writable info related to VST Parameters 
```

## Serialization

MultichannelPipeline supports full JSON serialization:

    - Global State
        - VST Parameter binding 
    - Channel Serialization
        - Layout
            - Behaviors
        - Modifiers
        - Shaders
            - Easing

This enables saving and restoring complex visual setups per channel.

## Easing Functions

Easing functions shape how visual effects change over time — they control intensity, scale, 
opacity, distortion, or any other effect parameter in a way that feels natural, expressive, 
or rhythmic. Instead of effects snapping on or fading linearly, easings let us inject emotion, 
timing, and energy into each animation.

Each easing function takes a normalized time value t in the range [0.0, 1.0] and returns a value that modulates the strength or visibility of an effect at that point in time.

At the moment, the easings are either built into the Shader code or assigned manually to certain shader controls. 

### Use Cases:

    Controlling glow or blur intensity over time
    Adding bounce or flicker to ripple pulses
    Modulating brightness synced to beat pulses
    Fading particles in or out with style

Available Easing Types
```text 
Name	          Description
-----------------------------------------------------------------------------------------------
Disabled          Uses 1.f, i.e., no change
Fixed             Uses a user-assigned value that doesn't change over time
Time Continuous   Uses elapsed time without resetting
Time Intervallic  Uses elapsed time with resetting
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

---

# Contributing

Contributions, ideas, bug reports, and suggestions are welcome!

## Feature Roadmap

* Save custom load to/from files instead of copy/paste
* Add default presets
* Add OS support for Linux and Mac
* Better UI design
  * Better layout
  * More information/intuitive control naming
  * Controls work directly with BPM
* Rehydrate dynamically assigned parameters

# Media

### Reaper v7.28. nxvfxvst v1.0.1.0 pre-alpha. 1 channel, 2 effects
![Alt Text](https://media1.giphy.com/media/v1.Y2lkPTc5MGI3NjExNWd1ZXEwMHl0NDJlczVmYnQ3YzAzY2w3bGU3dDE1M3QwZjloY3JiNSZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/xJvrv2p5RFCdWlpItl/giphy.gif)

### Reaper v7.28 nxvfxst v1.0.8.0 pre-alpha. 1 channel, 4 effects
![Alt Text](https://media0.giphy.com/media/v1.Y2lkPTc5MGI3NjExaTA1NWt5YW15bjh6bnU5bmN4a3VuMjZtdmt0NDA0MzF6bms3MGk0dyZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/Os4AJXRxO7AV5TygMO/giphy.gif)

