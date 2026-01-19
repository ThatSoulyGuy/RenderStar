#pragma once

#include "RenderStar/Common/Module/IModule.hpp"
#include "RenderStar/Common/Event/IEventBus.hpp"

namespace RenderStar::Common::Module
{
    template<typename ModuleType>
    std::optional<std::reference_wrapper<ModuleType>> ModuleContext::GetModule()
    {
        auto typeIndex = std::type_index(typeid(ModuleType));
        auto iterator = modules.find(typeIndex);

        if (iterator != modules.end())
            return std::ref(static_cast<ModuleType&>(*iterator->second));

        for (auto& [key, module] : modules)
        {
            for (auto& subModule : module->GetSubModules())
            {
                if (subModule->GetTypeIndex() == typeIndex)
                    return std::ref(static_cast<ModuleType&>(*subModule));
            }
        }

        return std::nullopt;
    }

    template<typename EventBusType>
    std::optional<std::reference_wrapper<EventBusType>> ModuleContext::GetEventBus()
    {
        auto typeIndex = std::type_index(typeid(EventBusType));
        auto iterator = eventBuses.find(typeIndex);

        if (iterator != eventBuses.end())
            return std::ref(static_cast<EventBusType&>(*iterator->second));

        return std::nullopt;
    }
}
