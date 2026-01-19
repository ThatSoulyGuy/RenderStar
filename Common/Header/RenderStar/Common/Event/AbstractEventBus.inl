#pragma once

namespace RenderStar::Common::Event
{
    template<typename EventType>
    void AbstractEventBus::Subscribe(
        std::function<EventResult(const EventType&)> handler,
        HandlerPriority priority
    )
    {
        auto wrappedHandler = [handler](const IEvent& event) -> EventResult
        {
            return handler(static_cast<const EventType&>(event));
        };

        SubscribeRaw(std::type_index(typeid(EventType)), wrappedHandler, priority);
    }

    template<typename EventType>
    void AbstractEventBus::Publish(EventType event, EventPriority priority)
    {
        PublishRaw(std::make_unique<EventType>(std::move(event)), priority);
    }
}
