#pragma once

#include "RenderStar/Common/Component/AbstractAffector.hpp"
#include <cstdint>

namespace RenderStar::Client::Render::Affectors
{
    class CameraAffector final : public Common::Component::AbstractAffector
    {
    public:

        void SetViewportSize(int32_t width, int32_t height);

        void Affect(Common::Component::ComponentModule& componentModule) override;

    private:

        int32_t viewportWidth = 1280;
        int32_t viewportHeight = 720;
    };
}
