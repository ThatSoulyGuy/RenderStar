#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"
#include "RenderStar/Server/Network/ClientConnection.hpp"
#include <memory>

namespace RenderStar::Server::Network
{
    class ServerNetworkModule;
}

namespace RenderStar::Common::Network
{
    class PacketModule;
}

namespace RenderStar::Server::Core
{
    class ServerSceneModule final : public Common::Module::AbstractModule
    {
    public:

        [[nodiscard]]
        std::vector<std::type_index> GetDependencies() const override;

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        void OnClientJoined(std::shared_ptr<Network::ClientConnection> connection);

        Network::ServerNetworkModule* networkModule = nullptr;
    };
}
