#pragma once

#include "RenderStar/Common/Event/IEvent.hpp"
#include <string_view>
#include <string>
#include <cstdint>

namespace RenderStar::Client::Render
{
    class IRenderBackend;
}

namespace RenderStar::Client::Event::Events
{
    using namespace RenderStar::Common::Event;

    struct ClientPreinitializationEvent final : TypedEvent<ClientPreinitializationEvent>
    {
        [[nodiscard]]
        std::string_view GetName() const override { return "ClientPreinitializationEvent"; }
    };

    struct ClientInitializationEvent final : TypedEvent<ClientInitializationEvent>
    {
        [[nodiscard]]
        std::string_view GetName() const override { return "ClientInitializationEvent"; }
    };

    struct ClientWindowCreatedEvent final : TypedEvent<ClientWindowCreatedEvent>
    {
        [[nodiscard]]
        std::string_view GetName() const override { return "ClientWindowCreatedEvent"; }
    };

    struct ClientRendererInitializedEvent final : TypedEvent<ClientRendererInitializedEvent>
    {
        Render::IRenderBackend* backend;

        ClientRendererInitializedEvent() : backend(nullptr) { }
        explicit ClientRendererInitializedEvent(Render::IRenderBackend* backend) : backend(backend) { }

        [[nodiscard]]
        std::string_view GetName() const override { return "ClientRendererInitializedEvent"; }
    };

    struct ClientConnectedEvent final : TypedEvent<ClientConnectedEvent>
    {
        std::string host;
        int32_t port;

        ClientConnectedEvent() : port(0) { }
        ClientConnectedEvent(std::string hostValue, const int32_t portValue) : host(std::move(hostValue)), port(portValue) {}

        [[nodiscard]]
        std::string_view GetName() const override { return "ClientConnectedEvent"; }
    };

    struct ClientDisconnectedEvent final : TypedEvent<ClientDisconnectedEvent>
    {
        std::string reason;

        ClientDisconnectedEvent() = default;

        explicit ClientDisconnectedEvent(std::string reasonValue) : reason(std::move(reasonValue)) {}

        [[nodiscard]]
        std::string_view GetName() const override { return "ClientDisconnectedEvent"; }
    };

    struct ClientUpdateEvent final : TypedEvent<ClientUpdateEvent>
    {
        [[nodiscard]]
        std::string_view GetName() const override { return "ClientUpdateEvent"; }
    };

    struct ClientRenderFrameEvent final : TypedEvent<ClientRenderFrameEvent>
    {
        Render::IRenderBackend* backend;

        ClientRenderFrameEvent() : backend(nullptr) { }
        explicit ClientRenderFrameEvent(Render::IRenderBackend* backend) : backend(backend) { }

        [[nodiscard]]
        std::string_view GetName() const override { return "ClientRenderFrameEvent"; }
    };
}
