#include <gtest/gtest.h>
#include "RenderStar/Common/Module/ModuleManager.hpp"
#include "RenderStar/Common/Module/AbstractModule.hpp"

using namespace RenderStar::Common::Module;

class TrackingModule final : public AbstractModule
{
public:
    bool initCalled = false;
    bool cleanupCalled = false;
    int cleanupOrder = -1;
    static int globalCleanupCounter;

    void OnInitialize(ModuleContext&) override { initCalled = true; }
    void OnCleanup() override
    {
        cleanupCalled = true;
        cleanupOrder = globalCleanupCounter++;
    }
};

int TrackingModule::globalCleanupCounter = 0;

class OtherTrackingModule final : public AbstractModule
{
public:
    bool initCalled = false;
    bool cleanupCalled = false;
    int cleanupOrder = -1;

    void OnInitialize(ModuleContext&) override { initCalled = true; }
    void OnCleanup() override
    {
        cleanupCalled = true;
        cleanupOrder = TrackingModule::globalCleanupCounter++;
    }
};

class ModuleManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        TrackingModule::globalCleanupCounter = 0;
    }
};

TEST_F(ModuleManagerTest, RegisterAndRetrieveModule)
{
    auto tm = std::make_unique<TrackingModule>();
    auto builder = ModuleManager::Builder();
    auto mgr = builder.Module(std::move(tm)).Build();
    mgr->Start();

    auto mod = mgr->GetContext().GetModule<TrackingModule>();
    ASSERT_TRUE(mod.has_value());
    mgr->Shutdown();
}

TEST_F(ModuleManagerTest, OnInitializeCalledDuringStart)
{
    auto tm = std::make_unique<TrackingModule>();
    auto* ptr = tm.get();
    auto builder = ModuleManager::Builder();
    auto mgr = builder.Module(std::move(tm)).Build();

    EXPECT_FALSE(ptr->initCalled);
    mgr->Start();
    EXPECT_TRUE(ptr->initCalled);
    mgr->Shutdown();
}

TEST_F(ModuleManagerTest, OnCleanupCalledDuringShutdown)
{
    auto tm = std::make_unique<TrackingModule>();
    auto* ptr = tm.get();
    auto builder = ModuleManager::Builder();
    auto mgr = builder.Module(std::move(tm)).Build();
    mgr->Start();

    EXPECT_FALSE(ptr->cleanupCalled);
    mgr->Shutdown();
    EXPECT_TRUE(ptr->cleanupCalled);
}

TEST_F(ModuleManagerTest, CleanupInReverseOrder)
{
    auto m1 = std::make_unique<TrackingModule>();
    auto m2 = std::make_unique<OtherTrackingModule>();
    auto* p1 = m1.get();
    auto* p2 = m2.get();

    auto builder = ModuleManager::Builder();
    auto mgr = builder.Module(std::move(m1)).Module(std::move(m2)).Build();
    mgr->Start();
    mgr->Shutdown();

    EXPECT_TRUE(p2->cleanupOrder < p1->cleanupOrder);
}

TEST_F(ModuleManagerTest, GetModuleReturnsNulloptForUnregistered)
{
    auto builder = ModuleManager::Builder();
    auto mgr = builder.Build();
    mgr->Start();

    auto mod = mgr->GetContext().GetModule<TrackingModule>();
    EXPECT_FALSE(mod.has_value());
    mgr->Shutdown();
}

TEST_F(ModuleManagerTest, MultipleModules)
{
    auto m1 = std::make_unique<TrackingModule>();
    auto m2 = std::make_unique<OtherTrackingModule>();

    auto builder = ModuleManager::Builder();
    auto mgr = builder.Module(std::move(m1)).Module(std::move(m2)).Build();
    mgr->Start();

    EXPECT_TRUE(mgr->GetContext().GetModule<TrackingModule>().has_value());
    EXPECT_TRUE(mgr->GetContext().GetModule<OtherTrackingModule>().has_value());
    mgr->Shutdown();
}

TEST_F(ModuleManagerTest, IsRunningState)
{
    auto builder = ModuleManager::Builder();
    auto mgr = builder.Build();

    EXPECT_FALSE(mgr->IsRunning());
    mgr->Start();
    EXPECT_TRUE(mgr->IsRunning());
    mgr->Shutdown();
    EXPECT_FALSE(mgr->IsRunning());
}

TEST_F(ModuleManagerTest, ModuleContextPassedToInitialize)
{
    class ContextCheckModule final : public AbstractModule
    {
    public:
        ModuleContext* receivedContext = nullptr;
        void OnInitialize(ModuleContext& ctx) override { receivedContext = &ctx; }
    };

    auto m = std::make_unique<ContextCheckModule>();
    auto* ptr = m.get();
    auto builder = ModuleManager::Builder();
    auto mgr = builder.Module(std::move(m)).Build();
    mgr->Start();

    EXPECT_NE(ptr->receivedContext, nullptr);
    mgr->Shutdown();
}
