#pragma once

#include "RenderStar/Common/Module/IModule.hpp"
#include <spdlog/spdlog.h>
#include <vector>
#include <typeindex>

namespace RenderStar::Common::Module
{
    class AbstractModule : public IModule
    {
    public:

        AbstractModule();

        ~AbstractModule() override = default;

        void OnRegistration(ModuleContext& context) final;

        void SetParent(IModule* parent) override;

        [[nodiscard]]
        IModule* GetParent() const override;

        std::span<std::unique_ptr<IModule>> GetSubModules() override;

        [[nodiscard]]
        std::type_index GetTypeIndex() const override;

        template<typename SubModuleType>
        void RegisterSubModule(std::unique_ptr<SubModuleType> subModule);

        template<typename SubModuleType>
        std::optional<std::reference_wrapper<SubModuleType>> GetSubModule();

    protected:

        virtual void OnInitialize(ModuleContext& context) = 0;

        std::shared_ptr<spdlog::logger> logger;
        ModuleContext* context;

        IModule* parent;
        bool initialized;

        std::vector<std::unique_ptr<IModule>> subModules;
    };
}

#include "RenderStar/Common/Module/AbstractModule.inl"
