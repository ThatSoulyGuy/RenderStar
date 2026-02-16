#pragma once

#include <memory>
#include <span>
#include <typeindex>
#include <vector>

namespace RenderStar::Common::Module
{
    class ModuleContext;

    class IModule
    {
    public:

        virtual ~IModule() = default;

        virtual void OnRegistration(ModuleContext& context) = 0;

        virtual void OnPrepareShutdown() = 0;

        virtual void OnShutdown() = 0;

        virtual void SetParent(IModule* parent) = 0;

        [[nodiscard]]
        virtual IModule* GetParent() const = 0;

        virtual std::span<std::unique_ptr<IModule>> GetSubModules() = 0;

        [[nodiscard]]
        virtual std::type_index GetTypeIndex() const = 0;

        [[nodiscard]]
        virtual std::vector<std::type_index> GetDependencies() const = 0;
    };
}
