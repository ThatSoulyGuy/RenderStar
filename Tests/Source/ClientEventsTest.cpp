#include <gtest/gtest.h>
#include "RenderStar/Client/Event/Events/ClientEvents.hpp"

using namespace RenderStar::Client::Event::Events;

TEST(ClientEventsTest, PreinitializationEventName)
{
    ClientPreinitializationEvent event;
    EXPECT_EQ(event.GetName(), "ClientPreinitializationEvent");
}

TEST(ClientEventsTest, InitializationEventName)
{
    ClientInitializationEvent event;
    EXPECT_EQ(event.GetName(), "ClientInitializationEvent");
}

TEST(ClientEventsTest, WindowCreatedEventName)
{
    ClientWindowCreatedEvent event;
    EXPECT_EQ(event.GetName(), "ClientWindowCreatedEvent");
}

TEST(ClientEventsTest, RendererInitializedEvent)
{
    ClientRendererInitializedEvent event;
    EXPECT_EQ(event.GetName(), "ClientRendererInitializedEvent");
    EXPECT_EQ(event.backend, nullptr);
}

TEST(ClientEventsTest, ConnectedEvent)
{
    ClientConnectedEvent event("localhost", 25565);
    EXPECT_EQ(event.GetName(), "ClientConnectedEvent");
    EXPECT_EQ(event.host, "localhost");
    EXPECT_EQ(event.port, 25565);
}

TEST(ClientEventsTest, DisconnectedEvent)
{
    ClientDisconnectedEvent event("timed out");
    EXPECT_EQ(event.GetName(), "ClientDisconnectedEvent");
    EXPECT_EQ(event.reason, "timed out");
}

TEST(ClientEventsTest, UpdateEventName)
{
    ClientUpdateEvent event;
    EXPECT_EQ(event.GetName(), "ClientUpdateEvent");
}

TEST(ClientEventsTest, RenderFrameEvent)
{
    ClientRenderFrameEvent event;
    EXPECT_EQ(event.GetName(), "ClientRenderFrameEvent");
    EXPECT_EQ(event.backend, nullptr);
}

TEST(ClientEventsTest, DefaultConnectedEvent)
{
    ClientConnectedEvent event;
    EXPECT_TRUE(event.host.empty());
    EXPECT_EQ(event.port, 0);
}
