#pragma once

#include <cstdint>

namespace RenderStar::Client::Render
{
    struct BackendCapabilities
    {
        bool supportsCompute;
        bool supportsMultiDrawIndirect;
        bool supportsBindlessTextures;
        int32_t maxUniformBufferSize;
        int32_t maxTextureSize;
        int32_t maxFramesInFlight;

        static BackendCapabilities ForVulkan()
        {
            return BackendCapabilities{
                .supportsCompute = true,
                .supportsMultiDrawIndirect = true,
                .supportsBindlessTextures = true,
                .maxUniformBufferSize = 65536,
                .maxTextureSize = 16384,
                .maxFramesInFlight = 2
            };
        }

        static BackendCapabilities ForOpenGL33()
        {
            return BackendCapabilities{
                .supportsCompute = false,
                .supportsMultiDrawIndirect = false,
                .supportsBindlessTextures = false,
                .maxUniformBufferSize = 16384,
                .maxTextureSize = 16384,
                .maxFramesInFlight = 2
            };
        }

        static BackendCapabilities ForOpenGL45()
        {
            return BackendCapabilities{
                .supportsCompute = true,
                .supportsMultiDrawIndirect = true,
                .supportsBindlessTextures = true,
                .maxUniformBufferSize = 65536,
                .maxTextureSize = 16384,
                .maxFramesInFlight = 2
            };
        }
    };
}
