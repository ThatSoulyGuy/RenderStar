#pragma once

#include "RenderStar/Common/Component/AbstractAffector.hpp"

namespace RenderStar::Common::Time
{
    class TimeModule;
}

namespace RenderStar::Common::Physics
{
    class PhysicsModule;
}

namespace RenderStar::Client::Render::Affectors
{
    class RemotePlayerInterpolationAffector final : public Common::Component::AbstractAffector
    {
    public:

        void Affect(Common::Component::ComponentModule& componentModule) override;

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        Common::Time::TimeModule* timeModule = nullptr;
        Common::Physics::PhysicsModule* physicsModule = nullptr;
        double latestServerTime = 0.0;
        double localTimeAtLatestServer = 0.0;
        static constexpr double INTERPOLATION_DELAY = 0.1;  // 100ms
    };
}
