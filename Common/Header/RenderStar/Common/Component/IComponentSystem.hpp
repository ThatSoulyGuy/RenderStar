#pragma once

#include <functional>

namespace RenderStar::Common::Module
{
    class ModuleContext;
}

namespace RenderStar::Common::Component
{
    class ComponentModule;

    class IComponentSystem
    {
    public:

        virtual ~IComponentSystem() = default;

        virtual void Run(Module::ModuleContext& context, ComponentModule& componentModule) = 0;
    };

    using ComponentSystemFunction = std::function<void(Module::ModuleContext&, ComponentModule&)>;
}
