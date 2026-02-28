#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"
#include "RenderStar/Common/Component/GameObject.hpp"
#include "RenderStar/Server/Network/ClientConnection.hpp"
#include <memory>

namespace RenderStar::Common::Scene
{
    class SceneModule;
}

namespace RenderStar::Server::Network
{
    class ServerNetworkModule;
}

namespace RenderStar::Server::Core
{
    class ServerSceneModule final : public Common::Module::AbstractModule
    {
    public:

        static constexpr int32_t DEFAULT_BATCH_SIZE = 50;

        Common::Component::GameObject CreateAndBroadcastEntity(const std::string& name);
        void DestroyAndBroadcastEntity(Common::Component::GameObject entity);

        [[nodiscard]]
        std::vector<std::type_index> GetDependencies() const override;

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        void OnClientJoined(std::shared_ptr<Network::ClientConnection> connection);
        void SendEntityBatches(Network::ClientConnection& connection);

        Network::ServerNetworkModule* networkModule = nullptr;
        Common::Scene::SceneModule* sceneModule = nullptr;
    };
}
