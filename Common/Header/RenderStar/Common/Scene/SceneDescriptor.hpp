#pragma once

#include <filesystem>
#include <string>

namespace RenderStar::Common::Scene
{
    struct SceneDescriptor
    {
        std::string name;
        std::string version{ "1.0" };
        std::string author;
        std::string description;
        std::string mapGeometryFile;

        std::filesystem::path filePath;
    };
}
