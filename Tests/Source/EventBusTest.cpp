#include <gtest/gtest.h>
#include "RenderStar/Common/Event/AbstractEventBus.hpp"
#include "RenderStar/Common/Event/IEvent.hpp"
#include "RenderStar/Common/Event/EventResult.hpp"
#include "RenderStar/Common/Event/HandlerPriority.hpp"

using namespace RenderStar::Common::Event;

struct TestEvent : public Event<TestEvent>
{
    int32_t value;

    explicit TestEvent(int32_t v) : value(v) {}

    std::string_view GetName() const override { return "TestEvent"; }
};

struct AnotherTestEvent : public Event<AnotherTestEvent>
{
    std::string message;

    explicit AnotherTestEvent(std::string msg) : message(std::move(msg)) {}

    std::string_view GetName() const override { return "AnotherTestEvent"; }
};

class TestEventBus : public AbstractEventBus
{
public:

    TestEventBus() : AbstractEventBus(true) {}

    std::string_view GetName() const { return GetBusName(); }

protected:

    std::string_view GetBusName() const override
    {
        return "test_event_bus";
    }
};

class EventBusTest : public ::testing::Test
{
protected:

    TestEventBus eventBus;

    void SetUp() override
    {
        eventBus.Start();
    }

    void TearDown() override
    {
        eventBus.Shutdown();
    }
};

TEST_F(EventBusTest, SubscribeAndPublish)
{
    int32_t receivedValue = 0;

    eventBus.Subscribe<TestEvent>([&receivedValue](const TestEvent& event)
    {
        receivedValue = event.value;
        return EventResult::Success();
    });

    eventBus.Publish(TestEvent{42});

    EXPECT_EQ(receivedValue, 42);
}

TEST_F(EventBusTest, MultipleSubscribers)
{
    int32_t sum = 0;

    eventBus.Subscribe<TestEvent>([&sum](const TestEvent& event)
    {
        sum += event.value;
        return EventResult::Success();
    });

    eventBus.Subscribe<TestEvent>([&sum](const TestEvent& event)
    {
        sum += event.value * 2;
        return EventResult::Success();
    });

    eventBus.Publish(TestEvent{10});

    EXPECT_EQ(sum, 30);
}

TEST_F(EventBusTest, DifferentEventTypes)
{
    int32_t intValue = 0;
    std::string stringValue;

    eventBus.Subscribe<TestEvent>([&intValue](const TestEvent& event)
    {
        intValue = event.value;
        return EventResult::Success();
    });

    eventBus.Subscribe<AnotherTestEvent>([&stringValue](const AnotherTestEvent& event)
    {
        stringValue = event.message;
        return EventResult::Success();
    });

    eventBus.Publish(TestEvent{42});
    eventBus.Publish(AnotherTestEvent{"Hello"});

    EXPECT_EQ(intValue, 42);
    EXPECT_EQ(stringValue, "Hello");
}

TEST_F(EventBusTest, PriorityOrdering)
{
    std::vector<int32_t> order;

    eventBus.Subscribe<TestEvent>([&order](const TestEvent&)
    {
        order.push_back(2);
        return EventResult::Success();
    }, HandlerPriority::NORMAL);

    eventBus.Subscribe<TestEvent>([&order](const TestEvent&)
    {
        order.push_back(1);
        return EventResult::Success();
    }, HandlerPriority::HIGH);

    eventBus.Subscribe<TestEvent>([&order](const TestEvent&)
    {
        order.push_back(3);
        return EventResult::Success();
    }, HandlerPriority::LOW);

    eventBus.Publish(TestEvent{0});

    ASSERT_EQ(order.size(), 3);
    EXPECT_EQ(order[0], 1);
    EXPECT_EQ(order[1], 2);
    EXPECT_EQ(order[2], 3);
}

TEST_F(EventBusTest, NoSubscribersDoesNotCrash)
{
    EXPECT_NO_THROW(eventBus.Publish(TestEvent{42}));
}

TEST_F(EventBusTest, GetBusNameReturnsCorrectName)
{
    EXPECT_EQ(eventBus.GetName(), "test_event_bus");
}
