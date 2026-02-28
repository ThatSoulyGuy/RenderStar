#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"

namespace RenderStar::Server::Core
{
    class ServerLifecycleModule final : public Common::Module::AbstractModule
    {
    public:

        [[nodiscard]]
        std::vector<std::type_index> GetDependencies() const override;

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;
    };
}
