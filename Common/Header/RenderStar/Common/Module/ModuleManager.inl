#pragma once

#include "RenderStar/Common/Module/IModule.hpp"
#include "RenderStar/Common/Event/IEventBus.hpp"

namespace RenderStar::Common::Module
{
    template<typename ModuleType>
    ModuleManager::Builder& ModuleManager::Builder::Module(std::unique_ptr<ModuleType> module)
    {
        auto typeIndex = std::type_index(typeid(ModuleType));

        if (modules.contains(typeIndex))
        {
            spdlog::error("Module map already contains '{}'!", typeid(ModuleType).name());
            return *this;
        }

        modules[typeIndex] = std::move(module);
        moduleOrder.push_back(typeIndex);
        return *this;
    }

    template<typename EventBusType>
    ModuleManager::Builder& ModuleManager::Builder::EventBus(std::unique_ptr<EventBusType> eventBus)
    {
        auto typeIndex = std::type_index(typeid(EventBusType));

        if (eventBuses.contains(typeIndex))
        {
            spdlog::error("Event bus map already contains '{}'!", typeid(EventBusType).name());
            return *this;
        }

        eventBuses[typeIndex] = std::move(eventBus);
        return *this;
    }
}
