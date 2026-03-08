#pragma once

#include "RenderStar/Common/Component/AbstractAffector.hpp"

namespace RenderStar::Common::Physics
{
    class PhysicsModule;
}

namespace RenderStar::Common::Time
{
    class TimeModule;
}

namespace RenderStar::Client::Render::Affectors
{
    class PlayerPhysicsAffector final : public Common::Component::AbstractAffector
    {
    public:

        void Affect(Common::Component::ComponentModule& componentModule) override;

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        Common::Physics::PhysicsModule* physicsModule = nullptr;
        Common::Time::TimeModule* timeModule = nullptr;

        bool diagnosticLogged = false;
        int32_t diagFrameCount = 0;
    };
}
