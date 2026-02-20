#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"
#include "RenderStar/Common/Scene/MapbinLoader.hpp"
#include <atomic>
#include <mutex>
#include <vector>

namespace RenderStar::Client::Core
{
    class ClientSceneModule final : public Common::Module::AbstractModule
    {
    public:

        [[nodiscard]]
        bool HasPendingSceneData() const;

        std::vector<Common::Scene::MapbinGroup> TakePendingSceneData();

        [[nodiscard]]
        std::vector<std::type_index> GetDependencies() const override;

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        mutable std::mutex sceneMutex;
        std::vector<Common::Scene::MapbinGroup> pendingGroups;
        std::atomic<bool> hasPending = false;
    };
}
