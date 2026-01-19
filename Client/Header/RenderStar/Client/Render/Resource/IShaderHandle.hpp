#pragma once

#include <cstdint>

namespace RenderStar::Client::Render
{
    class IShaderHandle
    {
    public:

        virtual ~IShaderHandle() = default;

        virtual uint64_t GetId() const = 0;
        virtual void Destroy() = 0;
        virtual bool IsValid() const = 0;
    };
}
