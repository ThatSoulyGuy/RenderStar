#pragma once

#include <cstdint>

namespace RenderStar::Common::Component
{
    class ComponentModule;
}

namespace RenderStar::Client::Render::Systems
{
    class CameraComponentSystem
    {
    public:

        CameraComponentSystem();

        void SetViewportSize(int32_t width, int32_t height);

        void Run(Common::Component::ComponentModule& componentModule);

    private:

        int32_t viewportWidth;
        int32_t viewportHeight;
    };
}
