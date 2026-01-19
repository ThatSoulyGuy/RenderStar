#pragma once

namespace RenderStar::Common::Component
{
    class ComponentModule;
}

namespace RenderStar::Common::Component::Systems
{
    class TransformComponentSystem
    {
    public:

        void Run(ComponentModule& componentModule);
    };
}
