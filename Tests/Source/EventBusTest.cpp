#include <gtest/gtest.h>
#include "RenderStar/Common/Event/AbstractEventBus.hpp"
#include "RenderStar/Common/Event/EventResult.hpp"

using namespace RenderStar::Common::Event;

class TestEventBus final : public AbstractEventBus
{
public:
    TestEventBus() : AbstractEventBus(true) {}
protected:
    std::string_view GetBusName() const override { return "TestEventBus"; }
};

struct TestEvent final : TypedEvent<TestEvent>
{
    int value = 0;
    explicit TestEvent(int v = 0) : value(v) {}
    std::string_view GetName() const override { return "TestEvent"; }
};

struct OtherEvent final : TypedEvent<OtherEvent>
{
    std::string message;
    explicit OtherEvent(std::string m = "") : message(std::move(m)) {}
    std::string_view GetName() const override { return "OtherEvent"; }
};

class EventBusTest : public ::testing::Test
{
protected:
    TestEventBus bus;

    void SetUp() override
    {
        bus.Start();
    }

    void TearDown() override
    {
        bus.Shutdown();
    }
};

TEST_F(EventBusTest, SubscribeAndPublish)
{
    int receivedValue = 0;
    bus.Subscribe<TestEvent>([&](const TestEvent& e) -> EventResult
    {
        receivedValue = e.value;
        return EventResult::Success();
    });

    bus.Publish(TestEvent{42});

    EXPECT_EQ(receivedValue, 42);
}

TEST_F(EventBusTest, MultipleSubscribers)
{
    int count = 0;
    bus.Subscribe<TestEvent>([&](const TestEvent&) -> EventResult
    {
        ++count;
        return EventResult::Success();
    });
    bus.Subscribe<TestEvent>([&](const TestEvent&) -> EventResult
    {
        ++count;
        return EventResult::Success();
    });

    bus.Publish(TestEvent{1});

    EXPECT_EQ(count, 2);
}

TEST_F(EventBusTest, HandlerPriorityOrdering)
{
    std::vector<int> order;

    bus.Subscribe<TestEvent>([&](const TestEvent&) -> EventResult
    {
        order.push_back(2);
        return EventResult::Success();
    }, HandlerPriority::LOW);

    bus.Subscribe<TestEvent>([&](const TestEvent&) -> EventResult
    {
        order.push_back(0);
        return EventResult::Success();
    }, HandlerPriority::HIGH);

    bus.Subscribe<TestEvent>([&](const TestEvent&) -> EventResult
    {
        order.push_back(1);
        return EventResult::Success();
    }, HandlerPriority::NORMAL);

    bus.Publish(TestEvent{1});

    ASSERT_EQ(order.size(), 3);
    EXPECT_EQ(order[0], 0);
    EXPECT_EQ(order[1], 1);
    EXPECT_EQ(order[2], 2);
}

TEST_F(EventBusTest, PublishWithNoSubscribers)
{
    bus.Publish(TestEvent{99});
}

TEST_F(EventBusTest, DifferentEventTypes)
{
    int testCount = 0;
    int otherCount = 0;

    bus.Subscribe<TestEvent>([&](const TestEvent&) -> EventResult
    {
        ++testCount;
        return EventResult::Success();
    });
    bus.Subscribe<OtherEvent>([&](const OtherEvent&) -> EventResult
    {
        ++otherCount;
        return EventResult::Success();
    });

    bus.Publish(TestEvent{1});

    EXPECT_EQ(testCount, 1);
    EXPECT_EQ(otherCount, 0);
}

TEST_F(EventBusTest, EventDataPreserved)
{
    std::string receivedMessage;
    bus.Subscribe<OtherEvent>([&](const OtherEvent& e) -> EventResult
    {
        receivedMessage = e.message;
        return EventResult::Success();
    });

    bus.Publish(OtherEvent{"hello world"});

    EXPECT_EQ(receivedMessage, "hello world");
}

TEST_F(EventBusTest, RunsOnMainThread)
{
    EXPECT_TRUE(bus.RunsOnMainThread());
}

TEST_F(EventBusTest, MultiplePublishDispatched)
{
    int count = 0;
    bus.Subscribe<TestEvent>([&](const TestEvent&) -> EventResult
    {
        ++count;
        return EventResult::Success();
    });

    bus.Publish(TestEvent{1});
    bus.Publish(TestEvent{2});
    bus.Publish(TestEvent{3});

    EXPECT_EQ(count, 3);
}

TEST_F(EventBusTest, EventResultFromHandler)
{
    bus.Subscribe<TestEvent>([](const TestEvent&) -> EventResult
    {
        return EventResult::Failure("test failure");
    });

    bus.Publish(TestEvent{1});
}

TEST_F(EventBusTest, DeferredEventsQueuedUntilFlush)
{
    int receivedValue = 0;

    bus.SetDeferred(true);
    bus.Publish(TestEvent{42});

    bus.Subscribe<TestEvent>([&](const TestEvent& e) -> EventResult
    {
        receivedValue = e.value;
        return EventResult::Success();
    });

    EXPECT_EQ(receivedValue, 0);

    bus.FlushDeferred();

    EXPECT_EQ(receivedValue, 42);
}

TEST_F(EventBusTest, DeferredFlushDispatchesMultipleEvents)
{
    std::vector<int> received;

    bus.SetDeferred(true);
    bus.Publish(TestEvent{1});
    bus.Publish(TestEvent{2});
    bus.Publish(TestEvent{3});

    bus.Subscribe<TestEvent>([&](const TestEvent& e) -> EventResult
    {
        received.push_back(e.value);
        return EventResult::Success();
    });

    bus.FlushDeferred();

    ASSERT_EQ(received.size(), 3);
    EXPECT_EQ(received[0], 1);
    EXPECT_EQ(received[1], 2);
    EXPECT_EQ(received[2], 3);
}

TEST_F(EventBusTest, AfterFlushPublishesDispatchNormally)
{
    int count = 0;

    bus.SetDeferred(true);
    bus.Subscribe<TestEvent>([&](const TestEvent&) -> EventResult
    {
        ++count;
        return EventResult::Success();
    });
    bus.FlushDeferred();

    bus.Publish(TestEvent{1});

    EXPECT_EQ(count, 1);
}
