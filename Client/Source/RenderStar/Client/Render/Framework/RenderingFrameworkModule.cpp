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
        {
            sceneLightingBuffer = bufferManager->CreateUniformBuffer(SceneLightingData::Size());
            postProcessBuffer = bufferManager->CreateUniformBuffer(PostProcessData::Size());

            postProcessData = PostProcessData::Defaults();
            postProcessBuffer->SetSubData(&postProcessData, PostProcessData::Size(), 0);
        }
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

                constexpr float shadowRadius = 30.0f;
                constexpr float shadowNear = 0.1f;
                constexpr float shadowFar = 150.0f;
                constexpr float shadowMapSize = 2048.0f;
                constexpr float texelSize = (shadowRadius * 2.0f) / shadowMapSize;

                glm::vec3 up = std::abs(glm::dot(lightDir, glm::vec3(0.0f, 1.0f, 0.0f))) > 0.99f
                    ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);

                glm::mat4 lightView = glm::lookAt(glm::vec3(0.0f) - lightDir, glm::vec3(0.0f), up);
                glm::vec3 shadowOrigin = glm::vec3(lightView * glm::vec4(cameraPosition, 1.0f));
                shadowOrigin.x = std::floor(shadowOrigin.x / texelSize) * texelSize;
                shadowOrigin.y = std::floor(shadowOrigin.y / texelSize) * texelSize;
                glm::vec3 snappedCenter = glm::vec3(glm::inverse(lightView) * glm::vec4(shadowOrigin, 1.0f));

                lightView = glm::lookAt(snappedCenter - lightDir * (shadowFar * 0.5f), snappedCenter, up);
                glm::mat4 lightProj = glm::ortho(-shadowRadius, shadowRadius, -shadowRadius, shadowRadius, shadowNear, shadowFar);
                sceneLightingData.directionalLightVP = lightProj * lightView;
                constexpr float pcssLightSize = 60.0f;
                sceneLightingData.shadowParams = glm::vec4(1.0f, 0.0008f, shadowMapSize, pcssLightSize);
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

    const SceneLightingData& RenderingFrameworkModule::GetSceneLightingData() const
    {
        return sceneLightingData;
    }

    glm::mat4 RenderingFrameworkModule::GetLightViewProjection() const
    {
        return sceneLightingData.directionalLightVP;
    }

    void RenderingFrameworkModule::UploadPostProcessData(const PostProcessData& data)
    {
        postProcessData = data;

        if (postProcessBuffer)
            postProcessBuffer->SetSubData(&postProcessData, PostProcessData::Size(), 0);
    }

    IBufferHandle* RenderingFrameworkModule::GetPostProcessBuffer() const
    {
        return postProcessBuffer.get();
    }

    const PostProcessData& RenderingFrameworkModule::GetPostProcessData() const
    {
        return postProcessData;
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
        postProcessBuffer.reset();
        postProcessData = PostProcessData::Defaults();
        bufferManager = nullptr;
    }
}
