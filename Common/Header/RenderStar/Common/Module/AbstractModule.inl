#pragma once

namespace RenderStar::Common::Module
{
    template <typename SubModuleType>
    void AbstractModule::RegisterSubModule(std::unique_ptr<SubModuleType> subModule)
    {
        subModule->SetParent(this);
        subModules.push_back(std::move(subModule));
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
}
