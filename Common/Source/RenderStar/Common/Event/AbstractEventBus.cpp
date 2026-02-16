#include "RenderStar/Common/Event/AbstractEventBus.hpp"
#include <algorithm>

namespace RenderStar::Common::Event
{
    AbstractEventBus::AbstractEventBus(const bool runsOnMainThread) : logger(spdlog::default_logger()), running(false), mainThread(runsOnMainThread) { }

    AbstractEventBus::~AbstractEventBus()
    {
        if (running.load())
            AbstractEventBus::Shutdown();
    }

    void AbstractEventBus::Start()
    {
        running.store(true);

        if (!mainThread)
        {
            processingThread = std::jthread([this](const std::stop_token& stopToken)
            {
                while (!stopToken.stop_requested() && running.load())
                    ProcessEvents();
            });
        }

        logger->info("Event bus '{}' started", GetBusName());
    }

    void AbstractEventBus::Run()
    {
        logger->info("Event bus '{}' entering main loop, running={}", GetBusName(), running.load());

        while (running.load())
        {
            ProcessEvents();

            if (tickHandler)
                tickHandler();
        }

        logger->info("Event bus '{}' exiting main loop", GetBusName());
    }

    void AbstractEventBus::Shutdown()
    {
        running.store(false);
        queueCondition.notify_all();

        if (processingThread.joinable())
            processingThread.request_stop();

        logger->info("Event bus '{}' shut down", GetBusName());
    }

    bool AbstractEventBus::RunsOnMainThread() const
    {
        return mainThread;
    }

    bool AbstractEventBus::HasTickHandler() const
    {
        return static_cast<bool>(tickHandler);
    }

    void AbstractEventBus::SubscribeRaw(const std::type_index eventType, EventHandlerFunction handler, const HandlerPriority priority)
    {
        std::lock_guard lock(queueMutex);

        auto& handlerList = handlers[eventType];
        handlerList.push_back({ std::move(handler), priority });

        std::sort(handlerList.begin(), handlerList.end());
    }

    void AbstractEventBus::PublishRaw(std::unique_ptr<IEvent> event, const EventPriority priority)
    {
        if (deferred.load())
        {
            std::lock_guard lock(queueMutex);
            deferredEvents.push_back({ std::move(event), priority });
            return;
        }

        if (mainThread)
        {
            DispatchEvent(*event);
            return;
        }

        {
            std::lock_guard lock(queueMutex);
            eventQueue.push({ std::move(event), priority });
        }

        queueCondition.notify_one();
    }

    void AbstractEventBus::SetTickHandler(TickHandlerFunction handler)
    {
        tickHandler = std::move(handler);
    }

    void AbstractEventBus::SetDeferred(const bool value)
    {
        deferred.store(value);
    }

    void AbstractEventBus::FlushDeferred()
    {
        deferred.store(false);

        std::vector<PrioritizedEvent> pending;

        {
            std::lock_guard lock(queueMutex);
            pending = std::move(deferredEvents);
            deferredEvents.clear();
        }

        std::sort(pending.begin(), pending.end(), [](const PrioritizedEvent& a, const PrioritizedEvent& b)
        {
            return static_cast<int32_t>(a.priority) < static_cast<int32_t>(b.priority);
        });

        for (const auto& entry : pending)
            DispatchEvent(*entry.event);
    }

    void AbstractEventBus::ProcessEvents()
    {
        std::unique_lock lock(queueMutex);

        if (eventQueue.empty())
        {
            if (mainThread)
                return;

            queueCondition.wait_for(lock, std::chrono::milliseconds(10));
            return;
        }

        PrioritizedEvent prioritizedEvent = std::move(const_cast<PrioritizedEvent&>(eventQueue.top()));
        eventQueue.pop();
        lock.unlock();

        DispatchEvent(*prioritizedEvent.event);
    }

    void AbstractEventBus::DispatchEvent(const IEvent& event)
    {
        const auto typeIndex = event.GetTypeIndex();

        std::lock_guard lock(queueMutex);

        const auto iterator = handlers.find(typeIndex);

        if (iterator == handlers.end())
            return;

        for (auto& [handler, priority] : iterator->second)
        {
            auto [type, message] = handler(event);

            if (type == EventResultType::FATAL)
            {
                logger->error("Fatal event handler error: {}", message);
                running.store(false);

                return;
            }

            if (type == EventResultType::FAILURE)
                logger->warn("Event handler failed: {}", message);
        }
    }
}
