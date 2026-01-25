#pragma once

#include <ranges>
#include "RenderStar/Common/Module/IModule.hpp"

namespace RenderStar::Common::Module
{
    template <typename ModuleType>
    std::optional<std::reference_wrapper<ModuleType>> ModuleContext::GetModule()
    {
        const auto typeIndex = std::type_index(typeid(ModuleType));

        if (const auto iterator = modules.find(typeIndex); iterator != modules.end())
            return std::ref(static_cast<ModuleType&>(*iterator->second));

        for (const auto& module : modules | std::views::values)
        {
            for (auto& subModule : module->GetSubModules())
            {
                if (subModule->GetTypeIndex() == typeIndex)
                    return std::ref(static_cast<ModuleType&>(*subModule));
            }
        }

        return std::nullopt;
    }

    template <typename EventBusType>
    std::optional<std::reference_wrapper<EventBusType>> ModuleContext::GetEventBus()
    {
        const auto typeIndex = std::type_index(typeid(EventBusType));

        if (const auto iterator = eventBuses.find(typeIndex); iterator != eventBuses.end())
            return std::ref(static_cast<EventBusType&>(*iterator->second));

        return std::nullopt;
    }
}
