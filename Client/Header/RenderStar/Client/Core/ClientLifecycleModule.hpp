#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"

namespace RenderStar::Client::Core
{
    class ClientLifecycleModule final : public Common::Module::AbstractModule
    {
    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        void SetupMainLoop();
    };
}
