#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"
#include "RenderStar/Common/Component/GameObject.hpp"
#include "RenderStar/Common/Scene/EntityIdRemapper.hpp"
#include <atomic>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

namespace RenderStar::Common::Scene { class SceneModule; }
namespace RenderStar::Common::Component { class ComponentModule; }
namespace RenderStar::Client::Network { class ClientNetworkModule; }

namespace RenderStar::Client::Core
{
    class ClientSceneModule final : public Common::Module::AbstractModule
    {
    public:

        [[nodiscard]]
        bool HasPendingData() const;

        void ProcessPendingEntityData();

        void SendDirtyEntityUpdates();

        [[nodiscard]]
        Common::Component::GameObject RemapServerEntity(int32_t serverEntityId) const;

        [[nodiscard]]
        std::vector<std::type_index> GetDependencies() const override;

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        struct PendingBatch
        {
            int32_t batchIndex;
            int32_t totalBatches;
            std::string xmlData;
        };

        struct PendingCreate
        {
            std::string xmlData;
        };

        struct PendingDestroy
        {
            int32_t serverEntityId;
        };

        struct PendingComponentUpdate
        {
            int32_t serverEntityId;
            std::string xmlData;
        };

        struct PendingAuthorityChange
        {
            int32_t serverEntityId;
            uint8_t authorityLevel;
            int32_t ownerId;
        };

        void ProcessBatches();
        void ProcessCreates();
        void ProcessDestroys();
        void ProcessComponentUpdates();
        void ProcessAuthorityChanges();

        Common::Scene::SceneModule* sceneModule = nullptr;
        Common::Component::ComponentModule* componentModule = nullptr;
        Network::ClientNetworkModule* networkModule = nullptr;

        Common::Scene::EntityIdRemapper remapper;

        mutable std::mutex pendingMutex;
        std::vector<PendingBatch> pendingBatches;
        std::queue<PendingCreate> pendingCreates;
        std::queue<PendingDestroy> pendingDestroys;
        std::queue<PendingComponentUpdate> pendingUpdates;
        std::queue<PendingAuthorityChange> pendingAuthorityChanges;
        std::atomic<bool> hasPending{false};

        int32_t expectedTotalBatches = 0;
        int32_t receivedBatchCount = 0;
        bool initialSyncComplete = false;
    };
}
