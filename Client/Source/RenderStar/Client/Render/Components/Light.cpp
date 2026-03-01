#include "RenderStar/Client/Render/Components/Light.hpp"

namespace RenderStar::Client::Render::Components
{
    Light Light::Directional(glm::vec3 direction, glm::vec3 color, float intensity)
    {
        Light light;
        light.type = LightType::DIRECTIONAL;
        light.direction = direction;
        light.color = color;
        light.intensity = intensity;
        return light;
    }

    Light Light::Point(glm::vec3 color, float intensity, float range)
    {
        Light light;
        light.type = LightType::POINT;
        light.color = color;
        light.intensity = intensity;
        light.range = range;
        return light;
    }

    Light Light::Spot(glm::vec3 direction, glm::vec3 color, float intensity, float range, float angle)
    {
        Light light;
        light.type = LightType::SPOT;
        light.direction = direction;
        light.color = color;
        light.intensity = intensity;
        light.range = range;
        light.spotAngle = angle;
        return light;
    }
}
