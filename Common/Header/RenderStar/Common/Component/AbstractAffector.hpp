#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"

namespace RenderStar::Common::Component
{
    class ComponentModule;

    class AbstractAffector : public Module::AbstractModule
    {
    public:

        virtual void Affect(ComponentModule& componentModule) = 0;

    protected:

        void OnInitialize(Module::ModuleContext&) override {}
    };
}
