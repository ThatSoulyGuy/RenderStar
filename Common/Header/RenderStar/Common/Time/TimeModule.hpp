#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"
#include <chrono>

namespace RenderStar::Common::Time
{
    class TimeModule final : public Module::AbstractModule
    {
    public:

        TimeModule();

        void Tick();

        [[nodiscard]]
        float GetDeltaTime() const;

        [[nodiscard]]
        float GetDeltaTimeMilliseconds() const;

        [[nodiscard]]
        double GetElapsedTime() const;

        [[nodiscard]]
        int64_t GetFrameCount() const;

        [[nodiscard]]
        float GetFramesPerSecond() const;

        [[nodiscard]]
        float GetFrameTime() const;

    protected:

        void OnInitialize(Module::ModuleContext& context) override;

    private:

        using Clock = std::chrono::high_resolution_clock;
        using TimePoint = std::chrono::time_point<Clock>;
        using Duration = std::chrono::duration<double>;

        TimePoint startTime;
        TimePoint lastFrameTime;
        TimePoint lastFpsUpdateTime;

        float deltaTime;
        double elapsedTime;

        int64_t frameCount;
        int64_t fpsFrameCount;

        float framesPerSecond;
        float frameTime;
    };
}
