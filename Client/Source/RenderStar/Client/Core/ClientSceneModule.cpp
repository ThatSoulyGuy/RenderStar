#include "RenderStar/Client/Core/ClientSceneModule.hpp"
#include "RenderStar/Client/Network/ClientNetworkModule.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/MapGeometry.hpp"
#include "RenderStar/Common/Component/Components/PlayerIdentity.hpp"
#include "RenderStar/Common/Component/Components/SerializableTransform.hpp"
#include "RenderStar/Common/Component/EntityAuthority.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Network/PacketModule.hpp"
#include "RenderStar/Common/Network/Packets/AuthorityChangePacket.hpp"
#include "RenderStar/Common/Network/Packets/ComponentUpdatePacket.hpp"
#include "RenderStar/Common/Network/Packets/EntityBatchPacket.hpp"
#include "RenderStar/Common/Network/Packets/EntityCreatePacket.hpp"
#include "RenderStar/Common/Network/Packets/EntityDestroyPacket.hpp"
#include "RenderStar/Common/Scene/SceneModule.hpp"
#include <algorithm>

namespace RenderStar::Client::Core
{
    void ClientSceneModule::OnInitialize(Common::Module::ModuleContext& context)
    {
        sceneModule = &context.GetDependency<Common::Scene::SceneModule>();
        componentModule = &context.GetDependency<Common::Component::ComponentModule>();
        networkModule = &context.GetDependency<Network::ClientNetworkModule>();

        sceneModule->RegisterSerializableComponent<Common::Component::MapGeometry>();
        sceneModule->RegisterSerializableComponent<Common::Component::PlayerIdentity>();
        sceneModule->RegisterSerializableComponent<Common::Component::Transform>();

        auto& packetModule = context.GetDependency<Common::Network::PacketModule>();

        packetModule.RegisterHandler<Common::Network::Packets::EntityBatchPacket>(
            [this](Common::Network::Packets::EntityBatchPacket& packet)
            {
                std::lock_guard lock(pendingMutex);
                pendingBatches.push_back({ packet.batchIndex, packet.totalBatches, std::move(packet.xmlData) });
                hasPending.store(true);
                logger->info("Received entity batch {}/{}", packet.batchIndex + 1, packet.totalBatches);
            });

        packetModule.RegisterHandler<Common::Network::Packets::EntityCreatePacket>(
            [this](Common::Network::Packets::EntityCreatePacket& packet)
            {
                std::lock_guard lock(pendingMutex);
                pendingCreates.push({ std::move(packet.xmlData) });
                hasPending.store(true);
            });

        packetModule.RegisterHandler<Common::Network::Packets::EntityDestroyPacket>(
            [this](Common::Network::Packets::EntityDestroyPacket& packet)
            {
                std::lock_guard lock(pendingMutex);
                pendingDestroys.push({ packet.serverEntityId });
                hasPending.store(true);
            });

        packetModule.RegisterHandler<Common::Network::Packets::ComponentUpdatePacket>(
            [this](Common::Network::Packets::ComponentUpdatePacket& packet)
            {
                std::lock_guard lock(pendingMutex);
                pendingUpdates.push({ packet.entityId, std::move(packet.xmlData) });
                hasPending.store(true);
            });

        packetModule.RegisterHandler<Common::Network::Packets::AuthorityChangePacket>(
            [this](Common::Network::Packets::AuthorityChangePacket& packet)
            {
                std::lock_guard lock(pendingMutex);
                pendingAuthorityChanges.push({ packet.entityId, packet.authorityLevel, packet.ownerId });
                hasPending.store(true);
            });

        logger->info("ClientSceneModule initialized");
    }

    bool ClientSceneModule::HasPendingData() const
    {
        return hasPending.load();
    }

    void ClientSceneModule::ProcessPendingEntityData()
    {
        if (!hasPending.load())
            return;

        ProcessBatches();
        ProcessCreates();
        ProcessDestroys();
        ProcessComponentUpdates();
        ProcessAuthorityChanges();
    }

    void ClientSceneModule::ProcessBatches()
    {
        std::vector<PendingBatch> batches;
        {
            std::lock_guard lock(pendingMutex);
            batches = std::move(pendingBatches);
            pendingBatches.clear();
        }

        if (batches.empty())
            return;

        if (!initialSyncComplete && receivedBatchCount == 0)
        {
            sceneModule->ClearScene();
            remapper = Common::Scene::EntityIdRemapper{};
            expectedTotalBatches = batches[0].totalBatches;
        }

        std::sort(batches.begin(), batches.end(),
            [](const PendingBatch& a, const PendingBatch& b)
            { return a.batchIndex < b.batchIndex; });

        for (auto& batch : batches)
        {
            logger->info("Processing batch {}/{}, xmlData size={}", batch.batchIndex + 1, batch.totalBatches, batch.xmlData.size());
            sceneModule->DeserializeEntities(batch.xmlData, remapper);
            receivedBatchCount++;
        }

        logger->info("After deserialization: {} owned entities", sceneModule->GetOwnedEntityIds().size());

        if (receivedBatchCount >= expectedTotalBatches)
        {
            sceneModule->RemapEntityReferences(remapper);
            initialSyncComplete = true;
            logger->info("Initial entity sync complete: {} batches", receivedBatchCount);
        }

        {
            std::lock_guard lock(pendingMutex);
            if (pendingBatches.empty() && pendingCreates.empty() && pendingDestroys.empty()
                && pendingUpdates.empty() && pendingAuthorityChanges.empty())
                hasPending.store(false);
        }
    }

    void ClientSceneModule::ProcessCreates()
    {
        std::queue<PendingCreate> creates;
        {
            std::lock_guard lock(pendingMutex);
            creates = std::move(pendingCreates);
            pendingCreates = {};
        }

        while (!creates.empty())
        {
            auto& create = creates.front();
            logger->info("Processing entity create, xmlData size={}", create.xmlData.size());
            sceneModule->DeserializeEntities(create.xmlData, remapper);
            sceneModule->RemapEntityReferences(remapper);
            logger->info("After entity create: {} owned entities", sceneModule->GetOwnedEntityIds().size());
            creates.pop();
        }

        {
            std::lock_guard lock(pendingMutex);
            if (pendingBatches.empty() && pendingCreates.empty() && pendingDestroys.empty()
                && pendingUpdates.empty() && pendingAuthorityChanges.empty())
                hasPending.store(false);
        }
    }

    void ClientSceneModule::ProcessDestroys()
    {
        std::queue<PendingDestroy> destroys;
        {
            std::lock_guard lock(pendingMutex);
            destroys = std::move(pendingDestroys);
            pendingDestroys = {};
        }

        while (!destroys.empty())
        {
            auto& destroy = destroys.front();
            auto localEntity = remapper.Remap(destroy.serverEntityId);

            if (localEntity.IsValid())
                sceneModule->DestroyEntity(localEntity);

            destroys.pop();
        }

        {
            std::lock_guard lock(pendingMutex);
            if (pendingBatches.empty() && pendingCreates.empty() && pendingDestroys.empty()
                && pendingUpdates.empty() && pendingAuthorityChanges.empty())
                hasPending.store(false);
        }
    }

    void ClientSceneModule::ProcessComponentUpdates()
    {
        std::queue<PendingComponentUpdate> updates;
        {
            std::lock_guard lock(pendingMutex);
            updates = std::move(pendingUpdates);
            pendingUpdates = {};
        }

        while (!updates.empty())
        {
            auto& update = updates.front();
            auto localEntity = remapper.Remap(update.serverEntityId);

            if (localEntity.IsValid())
                sceneModule->UpdateEntityComponents(localEntity, update.xmlData);

            updates.pop();
        }

        {
            std::lock_guard lock(pendingMutex);
            if (pendingBatches.empty() && pendingCreates.empty() && pendingDestroys.empty()
                && pendingUpdates.empty() && pendingAuthorityChanges.empty())
                hasPending.store(false);
        }
    }

    void ClientSceneModule::ProcessAuthorityChanges()
    {
        std::queue<PendingAuthorityChange> changes;
        {
            std::lock_guard lock(pendingMutex);
            changes = std::move(pendingAuthorityChanges);
            pendingAuthorityChanges = {};
        }

        while (!changes.empty())
        {
            auto& change = changes.front();
            auto localEntity = remapper.Remap(change.serverEntityId);

            if (localEntity.IsValid())
            {
                Common::Component::EntityAuthority authority;
                authority.level = static_cast<Common::Component::AuthorityLevel>(change.authorityLevel);
                authority.ownerId = change.ownerId;
                componentModule->SetEntityAuthority(localEntity, authority);
            }

            changes.pop();
        }

        {
            std::lock_guard lock(pendingMutex);
            if (pendingBatches.empty() && pendingCreates.empty() && pendingDestroys.empty()
                && pendingUpdates.empty() && pendingAuthorityChanges.empty())
                hasPending.store(false);
        }
    }

    void ClientSceneModule::SendDirtyEntityUpdates()
    {
        if (!componentModule || !networkModule || !networkModule->IsConnected())
            return;

        auto dirtyIds = componentModule->ConsumeDirtyEntities();

        for (int32_t localId : dirtyIds)
        {
            Common::Component::GameObject entity{ localId };

            if (componentModule->HasComponent<Common::Component::PlayerIdentity>(entity))
                continue;

            auto authority = componentModule->GetEntityAuthority(entity);

            if (authority.level != Common::Component::AuthorityLevel::CLIENT)
                continue;

            int32_t serverId = remapper.GetServerIdForLocalId(localId);

            if (serverId < 0)
                continue;

            auto xmlData = sceneModule->SerializeEntities({ localId });

            Common::Network::Packets::ComponentUpdatePacket packet;
            packet.entityId = serverId;
            packet.xmlData = std::move(xmlData);

            networkModule->Send(packet);
        }
    }

    Common::Component::GameObject ClientSceneModule::RemapServerEntity(int32_t serverEntityId) const
    {
        return remapper.Remap(serverEntityId);
    }

    std::vector<std::type_index> ClientSceneModule::GetDependencies() const
    {
        return DependsOn<
            Common::Scene::SceneModule,
            Common::Component::ComponentModule,
            Network::ClientNetworkModule,
            Common::Network::PacketModule>();
    }
}
