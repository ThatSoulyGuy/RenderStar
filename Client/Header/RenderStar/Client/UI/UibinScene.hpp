#pragma once

#include "RenderStar/Client/UI/UIElement.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace RenderStar::Client::UI
{
    struct UibinScene
    {
        UIElement root;
        std::unordered_map<std::string, std::vector<uint8_t>> assets;
        double designWidth = 0.0;
        double designHeight = 0.0;

        UIElement* FindById(const std::string& id)
        {
            return root.FindById(id);
        }

        UIElement* FindByName(const std::string& name)
        {
            return root.FindByName(name);
        }
    };
}
