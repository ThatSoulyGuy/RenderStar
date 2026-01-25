#pragma once

#include <memory>
#include <optional>
#include <typeindex>
#include <unordered_map>
#include <functional>

namespace RenderStar::Common::Event
{
    class IEventBus;
}

namespace RenderStar::Common::Module
{
    class IModule;

    class ModuleContext
    {
    public:

        using ModuleMap = std::unordered_map<std::type_index, std::unique_ptr<IModule>>;
        using EventBusMap = std::unordered_map<std::type_index, std::unique_ptr<Event::IEventBus>>;

        ModuleContext(ModuleMap& modules, EventBusMap& eventBuses);

        template<typename ModuleType>
        std::optional<std::reference_wrapper<ModuleType>> GetModule();

        template<typename EventBusType>
        std::optional<std::reference_wrapper<EventBusType>> GetEventBus();

        [[nodiscard]]
        int32_t GetTotalModuleCount() const;

    private:

        static int32_t CountModulesRecursive(IModule& module);

        ModuleMap& modules;
        EventBusMap& eventBuses;
    };
}

#include "RenderStar/Common/Module/ModuleContext.inl"
