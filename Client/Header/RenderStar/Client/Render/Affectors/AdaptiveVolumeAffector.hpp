#pragma once

#include "RenderStar/Client/Render/Framework/PostProcessData.hpp"
#include "RenderStar/Common/Component/AbstractAffector.hpp"

namespace RenderStar::Client::Render::Affectors
{
    class AdaptiveVolumeAffector final : public Common::Component::AbstractAffector
    {
    public:

        void Affect(Common::Component::ComponentModule& componentModule) override;

        const Framework::PostProcessData& GetResolvedPostProcessData() const;

    private:

        Framework::PostProcessData resolvedData = Framework::PostProcessData::Defaults();
    };
}
