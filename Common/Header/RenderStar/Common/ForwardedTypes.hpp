#pragma once

#include <cstdint>

namespace RenderStar::Common::Module
{
    class IModule;
    class AbstractModule;
    class ModuleManager;
    class ModuleContext;
}

namespace RenderStar::Common::Event
{
    class IEvent;
    class IEventBus;
    class AbstractEventBus;
    struct EventResult;
    enum class EventPriority : int32_t;
    enum class HandlerPriority : int32_t;
}

namespace RenderStar::Common::Component
{
    struct GameObject;
    class IComponentPool;

    template <typename ComponentType>
    class ComponentPool;

    template <typename... ComponentTypes>
    class ComponentView;

    class ComponentModule;
}

namespace RenderStar::Common::Configuration
{
    class IConfiguration;
    class Configuration;
}

namespace RenderStar::Common::Network
{
    class IPacket;
    class PacketBuffer;
    class PacketModule;
}

namespace RenderStar::Common::Time
{
    class TimeModule;
}
