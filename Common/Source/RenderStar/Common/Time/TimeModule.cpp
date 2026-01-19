#include "RenderStar/Common/Time/TimeModule.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"

namespace RenderStar::Common::Time
{
    TimeModule::TimeModule()
        : deltaTime(0.0f)
        , elapsedTime(0.0)
        , frameCount(0)
        , fpsFrameCount(0)
        , framesPerSecond(0.0f)
        , frameTime(0.0f)
    {
    }

    void TimeModule::Tick()
    {
        TimePoint currentTime = Clock::now();
        Duration frameDuration = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        deltaTime = static_cast<float>(frameDuration.count());
        frameTime = deltaTime * 1000.0f;

        Duration totalDuration = currentTime - startTime;
        elapsedTime = totalDuration.count();

        frameCount++;
        fpsFrameCount++;

        Duration fpsDuration = currentTime - lastFpsUpdateTime;

        if (fpsDuration.count() >= 1.0)
        {
            framesPerSecond = static_cast<float>(fpsFrameCount) / static_cast<float>(fpsDuration.count());
            fpsFrameCount = 0;
            lastFpsUpdateTime = currentTime;
        }
    }

    float TimeModule::GetDeltaTime() const
    {
        return deltaTime;
    }

    float TimeModule::GetDeltaTimeMilliseconds() const
    {
        return deltaTime * 1000.0f;
    }

    double TimeModule::GetElapsedTime() const
    {
        return elapsedTime;
    }

    int64_t TimeModule::GetFrameCount() const
    {
        return frameCount;
    }

    float TimeModule::GetFramesPerSecond() const
    {
        return framesPerSecond;
    }

    float TimeModule::GetFrameTime() const
    {
        return frameTime;
    }

    void TimeModule::OnInitialize(Module::ModuleContext& context)
    {
        startTime = Clock::now();
        lastFrameTime = startTime;
        lastFpsUpdateTime = startTime;
        logger->info("TimeModule initialized");
    }
}
