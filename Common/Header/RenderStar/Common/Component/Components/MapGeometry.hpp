#pragma once

#include "RenderStar/Common/Scene/SerializableComponent.hpp"
#include <pugixml.hpp>
#include <string>

namespace RenderStar::Common::Component
{
    struct MapGeometry
    {
        std::string assetPath;
    };
}

template<>
struct RenderStar::Common::Scene::SerializableComponent<RenderStar::Common::Component::MapGeometry> : std::true_type
{
    static constexpr const char* XmlTag = "MapGeometry";

    static void Write(const RenderStar::Common::Component::MapGeometry& component, pugi::xml_node& node)
    {
        node.append_attribute("assetPath").set_value(component.assetPath.c_str());
    }

    static RenderStar::Common::Component::MapGeometry Read(const pugi::xml_node& node)
    {
        RenderStar::Common::Component::MapGeometry result;
        result.assetPath = node.attribute("assetPath").as_string("");
        return result;
    }
};
