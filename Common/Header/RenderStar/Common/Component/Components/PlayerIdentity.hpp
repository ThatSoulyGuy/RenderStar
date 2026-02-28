#pragma once

#include "RenderStar/Common/Scene/SerializableComponent.hpp"
#include <pugixml.hpp>
#include <cstdint>

namespace RenderStar::Common::Component
{
    struct PlayerIdentity
    {
        int32_t playerId = -1;
    };
}

template<>
struct RenderStar::Common::Scene::SerializableComponent<RenderStar::Common::Component::PlayerIdentity> : std::true_type
{
    static constexpr const char* XmlTag = "PlayerIdentity";

    static void Write(const RenderStar::Common::Component::PlayerIdentity& component, pugi::xml_node& node)
    {
        node.append_attribute("playerId").set_value(component.playerId);
    }

    static RenderStar::Common::Component::PlayerIdentity Read(const pugi::xml_node& node)
    {
        RenderStar::Common::Component::PlayerIdentity result;
        result.playerId = node.attribute("playerId").as_int(-1);
        return result;
    }
};
