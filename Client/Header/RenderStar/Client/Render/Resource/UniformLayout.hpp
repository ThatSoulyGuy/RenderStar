#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace RenderStar::Client::Render
{
    enum class UniformType
    {
        UNIFORM_BUFFER,
        STORAGE_BUFFER,
        COMBINED_IMAGE_SAMPLER,
        STORAGE_IMAGE
    };

    enum class ShaderStage
    {
        VERTEX,
        FRAGMENT,
        VERTEX_FRAGMENT,
        COMPUTE,
        ALL
    };

    struct UniformBinding
    {
        uint32_t binding;
        std::string name;
        UniformType type;
        size_t size;
        ShaderStage stages;
    };

    struct UniformLayout
    {
        std::vector<UniformBinding> bindings;

        static UniformLayout ForMVP()
        {
            return UniformLayout{
                {
                    UniformBinding{
                        0,
                        "UniformBufferObject",
                        UniformType::UNIFORM_BUFFER,
                        3 * 16 * sizeof(float),
                        ShaderStage::VERTEX
                    }
                }
            };
        }

        static UniformLayout Empty()
        {
            return UniformLayout{ {} };
        }

        int32_t GetBindingCount() const
        {
            return static_cast<int32_t>(bindings.size());
        }

        const UniformBinding& GetBinding(int32_t index) const
        {
            return bindings[index];
        }
    };
}
