#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Module/IModule.hpp"

namespace RenderStar::Common::Module
{
    ModuleContext::ModuleContext(ModuleMap& modules, EventBusMap& eventBuses) : modules(modules), eventBuses(eventBuses) { }

    int32_t ModuleContext::GetTotalModuleCount() const
    {
        auto count = static_cast<int32_t>(modules.size());

        for (auto& module : modules | std::views::values)
            count += CountModulesRecursive(*module);

        return count;
    }

    int32_t ModuleContext::CountModulesRecursive(IModule& module)
    {
        int32_t count = 0;

        for (auto& subModule : module.GetSubModules())
        {
            count++;
            count += CountModulesRecursive(*subModule);
        }

        return count;
    }
}
