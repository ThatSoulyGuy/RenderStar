#pragma once

#include "RenderStar/Common/Event/IEventBus.hpp"
#include "RenderStar/Common/Event/IEvent.hpp"
#include <spdlog/spdlog.h>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>
#include <memory>

namespace RenderStar::Common::Event
{
    class AbstractEventBus : public IEventBus
    {
    public:

        AbstractEventBus(bool runsOnMainThread);

        ~AbstractEventBus() override;

        void Start() override;

        void Run() override;

        void Shutdown() override;

        bool RunsOnMainThread() const override;

        bool HasTickHandler() const override;

        void SubscribeRaw(std::type_index eventType, EventHandlerFunction handler, HandlerPriority priority) override;

        void PublishRaw(std::unique_ptr<IEvent> event, EventPriority priority) override;

        void SetTickHandler(TickHandlerFunction handler) override;

        template <typename EventType>
        void Subscribe(std::function<EventResult(const EventType&)> handler, HandlerPriority priority = HandlerPriority::NORMAL);

        template <typename EventType>
        void Publish(EventType event, EventPriority priority = EventPriority::NORMAL);

    protected:

        virtual std::string_view GetBusName() const = 0;

    private:

        struct PrioritizedHandler
        {
            EventHandlerFunction handler;
            HandlerPriority priority;

            bool operator<(const PrioritizedHandler& other) const
            {
                return static_cast<int32_t>(priority) < static_cast<int32_t>(other.priority);
            }
        };

        struct PrioritizedEvent
        {
            std::unique_ptr<IEvent> event;
            EventPriority priority;

            bool operator<(const PrioritizedEvent& other) const
            {
                return static_cast<int32_t>(priority) > static_cast<int32_t>(other.priority);
            }
        };

        void ProcessEvents();

        void DispatchEvent(const IEvent& event);

        std::shared_ptr<spdlog::logger> logger;
        std::unordered_map<std::type_index, std::vector<PrioritizedHandler>> handlers;
        std::priority_queue<PrioritizedEvent> eventQueue;
        std::mutex queueMutex;
        std::condition_variable queueCondition;
        std::atomic<bool> running;
        std::jthread processingThread;

        TickHandlerFunction tickHandler;

        bool mainThread;
    };
}

#include "RenderStar/Common/Event/AbstractEventBus.inl"
