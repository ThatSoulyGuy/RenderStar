#pragma once

namespace RenderStar::Common::Module
{
    template <typename SubModuleType>
    void AbstractModule::RegisterSubModule(std::unique_ptr<SubModuleType> subModule)
    {
        subModule->SetParent(this);
        IModule* raw = subModule.get();
        subModules.push_back(std::move(subModule));

        if (initialized && context != nullptr)
            raw->OnRegistration(*context);
    }

    template <typename SubModuleType>
    std::optional<std::reference_wrapper<SubModuleType>> AbstractModule::GetSubModule()
    {
        auto typeIndex = std::type_index(typeid(SubModuleType));

        for (auto& subModule : subModules)
        {
            if (subModule->GetTypeIndex() == typeIndex)
                return std::ref(static_cast<SubModuleType&>(*subModule));
        }

        return std::nullopt;
    }

    template<typename... Deps>
    std::vector<std::type_index> AbstractModule::DependsOn()
    {
        return { std::type_index(typeid(Deps))... };
    }
}
