#pragma once

#include "RenderStar/Common/Event/IEvent.hpp"
#include <string_view>
#include <string>
#include <cstdint>

namespace RenderStar::Client::Event::Events
{
    using namespace RenderStar::Common::Event;

    struct ClientPreinitializationEvent final : Event<ClientPreinitializationEvent>
    {
        std::string_view GetName() const override { return "ClientPreinitializationEvent"; }
    };

    struct ClientInitializationEvent final : Event<ClientInitializationEvent>
    {
        std::string_view GetName() const override { return "ClientInitializationEvent"; }
    };

    struct ClientWindowCreatedEvent final : Event<ClientWindowCreatedEvent>
    {
        std::string_view GetName() const override { return "ClientWindowCreatedEvent"; }
    };

    struct ClientRendererInitializedEvent final : Event<ClientRendererInitializedEvent>
    {
        std::string_view GetName() const override { return "ClientRendererInitializedEvent"; }
    };

    struct ClientConnectedEvent final : Event<ClientConnectedEvent>
    {
        std::string host;
        int32_t port;

        ClientConnectedEvent() : port(0) { }
        ClientConnectedEvent(std::string hostValue, const int32_t portValue) : host(std::move(hostValue)), port(portValue) {}

        std::string_view GetName() const override { return "ClientConnectedEvent"; }
    };

    struct ClientDisconnectedEvent final : Event<ClientDisconnectedEvent>
    {
        std::string reason;

        ClientDisconnectedEvent() {}

        explicit ClientDisconnectedEvent(std::string reasonValue) : reason(std::move(reasonValue)) {}

        std::string_view GetName() const override { return "ClientDisconnectedEvent"; }
    };

    struct ClientUpdateEvent final : Event<ClientUpdateEvent>
    {
        std::string_view GetName() const override { return "ClientUpdateEvent"; }
    };

    struct ClientRenderFrameEvent final : Event<ClientRenderFrameEvent>
    {
        std::string_view GetName() const override { return "ClientRenderFrameEvent"; }
    };
}
