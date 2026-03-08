#include "RenderStar/Client/Render/Affectors/RemotePlayerInterpolationAffector.hpp"
#include "RenderStar/Client/Render/Components/PhysicsBodyHandle.hpp"
#include "RenderStar/Client/Render/Components/RemotePlayerState.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/Transform.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Physics/MovementModel.hpp"
#include "RenderStar/Common/Physics/PhysicsModule.hpp"
#include "RenderStar/Common/Time/TimeModule.hpp"

namespace RenderStar::Client::Render::Affectors
{
    using namespace Common::Component;

    void RemotePlayerInterpolationAffector::OnInitialize(Common::Module::ModuleContext& context)
    {
        if (auto time = context.GetModule<Common::Time::TimeModule>(); time.has_value())
            timeModule = &time->get();

        if (auto physics = context.GetModule<Common::Physics::PhysicsModule>(); physics.has_value())
            physicsModule = &physics->get();
    }

    void RemotePlayerInterpolationAffector::Affect(ComponentModule& componentModule)
    {
        if (!timeModule)
            return;

        double localTime = timeModule->GetElapsedTime();

        auto& pool = componentModule.GetPool<Components::RemotePlayerState>();

        if (pool.GetSize() == 0)
            return;

        for (auto [entity, remoteState] : pool)
        {
            if (remoteState.snapshots.size() < 2)
            {
                if (!remoteState.snapshots.empty())
                {
                    auto transformOpt = componentModule.GetComponent<Transform>(entity);

                    if (transformOpt.has_value())
                    {
                        transformOpt->get().position = remoteState.snapshots.back().position;
                        componentModule.MarkEntityDirty(entity);
                    }
                }

                continue;
            }

            // Track latest server time and the local time when we received it
            double newestTime = remoteState.snapshots.back().serverTime;

            if (newestTime > latestServerTime)
            {
                latestServerTime = newestTime;
                localTimeAtLatestServer = localTime;
            }

            // Advance render time smoothly using local time between server snapshots
            double timeSinceLastSnapshot = localTime - localTimeAtLatestServer;
            double renderTime = latestServerTime + timeSinceLastSnapshot - INTERPOLATION_DELAY;

            // Find two bracketing snapshots
            const Components::RemotePlayerState::Snapshot* from = nullptr;
            const Components::RemotePlayerState::Snapshot* to = nullptr;

            for (size_t i = 0; i + 1 < remoteState.snapshots.size(); ++i)
            {
                if (remoteState.snapshots[i].serverTime <= renderTime &&
                    remoteState.snapshots[i + 1].serverTime >= renderTime)
                {
                    from = &remoteState.snapshots[i];
                    to = &remoteState.snapshots[i + 1];
                    break;
                }
            }

            glm::vec3 interpolatedPos;

            if (from && to)
            {
                double duration = to->serverTime - from->serverTime;
                float t = (duration > 0.0001) ? static_cast<float>((renderTime - from->serverTime) / duration) : 1.0f;
                t = glm::clamp(t, 0.0f, 1.0f);
                interpolatedPos = glm::mix(from->position, to->position, t);
            }
            else
            {
                interpolatedPos = remoteState.snapshots.back().position;
            }

            auto transformOpt = componentModule.GetComponent<Transform>(entity);

            if (transformOpt.has_value())
            {
                transformOpt->get().position = interpolatedPos;
                componentModule.MarkEntityDirty(entity);
            }

            // Sync kinematic collision body for remote player
            if (physicsModule)
            {
                auto physicsOpt = componentModule.GetComponent<Components::PhysicsBodyHandle>(entity);

                if (physicsOpt.has_value() && physicsOpt->get().body)
                {
                    float totalHeight = Common::Physics::PlayerDimensions::TOTAL_HEIGHT;
                    glm::vec3 centerPos(interpolatedPos.x, interpolatedPos.y + totalHeight * 0.5f, interpolatedPos.z);
                    physicsModule->SyncKinematicBody(physicsOpt->get().body, centerPos);
                }
            }
        }
    }
}
