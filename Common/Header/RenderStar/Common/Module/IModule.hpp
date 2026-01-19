#pragma once

#include <memory>
#include <span>
#include <typeindex>

namespace RenderStar::Common::Module
{
    class ModuleContext;

    class IModule
    {
    public:

        virtual ~IModule() = default;

        virtual void OnRegistration(ModuleContext& context) = 0;

        virtual void SetParent(IModule* parent) = 0;

        virtual IModule* GetParent() const = 0;

        virtual std::span<std::unique_ptr<IModule>> GetSubModules() = 0;

        virtual std::type_index GetTypeIndex() const = 0;
    };
}
