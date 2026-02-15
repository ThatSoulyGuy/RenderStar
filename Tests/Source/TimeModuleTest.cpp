#include <gtest/gtest.h>
#include "RenderStar/Common/Time/TimeModule.hpp"
#include "RenderStar/Common/Module/ModuleManager.hpp"
#include <thread>

using namespace RenderStar::Common::Time;
using namespace RenderStar::Common::Module;

class TimeModuleTest : public ::testing::Test
{
protected:
    std::unique_ptr<ModuleManager> manager;
    TimeModule* timeModule = nullptr;

    void SetUp() override
    {
        auto builder = ModuleManager::Builder();
        auto tm = std::make_unique<TimeModule>();
        timeModule = tm.get();
        manager = builder.Module(std::move(tm)).Build();
        manager->Start();
    }

    void TearDown() override
    {
        manager->Shutdown();
    }
};

TEST_F(TimeModuleTest, InitialFrameCount)
{
    EXPECT_EQ(timeModule->GetFrameCount(), 0);
}

TEST_F(TimeModuleTest, InitialElapsedTime)
{
    EXPECT_GE(timeModule->GetElapsedTime(), 0.0);
}

TEST_F(TimeModuleTest, TickIncrementsFrameCount)
{
    timeModule->Tick();
    EXPECT_EQ(timeModule->GetFrameCount(), 1);

    timeModule->Tick();
    EXPECT_EQ(timeModule->GetFrameCount(), 2);
}

TEST_F(TimeModuleTest, DeltaTimePositiveAfterTick)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    timeModule->Tick();
    EXPECT_GT(timeModule->GetDeltaTime(), 0.0f);
}

TEST_F(TimeModuleTest, DeltaTimeMillisecondsRelation)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    timeModule->Tick();
    float dtSec = timeModule->GetDeltaTime();
    float dtMs = timeModule->GetDeltaTimeMilliseconds();
    EXPECT_NEAR(dtMs, dtSec * 1000.0f, 0.1f);
}

TEST_F(TimeModuleTest, ElapsedTimeGrows)
{
    double before = timeModule->GetElapsedTime();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    timeModule->Tick();
    double after = timeModule->GetElapsedTime();
    EXPECT_GT(after, before);
}

TEST_F(TimeModuleTest, MultipleTicksAccumulate)
{
    for (int i = 0; i < 10; ++i)
        timeModule->Tick();

    EXPECT_EQ(timeModule->GetFrameCount(), 10);
}

TEST_F(TimeModuleTest, FrameTimeReasonable)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    timeModule->Tick();
    EXPECT_GE(timeModule->GetFrameTime(), 0.0f);
}
