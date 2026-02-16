#pragma once

#include "RenderStar/Common/Event/EventPriority.hpp"
#include "RenderStar/Common/Event/HandlerPriority.hpp"
#include "RenderStar/Common/Event/EventResult.hpp"
#include <functional>
#include <typeindex>
#include <memory>

namespace RenderStar::Common::Event
{
    class IEvent;

    using EventHandlerFunction = std::function<EventResult(const IEvent&)>;
    using TickHandlerFunction = std::function<void()>;

    class IEventBus
    {
    public:

        virtual ~IEventBus() = default;

        virtual void Start() = 0;

        virtual void Run() = 0;

        virtual void Shutdown() = 0;

        [[nodiscard]]
        virtual bool RunsOnMainThread() const = 0;

        [[nodiscard]]
        virtual bool HasTickHandler() const = 0;

        virtual void SubscribeRaw(std::type_index eventType,EventHandlerFunction handler,HandlerPriority priority) = 0;
        virtual void PublishRaw(std::unique_ptr<IEvent> event,EventPriority priority) = 0;

        virtual void SetTickHandler(TickHandlerFunction handler) = 0;

        virtual void SetDeferred(bool deferred) = 0;
        virtual void FlushDeferred() = 0;
    };
}
