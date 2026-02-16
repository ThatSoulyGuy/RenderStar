#include <gtest/gtest.h>
#include "RenderStar/Common/Module/ModuleManager.hpp"
#include "RenderStar/Common/Module/AbstractModule.hpp"

using namespace RenderStar::Common::Module;

class TrackingModule final : public AbstractModule
{
public:
    bool initCalled = false;
    bool cleanupCalled = false;
    int initOrder = -1;
    int cleanupOrder = -1;
    static int globalInitCounter;
    static int globalCleanupCounter;

    void OnInitialize(ModuleContext&) override
    {
        initCalled = true;
        initOrder = globalInitCounter++;
    }
    void OnCleanup() override
    {
        cleanupCalled = true;
        cleanupOrder = globalCleanupCounter++;
    }
};

int TrackingModule::globalInitCounter = 0;
int TrackingModule::globalCleanupCounter = 0;

class OtherTrackingModule final : public AbstractModule
{
public:
    bool initCalled = false;
    bool cleanupCalled = false;
    int initOrder = -1;
    int cleanupOrder = -1;

    void OnInitialize(ModuleContext&) override
    {
        initCalled = true;
        initOrder = TrackingModule::globalInitCounter++;
    }
    void OnCleanup() override
    {
        cleanupCalled = true;
        cleanupOrder = TrackingModule::globalCleanupCounter++;
    }
};

class DependentModule final : public AbstractModule
{
public:
    bool initCalled = false;
    bool cleanupCalled = false;
    int initOrder = -1;
    int cleanupOrder = -1;

    void OnInitialize(ModuleContext&) override
    {
        initCalled = true;
        initOrder = TrackingModule::globalInitCounter++;
    }
    void OnCleanup() override
    {
        cleanupCalled = true;
        cleanupOrder = TrackingModule::globalCleanupCounter++;
    }
    std::vector<std::type_index> GetDependencies() const override
    {
        return DependsOn<TrackingModule>();
    }
};

class ModuleManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        TrackingModule::globalInitCounter = 0;
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

TEST_F(ModuleManagerTest, DependencyOrderedInitialization)
{
    auto dep = std::make_unique<DependentModule>();
    auto base = std::make_unique<TrackingModule>();
    auto* depPtr = dep.get();
    auto* basePtr = base.get();

    auto mgr = ModuleManager::Builder()
        .Module(std::move(dep))
        .Module(std::move(base))
        .Build();
    mgr->Start();

    EXPECT_TRUE(basePtr->initOrder < depPtr->initOrder);
    mgr->Shutdown();
}

TEST_F(ModuleManagerTest, DependencyOrderedCleanup)
{
    auto dep = std::make_unique<DependentModule>();
    auto base = std::make_unique<TrackingModule>();
    auto* depPtr = dep.get();
    auto* basePtr = base.get();

    auto mgr = ModuleManager::Builder()
        .Module(std::move(dep))
        .Module(std::move(base))
        .Build();
    mgr->Start();
    mgr->Shutdown();

    EXPECT_TRUE(depPtr->cleanupOrder < basePtr->cleanupOrder);
}

TEST_F(ModuleManagerTest, DestructorCallsShutdown)
{
    bool cleanedUp = false;

    class NotifyModule final : public AbstractModule
    {
    public:
        explicit NotifyModule(bool& flag) : flag(flag) { }
        void OnInitialize(ModuleContext&) override { }
        void OnCleanup() override { flag = true; }
    private:
        bool& flag;
    };

    {
        auto mgr = ModuleManager::Builder()
            .Module(std::make_unique<NotifyModule>(cleanedUp))
            .Build();
        mgr->Start();
        EXPECT_FALSE(cleanedUp);
    }

    EXPECT_TRUE(cleanedUp);
}

TEST_F(ModuleManagerTest, NoDependenciesPreservesRegistrationOrder)
{
    auto m1 = std::make_unique<TrackingModule>();
    auto m2 = std::make_unique<OtherTrackingModule>();
    auto* p1 = m1.get();
    auto* p2 = m2.get();

    auto mgr = ModuleManager::Builder()
        .Module(std::move(m1))
        .Module(std::move(m2))
        .Build();
    mgr->Start();

    EXPECT_TRUE(p1->initOrder < p2->initOrder);
    mgr->Shutdown();
}
