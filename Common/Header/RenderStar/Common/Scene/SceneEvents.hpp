#pragma once

#include "RenderStar/Common/Scene/SceneDescriptor.hpp"
#include "RenderStar/Common/Event/IEvent.hpp"
#include <string>
#include <string_view>

namespace RenderStar::Common::Scene
{
    struct ScenePreLoadEvent final : Event::TypedEvent<ScenePreLoadEvent>
    {
        std::string filePath;

        ScenePreLoadEvent() = default;

        explicit ScenePreLoadEvent(std::string path) : filePath(std::move(path)) { }

        [[nodiscard]]
        std::string_view GetName() const override { return "ScenePreLoadEvent"; }
    };

    struct ScenePostLoadEvent final : Event::TypedEvent<ScenePostLoadEvent>
    {
        SceneDescriptor descriptor;
        bool success;

        ScenePostLoadEvent() : success(false) { }

        ScenePostLoadEvent(SceneDescriptor desc, const bool result) : descriptor(std::move(desc)), success(result) { }

        [[nodiscard]]
        std::string_view GetName() const override { return "ScenePostLoadEvent"; }
    };

    struct ScenePreSaveEvent final : Event::TypedEvent<ScenePreSaveEvent>
    {
        std::string filePath;

        ScenePreSaveEvent() = default;

        explicit ScenePreSaveEvent(std::string path) : filePath(std::move(path)) { }

        [[nodiscard]]
        std::string_view GetName() const override { return "ScenePreSaveEvent"; }
    };

    struct ScenePostSaveEvent final : Event::TypedEvent<ScenePostSaveEvent>
    {
        std::string filePath;
        bool success;

        ScenePostSaveEvent() : success(false) { }

        ScenePostSaveEvent(std::string path, const bool result) : filePath(std::move(path)), success(result) { }

        [[nodiscard]]
        std::string_view GetName() const override { return "ScenePostSaveEvent"; }
    };

    struct SceneClearedEvent final : Event::TypedEvent<SceneClearedEvent>
    {
        [[nodiscard]]
        std::string_view GetName() const override { return "SceneClearedEvent"; }
    };
}
