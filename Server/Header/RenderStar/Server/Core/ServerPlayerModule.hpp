#pragma once

#include "RenderStar/Common/Component/GameObject.hpp"
#include "RenderStar/Common/Module/AbstractModule.hpp"
#include "RenderStar/Server/Network/ClientConnection.hpp"
#include <cstdint>
#include <memory>
#include <unordered_map>

namespace RenderStar::Common::Component
{
    class ComponentModule;
}

namespace RenderStar::Common::Network
{
    class IPacket;
}

namespace RenderStar::Common::Scene
{
    class SceneModule;
}

namespace RenderStar::Server::Network
{
    class ServerNetworkModule;
}

namespace RenderStar::Server::Physics
{
    class ServerPhysicsModule;
}

namespace RenderStar::Server::Core
{
    class ServerSceneModule;

    struct PlayerState
    {
        std::shared_ptr<Network::ClientConnection> connection;
        Common::Component::GameObject entity = Common::Component::GameObject::Invalid();
    };

    class ServerPlayerModule final : public Common::Module::AbstractModule
    {
    public:

        [[nodiscard]]
        std::vector<std::type_index> GetDependencies() const override;

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        void OnClientJoined(std::shared_ptr<Network::ClientConnection> connection);
        void OnClientLeft(std::shared_ptr<Network::ClientConnection> connection);
        void OnPacketReceived(std::shared_ptr<Network::ClientConnection> connection, Common::Network::IPacket& packet);

        int32_t FindPlayerIdByConnection(const Network::ClientConnection& connection) const;

        Network::ServerNetworkModule* networkModule = nullptr;
        Common::Scene::SceneModule* sceneModule = nullptr;
        Common::Component::ComponentModule* componentModule = nullptr;
        ServerSceneModule* serverSceneModule = nullptr;
        Physics::ServerPhysicsModule* serverPhysicsModule = nullptr;

        int32_t nextPlayerId = 0;
        std::unordered_map<int32_t, PlayerState> players;
    };
}
