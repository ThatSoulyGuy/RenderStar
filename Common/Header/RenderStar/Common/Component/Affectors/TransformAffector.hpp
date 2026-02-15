#pragma once

#include "RenderStar/Common/Component/AbstractAffector.hpp"

namespace RenderStar::Common::Component::Affectors
{
    class TransformAffector final : public AbstractAffector
    {
    public:

        void Affect(ComponentModule& componentModule) override;
    };
}
