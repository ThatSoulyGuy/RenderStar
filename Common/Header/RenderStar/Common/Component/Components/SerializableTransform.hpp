#pragma once

#include "RenderStar/Common/Component/Components/Transform.hpp"
#include "RenderStar/Common/Scene/SerializableComponent.hpp"
#include <pugixml.hpp>

template<>
struct RenderStar::Common::Scene::SerializableComponent<RenderStar::Common::Component::Transform> : std::true_type
{
    static constexpr const char* XmlTag = "Transform";

    static void Write(const RenderStar::Common::Component::Transform& t, pugi::xml_node& node)
    {
        node.append_attribute("px").set_value(t.position.x);
        node.append_attribute("py").set_value(t.position.y);
        node.append_attribute("pz").set_value(t.position.z);
        node.append_attribute("rx").set_value(t.rotation.x);
        node.append_attribute("ry").set_value(t.rotation.y);
        node.append_attribute("rz").set_value(t.rotation.z);
        node.append_attribute("rw").set_value(t.rotation.w);
        node.append_attribute("sx").set_value(t.scale.x);
        node.append_attribute("sy").set_value(t.scale.y);
        node.append_attribute("sz").set_value(t.scale.z);
    }

    static RenderStar::Common::Component::Transform Read(const pugi::xml_node& node)
    {
        RenderStar::Common::Component::Transform t;
        t.position.x = node.attribute("px").as_float();
        t.position.y = node.attribute("py").as_float();
        t.position.z = node.attribute("pz").as_float();
        t.rotation.x = node.attribute("rx").as_float(0.0f);
        t.rotation.y = node.attribute("ry").as_float(0.0f);
        t.rotation.z = node.attribute("rz").as_float(0.0f);
        t.rotation.w = node.attribute("rw").as_float(1.0f);
        t.scale.x = node.attribute("sx").as_float(1.0f);
        t.scale.y = node.attribute("sy").as_float(1.0f);
        t.scale.z = node.attribute("sz").as_float(1.0f);
        return t;
    }
};
