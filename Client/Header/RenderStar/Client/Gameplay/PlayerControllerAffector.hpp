#pragma once

#include "RenderStar/Common/Component/AbstractAffector.hpp"
#include <cstdint>

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
    struct FrameInput
    {
        uint8_t flags = 0;
        float yaw = 0.0f;
        float pitch = 0.0f;
        float deltaTime = 0.0f;
        bool hasInput = false;
    };

    class PlayerControllerAffector final : public Common::Component::AbstractAffector
    {
    public:

        void Affect(Common::Component::ComponentModule& componentModule) override;

        const FrameInput& GetLastFrameInput() const { return lastFrameInput; }

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        Input::ClientInputModule* inputModule = nullptr;
        Common::Time::TimeModule* timeModule = nullptr;
        bool authorityWarningLogged = false;
        FrameInput lastFrameInput;
    };
}
