#pragma once

#include "RenderStar/Common/Component/AbstractAffector.hpp"

namespace RenderStar::Client::Input
{
    class ClientInputModule;
}

namespace RenderStar::Common::Time
{
    class TimeModule;
}

namespace RenderStar::Client::Gameplay
{
    class PlayerControllerAffector final : public Common::Component::AbstractAffector
    {
    public:

        void Affect(Common::Component::ComponentModule& componentModule) override;

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        Input::ClientInputModule* inputModule = nullptr;
        Common::Time::TimeModule* timeModule = nullptr;
    };
}
