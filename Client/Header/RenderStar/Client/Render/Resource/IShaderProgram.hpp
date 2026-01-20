#pragma once

namespace RenderStar::Client::Render
{
    class IShaderProgram
    {
    public:

        virtual ~IShaderProgram() = default;

        virtual bool IsValid() const = 0;
    };
}
