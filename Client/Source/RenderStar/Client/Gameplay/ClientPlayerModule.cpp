#include "RenderStar/Client/Gameplay/ClientPlayerModule.hpp"
#include "RenderStar/Client/Gameplay/PlayerController.hpp"
#include "RenderStar/Client/Render/Components/Camera.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/PlayerIdentity.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Network/PacketModule.hpp"
#include "RenderStar/Common/Network/Packets/PlayerAssignPacket.hpp"

namespace RenderStar::Client::Gameplay
{
    void ClientPlayerModule::OnInitialize(Common::Module::ModuleContext& context)
    {
        auto& packetModule = context.GetDependency<Common::Network::PacketModule>();

        packetModule.RegisterHandler<Common::Network::Packets::PlayerAssignPacket>(
            [this](Common::Network::Packets::PlayerAssignPacket& packet)
            {
                localPlayerId.store(packet.playerId);
                logger->info("Assigned player ID: {}", packet.playerId);
            });

        logger->info("ClientPlayerModule initialized");
    }

    int32_t ClientPlayerModule::GetLocalPlayerId() const
    {
        return localPlayerId.load();
    }

    Common::Component::GameObject ClientPlayerModule::GetLocalPlayerEntity() const
    {
        return localPlayerEntity;
    }

    void ClientPlayerModule::CheckForLocalPlayerEntity(Common::Component::ComponentModule& componentModule)
    {
        if (localPlayerSetUp)
            return;

        int32_t pid = localPlayerId.load();

        if (pid < 0)
            return;

        auto& pool = componentModule.GetPool<Common::Component::PlayerIdentity>();

        if (pool.GetSize() == 0)
        {
            logger->debug("Waiting for player entity: PlayerIdentity pool is empty (localPlayerId={})", pid);
            return;
        }

        for (auto [entity, identity] : pool)
        {
            if (identity.playerId != pid)
                continue;

            localPlayerEntity = entity;

            auto& controller = componentModule.AddComponent<PlayerController>(entity);
            controller.moveSpeed = 5.0f;
            controller.lookSensitivity = 0.1f;

            auto& camera = componentModule.AddComponent<Render::Components::Camera>(entity);
            camera.projectionType = Render::Components::ProjectionType::PERSPECTIVE;
            camera.fieldOfView = 60.0f;
            camera.nearPlane = 0.1f;
            camera.farPlane = 100.0f;

            auto authority = componentModule.GetEntityAuthority(entity);
            logger->info("Local player entity set up: id={}, playerId={}, authority={}, ownerId={}", entity.id, pid, static_cast<int>(authority.level), authority.ownerId);

            localPlayerSetUp = true;
            break;
        }

        if (!localPlayerSetUp)
            logger->debug("PlayerIdentity pool has {} entries but none match localPlayerId={}", pool.GetSize(), pid);
    }

    std::vector<std::type_index> ClientPlayerModule::GetDependencies() const
    {
        return DependsOn<Common::Network::PacketModule>();
    }
}
