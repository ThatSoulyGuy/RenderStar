#pragma once

#include "RenderStar/Common/Component/GameObject.hpp"
#include "RenderStar/Common/Module/AbstractModule.hpp"
#include <atomic>
#include <cstdint>

namespace RenderStar::Common::Component
{
    class ComponentModule;
}

namespace RenderStar::Client::Gameplay
{
    class ClientPlayerModule final : public Common::Module::AbstractModule
    {
    public:

        int32_t GetLocalPlayerId() const;

        Common::Component::GameObject GetLocalPlayerEntity() const;

        void CheckForLocalPlayerEntity(Common::Component::ComponentModule& componentModule);

        [[nodiscard]]
        std::vector<std::type_index> GetDependencies() const override;

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        std::atomic<int32_t> localPlayerId{-1};
        Common::Component::GameObject localPlayerEntity = Common::Component::GameObject::Invalid();
        bool localPlayerSetUp = false;
    };
}
