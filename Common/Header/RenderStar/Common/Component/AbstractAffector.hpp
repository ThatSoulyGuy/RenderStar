#pragma once

#include "RenderStar/Common/Component/AuthorityContext.hpp"
#include "RenderStar/Common/Module/AbstractModule.hpp"

namespace RenderStar::Common::Component
{
    class ComponentModule;

    class AbstractAffector : public Module::AbstractModule
    {
    public:

        virtual void Affect(ComponentModule& componentModule) = 0;

        void SetAuthorityContext(AuthorityContext context) { authorityContext = context; }

        [[nodiscard]]
        const AuthorityContext& GetAuthorityContext() const { return authorityContext; }

    protected:

        void OnInitialize(Module::ModuleContext&) override {}

        AuthorityContext authorityContext = AuthorityContext::AsNobody();
    };
}
