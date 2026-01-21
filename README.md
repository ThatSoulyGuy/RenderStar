# RenderStar

A modern, modular 3D rendering engine and game framework written in C++23. RenderStar features a clean abstraction layer supporting multiple graphics backends (OpenGL 4.6 and Vulkan), a flexible module system, and client-server architecture for both single-player and multiplayer applications.

## Features

### Multi-Backend Rendering
- **OpenGL 4.6** backend with modern features (UBOs, VAOs, core profile)
- **Vulkan** backend with full module-based architecture (16+ specialized modules)
- **Automatic backend detection** - selects the best available backend at runtime
- **Self-registering backends** via factory pattern for easy extensibility
- **Unified rendering API** - write rendering code once, run on any backend

### Architecture
- **Command Buffer Abstraction** - modern command-based rendering replacing state-based OpenGL patterns
- **Uniform Buffer Objects** - consistent uniform handling across both backends using std140 layout
- **Backend Factory Pattern** - backends self-register with priority-based selection
- **Clean Interface Design** - no backend-specific code in common rendering paths

### Module System
- **ModuleManager** - centralized lifecycle management for all modules
- **ModuleContext** - dependency injection and inter-module communication
- **AbstractModule** - base class with lifecycle hooks (registration, initialization)

### Entity Component System
- **GameObject** - lightweight entity representation
- **ComponentPool** - efficient object pooling for components
- **ComponentView** - type-safe component iteration
- **Built-in Components**: Transform, Hierarchy, Camera, MeshRenderer

### Event System
- **AbstractEventBus** - prioritized event processing
- **Event Priorities** - HIGHEST, HIGH, NORMAL, LOW, LOWEST
- **Handler Priorities** - control subscription execution order

### Asset Management
- **AssetModule** - centralized asset loading and caching
- **AssetLocation** - namespace-based asset paths (e.g., `renderstar:shader/test.vert`)
- **Multiple Loaders** - text assets, binary assets, with extensible loader system

### Networking
- **Client-Server Architecture** - supports dedicated and local server modes
- **PacketBuffer** - binary serialization with VarInt encoding
- **ASIO-based** - asynchronous networking

## Project Structure

```
RenderStar/
├── Client/                      # Client application
│   ├── Header/
│   │   └── RenderStar/Client/
│   │       ├── Core/            # Window, lifecycle management
│   │       ├── Event/           # Client event buses and events
│   │       ├── Network/         # Client networking
│   │       └── Render/          # Rendering subsystem
│   │           ├── Backend/     # IRenderBackend, BackendFactory
│   │           ├── Command/     # Command buffer interfaces
│   │           ├── Components/  # Camera, MeshRenderer
│   │           ├── OpenGL/      # OpenGL backend implementation
│   │           ├── Resource/    # Mesh, Material, Shader interfaces
│   │           ├── Shader/      # GLSL transformation utilities
│   │           ├── Systems/     # Render component systems
│   │           └── Vulkan/      # Vulkan backend (16 modules)
│   └── Source/
├── Common/                      # Shared code between client/server
│   ├── Header/
│   │   └── RenderStar/Common/
│   │       ├── Asset/           # Asset loading and caching
│   │       ├── Component/       # ECS implementation
│   │       ├── Configuration/   # XML/properties configuration
│   │       ├── Event/           # Event bus system
│   │       ├── Module/          # Module management
│   │       ├── Network/         # Packet serialization
│   │       ├── Time/            # Time management
│   │       └── Utility/         # Resource paths, helpers
│   └── Source/
├── Server/                      # Dedicated server
├── Tests/                       # Unit tests (Google Test)
├── CMake/                       # CMake modules and dependencies
└── Build/                       # Build output directories
```

## Dependencies

All dependencies are automatically fetched via CMake FetchContent:

| Library | Version | Purpose |
|---------|---------|---------|
| GLFW | 3.4 | Window creation and input |
| GLAD | 2.0.6 | OpenGL function loading |
| GLM | 1.0.1 | Mathematics (vectors, matrices) |
| spdlog | 1.14.1 | Logging |
| pugixml | 1.14 | XML configuration parsing |
| ASIO | 1.30.2 | Asynchronous networking |
| VulkanMemoryAllocator | 3.1.0 | GPU memory management |
| Google Test | 1.14.0 | Unit testing |
| Vulkan SDK | Required | Vulkan API (install separately) |

## Building

### Prerequisites
- CMake 3.25+
- C++23 compatible compiler (GCC 13+, Clang 16+, MSVC 2022+)
- Vulkan SDK (for Vulkan backend)

### Build Commands

```bash
# Configure
cmake -B Build/Release -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build Build/Release --config Release

# Run client
./Build/Release/Client/RenderStarClient

# Run server
./Build/Release/Server/RenderStarServer
```

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `RENDERSTAR_BUILD_TESTS` | ON | Build unit tests |
| `RENDERSTAR_BUILD_CLIENT` | ON | Build client application |
| `RENDERSTAR_BUILD_SERVER` | ON | Build server application |
| `RENDERSTAR_ENABLE_VALIDATION` | OFF | Enable Vulkan validation layers |
| `RENDERSTAR_ENABLE_ASAN` | OFF | Enable AddressSanitizer |

## Configuration

RenderStar uses XML configuration files. Example `render_star.xml`:

```xml
<configuration>
    <window_title>RenderStar</window_title>
    <window_dimensions>1280,720</window_dimensions>
    <render_backend>VULKAN</render_backend>
    <force_render_backend>false</force_render_backend>
</configuration>
```

## Render Backend Architecture

### Interface Hierarchy

```
IRenderBackend
├── Initialize() / Destroy()
├── BeginFrame() / EndFrame()
├── SubmitDrawCommand() / ExecuteDrawCommands()
├── GetBufferManager()   → IBufferManager
├── GetShaderManager()   → IShaderManager
├── GetUniformManager()  → IUniformManager
└── GetCommandQueue()    → IRenderCommandQueue
```

### Adding a New Backend

1. Create a new backend class implementing `IRenderBackend`
2. Add self-registration in the source file:
```cpp
static bool registered = []() {
    BackendFactory::RegisterBackend(
        RenderBackend::YOUR_BACKEND,
        []() { return std::make_unique<YourRenderBackend>(); },
        CheckYourBackendAvailability,
        priority);
    return true;
}();
```

The factory pattern ensures no modifications to existing code are required.

### Vulkan Modules

The Vulkan backend is highly modular:

| Module | Responsibility |
|--------|---------------|
| VulkanInstanceModule | Instance and debug messenger |
| VulkanDeviceModule | Physical/logical device, queues |
| VulkanSurfaceModule | Window surface |
| VulkanSwapchainModule | Swapchain management |
| VulkanRenderPassModule | Render pass configuration |
| VulkanFramebufferModule | Framebuffer management |
| VulkanCommandModule | Command buffer recording |
| VulkanSyncModule | Semaphores and fences |
| VulkanMemoryModule | VMA-based memory allocation |
| VulkanBufferModule | Buffer creation and management |
| VulkanShaderModule | SPIR-V shader loading |
| VulkanDescriptorModule | Descriptor sets and pools |

## Design Principles

- **Modular Architecture** - independent, loosely-coupled modules
- **Interface Segregation** - small, focused interfaces
- **Factory Pattern** - backends and resources created through factories
- **Command-Based Rendering** - modern GPU-friendly design
- **Zero Backend Switches** - no `switch(backendType)` outside factories

## License

MIT
