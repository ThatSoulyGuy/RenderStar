#pragma once

#include "RenderStar/Common/Module/ModuleContext.hpp"
#include <spdlog/spdlog.h>
#include <atomic>
#include <latch>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace RenderStar::Common::Event
{
    class IEventBus;
}

namespace RenderStar::Common::Module
{
    class IModule;

    class ModuleManager
    {
    public:

        class Builder
        {
        public:

            template<typename ModuleType>
            Builder& Module(std::unique_ptr<ModuleType> module);

            template<typename EventBusType>
            Builder& EventBus(std::unique_ptr<EventBusType> eventBus);

            std::unique_ptr<ModuleManager> Build();

        private:

            std::unordered_map<std::type_index, std::unique_ptr<IModule>> modules;
            std::unordered_map<std::type_index, std::unique_ptr<Event::IEventBus>> eventBuses;
            std::vector<std::type_index> moduleOrder;
        };

        ModuleManager(std::unordered_map<std::type_index, std::unique_ptr<IModule>> modules, std::unordered_map<std::type_index, std::unique_ptr<Event::IEventBus>> eventBuses, std::vector<std::type_index> moduleOrder);

        void Start();

        void RunMainLoop();

        void Shutdown();

        bool IsRunning() const;

        ModuleContext& GetContext() const;

    private:

        std::shared_ptr<spdlog::logger> logger;
        std::unordered_map<std::type_index, std::unique_ptr<IModule>> modules;
        std::unordered_map<std::type_index, std::unique_ptr<Event::IEventBus>> eventBuses;
        std::vector<std::type_index> moduleOrder;
        std::unique_ptr<ModuleContext> context;
        std::atomic<bool> isRunning;
        std::latch shutdownLatch;
    };
}

#include "RenderStar/Common/Module/ModuleManager.inl"
