#include "RenderStar/Client/Render/Affectors/AdaptiveVolumeAffector.hpp"
#include "RenderStar/Client/Render/Components/AdaptiveVolume.hpp"
#include "RenderStar/Client/Render/Components/Camera.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/Transform.hpp"
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <vector>

namespace RenderStar::Client::Render::Affectors
{
    using namespace Common::Component;
    using namespace Components;

    void AdaptiveVolumeAffector::Affect(ComponentModule& componentModule)
    {
        resolvedData = Framework::PostProcessData::Defaults();

        auto& volumePool = componentModule.GetPool<AdaptiveVolume>();

        if (volumePool.GetSize() == 0)
            return;

        glm::vec3 cameraPosition(0.0f);

        auto& cameraPool = componentModule.GetPool<Camera>();

        for (auto [entity, camera] : cameraPool)
        {
            auto transformOpt = componentModule.GetComponent<Transform>(entity);

            if (transformOpt.has_value())
            {
                cameraPosition = transformOpt->get().worldPosition;
                break;
            }
        }

        struct ActiveVolume
        {
            int32_t priority;
            float blendFactor;
            uint32_t overrideMask;
            const Framework::PostProcessData* settings;
        };

        std::vector<ActiveVolume> activeVolumes;

        for (auto [entity, volume] : volumePool)
        {
            auto transformOpt = componentModule.GetComponent<Transform>(entity);

            if (!transformOpt.has_value())
                continue;

            auto& transform = transformOpt->get();

            glm::mat4 invWorld = glm::inverse(transform.worldMatrix);
            glm::vec3 localPos = glm::vec3(invWorld * glm::vec4(cameraPosition, 1.0f));

            glm::vec3 absLocal = glm::abs(localPos);

            if (absLocal.x > volume.halfExtents.x ||
                absLocal.y > volume.halfExtents.y ||
                absLocal.z > volume.halfExtents.z)
                continue;

            float dx = volume.halfExtents.x - absLocal.x;
            float dy = volume.halfExtents.y - absLocal.y;
            float dz = volume.halfExtents.z - absLocal.z;
            float minDist = std::min({dx, dy, dz});

            float blend = (volume.blendDistance > 0.0f)
                ? glm::clamp(minDist / volume.blendDistance, 0.0f, 1.0f)
                : 1.0f;

            activeVolumes.push_back({volume.priority, blend, volume.overrideMask, &volume.settings});
        }

        if (activeVolumes.empty())
            return;

        std::sort(activeVolumes.begin(), activeVolumes.end(),
            [](const ActiveVolume& a, const ActiveVolume& b) { return a.priority > b.priority; });

        uint32_t claimedMask = 0;

        for (const auto& av : activeVolumes)
        {
            uint32_t applicableMask = av.overrideMask & ~claimedMask;

            if (applicableMask == 0)
                continue;

            float t = av.blendFactor;

            if (applicableMask & PP_EXPOSURE_BIAS)
                resolvedData.exposureBias = glm::mix(resolvedData.exposureBias, av.settings->exposureBias, t);

            if (applicableMask & PP_BLOOM_INTENSITY)
                resolvedData.bloomIntensity = glm::mix(resolvedData.bloomIntensity, av.settings->bloomIntensity, t);

            if (applicableMask & PP_CONTRAST)
                resolvedData.contrast = glm::mix(resolvedData.contrast, av.settings->contrast, t);

            if (applicableMask & PP_SATURATION)
                resolvedData.saturation = glm::mix(resolvedData.saturation, av.settings->saturation, t);

            if (applicableMask & PP_VIGNETTE)
                resolvedData.vignetteStrength = glm::mix(resolvedData.vignetteStrength, av.settings->vignetteStrength, t);

            if (applicableMask & PP_TEMPERATURE)
                resolvedData.temperature = glm::mix(resolvedData.temperature, av.settings->temperature, t);

            if (applicableMask & PP_FOG)
                resolvedData.fogColor = glm::mix(resolvedData.fogColor, av.settings->fogColor, t);

            if (applicableMask & PP_COLOR_FILTER)
                resolvedData.colorFilter = glm::mix(resolvedData.colorFilter, av.settings->colorFilter, t);

            if (t >= 1.0f)
                claimedMask |= applicableMask;
        }
    }

    const Framework::PostProcessData& AdaptiveVolumeAffector::GetResolvedPostProcessData() const
    {
        return resolvedData;
    }
}
