#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"

namespace RenderStar::Server::Core
{
    class ServerLifecycleModule : public Common::Module::AbstractModule
    {
    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;
    };
}
