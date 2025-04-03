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
* SFML v3 (graphics)
* ImGui (menus)
* nlohmann json (serialization)
* spdlog (logging)
* VST3 SDK (if running as a plugin)

### Roadmap

* Save and load from files
* Multichannel