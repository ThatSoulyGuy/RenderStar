#include "RenderStar/Client/Render/Framework/RenderingFrameworkModule.hpp"
#include "RenderStar/Client/Render/Components/Light.hpp"
#include "RenderStar/Client/Render/Resource/IBufferManager.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/Transform.hpp"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace RenderStar::Client::Render::Framework
{
    void RenderingFrameworkModule::OnInitialize(Common::Module::ModuleContext&)
    {
    }

    void RenderingFrameworkModule::SetupRenderState(IBufferManager* bm)
    {
        bufferManager = bm;

        if (bufferManager)
            sceneLightingBuffer = bufferManager->CreateUniformBuffer(SceneLightingData::Size());
    }

    void RenderingFrameworkModule::CollectSceneData(Common::Component::ComponentModule& componentModule,
                                                     const glm::vec3& cameraPosition)
    {
        sceneLightingData = SceneLightingData{};
        sceneLightingData.cameraPosition = glm::vec4(cameraPosition, 1.0f);
        sceneLightingData.ambientColor = glm::vec4(ambientColor, ambientIntensity);

        auto& lightPool = componentModule.GetPool<Components::Light>();
        bool directionalFound = false;
        int32_t pointCount = 0;
        int32_t spotCount = 0;

        for (auto [entity, light] : lightPool)
        {
            if (light.type == Components::LightType::DIRECTIONAL && !directionalFound)
            {
                directionalFound = true;
                glm::vec3 lightDir = glm::normalize(light.direction);
                sceneLightingData.directionalDirection = glm::vec4(lightDir, 0.0f);
                sceneLightingData.directionalColor = glm::vec4(light.color, light.intensity);

                glm::mat4 lightView = glm::lookAt(-lightDir * 50.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                glm::mat4 lightProj = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, 0.1f, 100.0f);
                sceneLightingData.directionalLightVP = lightProj * lightView;
                sceneLightingData.shadowParams = glm::vec4(1.0f, 0.005f, 2048.0f, 0.0f);
            }
            else if (light.type == Components::LightType::POINT && pointCount < MAX_POINT_LIGHTS)
            {
                auto transformOpt = componentModule.GetComponent<Common::Component::Transform>(entity);

                if (!transformOpt.has_value())
                    continue;

                auto& transform = transformOpt->get();

                sceneLightingData.pointLights[pointCount].positionAndRange =
                    glm::vec4(transform.worldPosition, light.range);
                sceneLightingData.pointLights[pointCount].colorAndIntensity =
                    glm::vec4(light.color, light.intensity);
                pointCount++;
            }
            else if (light.type == Components::LightType::SPOT && spotCount < MAX_SPOT_LIGHTS)
            {
                auto transformOpt = componentModule.GetComponent<Common::Component::Transform>(entity);

                if (!transformOpt.has_value())
                    continue;

                auto& transform = transformOpt->get();

                sceneLightingData.spotLights[spotCount].positionAndRange =
                    glm::vec4(transform.worldPosition, light.range);
                sceneLightingData.spotLights[spotCount].directionAndAngle =
                    glm::vec4(glm::normalize(light.direction), std::cos(glm::radians(light.spotAngle)));
                sceneLightingData.spotLights[spotCount].colorAndIntensity =
                    glm::vec4(light.color, light.intensity);

                float innerAngle = light.spotAngle * (1.0f - light.spotSoftness);
                sceneLightingData.spotLights[spotCount].spotParams =
                    glm::vec4(std::cos(glm::radians(innerAngle)), 0.0f, 0.0f, 0.0f);

                spotCount++;
            }
        }

        sceneLightingData.pointLightCount = pointCount;
        sceneLightingData.spotLightCount = spotCount;

        if (sceneLightingBuffer)
            sceneLightingBuffer->SetSubData(&sceneLightingData, SceneLightingData::Size(), 0);
    }

    IBufferHandle* RenderingFrameworkModule::GetSceneLightingBuffer() const
    {
        return sceneLightingBuffer.get();
    }

    glm::mat4 RenderingFrameworkModule::GetLightViewProjection() const
    {
        return sceneLightingData.directionalLightVP;
    }

    void RenderingFrameworkModule::SetAmbientLight(glm::vec3 color, float intensity)
    {
        ambientColor = color;
        ambientIntensity = intensity;
    }

    void RenderingFrameworkModule::Cleanup()
    {
        sceneLightingBuffer.reset();
        sceneLightingData = SceneLightingData{};
        bufferManager = nullptr;
    }
}
