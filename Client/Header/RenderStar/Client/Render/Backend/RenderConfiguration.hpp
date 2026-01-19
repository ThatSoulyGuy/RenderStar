#pragma once

#include "RenderStar/Client/Render/Backend/RenderBackend.hpp"
#include <cstdint>
#include <optional>

namespace RenderStar::Client::Render
{
    struct RenderConfiguration
    {
        std::optional<RenderBackend> preferredBackend;
        bool forceBackend;
        int32_t msaaSamples;
        bool vsync;

        static RenderConfiguration Defaults()
        {
            return RenderConfiguration{
                .preferredBackend = std::nullopt,
                .forceBackend = false,
                .msaaSamples = 1,
                .vsync = true
            };
        }

        static RenderConfiguration Vulkan()
        {
            return RenderConfiguration{
                .preferredBackend = RenderBackend::VULKAN,
                .forceBackend = false,
                .msaaSamples = 1,
                .vsync = true
            };
        }

        static RenderConfiguration OpenGL()
        {
            return RenderConfiguration{
                .preferredBackend = RenderBackend::OPENGL,
                .forceBackend = false,
                .msaaSamples = 1,
                .vsync = true
            };
        }

        RenderConfiguration WithPreferredBackend(RenderBackend backend) const
        {
            RenderConfiguration copy = *this;
            copy.preferredBackend = backend;
            return copy;
        }

        RenderConfiguration WithForceBackend(bool force) const
        {
            RenderConfiguration copy = *this;
            copy.forceBackend = force;
            return copy;
        }

        RenderConfiguration WithMsaaSamples(int32_t samples) const
        {
            RenderConfiguration copy = *this;
            copy.msaaSamples = samples;
            return copy;
        }

        RenderConfiguration WithVsync(bool vsyncValue) const
        {
            RenderConfiguration copy = *this;
            copy.vsync = vsyncValue;
            return copy;
        }
    };
}
