# Zenyth Engine

A DirectX 12 toy rendering engine written in C++20, designed for learning advanced real-time rendering techniques and serving as a research platform for hybrid precision rendering.

## Purpose

Zenyth is primarily an educational project focused on exploring modern GPU-driven rendering techniques. The engine serves as a testbed for implementing and experimenting with:

- **Deferred Rendering Pipeline**
- **Screen-Space Ambient Occlusion (SSAO)**
- **Anti-Aliasing Techniques** (MSAA, TAA, FXAA)
- **Hybrid Precision Rendering** - Research into mixed floating-point precision for performance optimization
- **Modern DirectX 12 Features** - Command queues, descriptor heaps, and GPU-driven rendering

The included Minicraft demo showcases a voxel-based renderer that demonstrates the engine's capabilities for handling large-scale geometry with multiple rendering passes.

## Features

### Rendering Core
- **Modern DirectX 12 Backend**: GPU-driven rendering with explicit resource management
- **Multi-Pass Rendering**: Deferred shading with geometry and lighting passes
- **Advanced Buffer Management**: Custom upload allocators and command batching system
- **Shader Reflection**: Automatic pipeline generation from HLSL shaders using DXC
- **Multiple Render Targets**: Support for G-buffer rendering and post-processing

### Development Tools
- **ImGui Integration**: Real-time debugging and parameter tweaking
- **Hot Shader Reloading**: Rapid iteration on rendering techniques

[//]: # (- **Performance Profiling**: GPU timing and resource usage monitoring)

### Sample Implementation
- **Voxel Renderer**: Chunk-based world with frustum culling
- **Texture Atlasing**: Efficient batch rendering of textured geometry
- **Multiple Shader Passes**: Opaque, transparent, and water rendering

## Current Research Focus

### Hybrid Precision Rendering
Investigating the use of mixed floating-point precision (FP32/FP16) in the rendering pipeline to:
- Reduce memory bandwidth requirements
- Improve performance on modern GPUs
- Maintain visual quality through selective precision usage

[//]: # (### Advanced Lighting Techniques)

[//]: # (- Implementation of physically-based deferred lighting)

[//]: # (- Screen-space reflection and ambient occlusion)

[//]: # (- Volumetric lighting and fog effects)

## Building

### Prerequisites
- **CMake 3.29+**
- **Visual Studio 2022** with C++20 support
- **Windows 10/11** with DirectX 12 compatible GPU
- **Windows Game SDK** (Windows 10 SDK version 10.0.19041.0 or later)
- **Git** with submodule support

### Build Instructions
```bash
git clone --recursive https://github.com/TNtube/Zenyth
cd Zenyth

# If you forgot --recursive, initialize submodules:
# git submodule update --init --recursive

mkdir build && cd build
cmake ..
cmake --build . --config Release
```

Dependencies are managed through:
- **Git Submodules**: DirectX Headers, DirectXTK12, ImGui
- **Auto-download**: DirectX Shader Compiler (DXC) via CMake
- **System**: Windows Game SDK (must be installed separately)

## Running the Demo

```bash
./Sandbox.exe
```

**Controls:**
- `ZQSD` - Camera movement
- `E/A` - Vertical movement
- `Mouse` - Look around (hold left/right button)
- `Shift` - Speed modifier

## Research Applications

This engine will be used to research on the domain of hybrid precision rendering:

- **Memory-Efficient Rendering**: Custom allocators reducing GPU memory fragmentation
- **Precision Analysis**: Tools for measuring numerical accuracy in rendering pipelines

## Limitations

As a research and learning project, Zenyth focuses on rendering techniques rather than being a full game engine:
- No audio system
- Minimal physics
- Limited asset pipeline
- Windows/DirectX 12 only

## Academic Use

If you use Zenyth in academic research, please consider citing this repository and any relevant papers that emerge from the hybrid precision rendering research.