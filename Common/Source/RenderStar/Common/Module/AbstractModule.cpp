#include "RenderStar/Common/Module/AbstractModule.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"

namespace RenderStar::Common::Module
{
    AbstractModule::AbstractModule()
        : logger(spdlog::default_logger())
        , context(nullptr)
        , parent(nullptr)
        , initialized(false)
    {
    }

    void AbstractModule::OnRegistration(ModuleContext& moduleContext)
    {
        if (initialized)
            return;

        context = &moduleContext;
        OnInitialize(moduleContext);
        initialized = true;

        for (auto& subModule : subModules)
            subModule->OnRegistration(moduleContext);
    }

    void AbstractModule::SetParent(IModule* parentModule)
    {
        parent = parentModule;
    }

    IModule* AbstractModule::GetParent() const
    {
        return parent;
    }

    std::span<std::unique_ptr<IModule>> AbstractModule::GetSubModules()
    {
        return subModules;
    }

    std::type_index AbstractModule::GetTypeIndex() const
    {
        return std::type_index(typeid(*this));
    }
}
