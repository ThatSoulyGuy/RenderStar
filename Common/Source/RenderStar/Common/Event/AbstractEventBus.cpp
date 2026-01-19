#include "RenderStar/Common/Event/AbstractEventBus.hpp"
#include <algorithm>

namespace RenderStar::Common::Event
{
    AbstractEventBus::AbstractEventBus(bool runsOnMainThread)
        : logger(spdlog::default_logger())
        , running(false)
        , mainThread(runsOnMainThread)
    {
    }

    AbstractEventBus::~AbstractEventBus()
    {
        if (running.load())
            Shutdown();
    }

    void AbstractEventBus::Start()
    {
        running.store(true);

        if (!mainThread)
        {
            processingThread = std::jthread([this](std::stop_token stopToken)
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

    void AbstractEventBus::SubscribeRaw(
        std::type_index eventType,
        EventHandlerFunction handler,
        HandlerPriority priority
    )
    {
        std::lock_guard<std::mutex> lock(queueMutex);

        auto& handlerList = handlers[eventType];
        handlerList.push_back({ std::move(handler), priority });

        std::sort(handlerList.begin(), handlerList.end());
    }

    void AbstractEventBus::PublishRaw(
        std::unique_ptr<IEvent> event,
        EventPriority priority
    )
    {
        if (mainThread)
        {
            DispatchEvent(*event);
            return;
        }

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            eventQueue.push({ std::move(event), priority });
        }

        queueCondition.notify_one();
    }

    void AbstractEventBus::SetTickHandler(TickHandlerFunction handler)
    {
        tickHandler = std::move(handler);
    }

    void AbstractEventBus::ProcessEvents()
    {
        std::unique_lock<std::mutex> lock(queueMutex);

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
        auto typeIndex = event.GetTypeIndex();

        std::lock_guard<std::mutex> lock(queueMutex);

        auto iterator = handlers.find(typeIndex);

        if (iterator == handlers.end())
            return;

        for (auto& prioritizedHandler : iterator->second)
        {
            EventResult result = prioritizedHandler.handler(event);

            if (result.type == EventResultType::FATAL)
            {
                logger->error("Fatal event handler error: {}", result.message);
                running.store(false);
                return;
            }

            if (result.type == EventResultType::FAILURE)
                logger->warn("Event handler failed: {}", result.message);
        }
    }
}
