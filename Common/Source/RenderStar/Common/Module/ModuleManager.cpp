#include "RenderStar/Common/Module/ModuleManager.hpp"
#include "RenderStar/Common/Event/IEventBus.hpp"
#include "RenderStar/Common/Module/IModule.hpp"
#include <algorithm>
#include <memory>
#include <queue>
#include <ranges>
#include <set>

namespace RenderStar::Common::Module
{
    std::unique_ptr<ModuleManager> ModuleManager::Builder::Build()
    {
        return std::make_unique<ModuleManager>(std::move(modules),std::move(eventBuses),std::move(moduleOrder));
    }

    ModuleManager::ModuleManager(std::unordered_map<std::type_index, std::unique_ptr<IModule>> modules,std::unordered_map<std::type_index, std::unique_ptr<Event::IEventBus>> eventBuses, std::vector<std::type_index> moduleOrder) : logger(spdlog::default_logger()), modules(std::move(modules)), eventBuses(std::move(eventBuses)), moduleOrder(std::move(moduleOrder)), context(nullptr), isRunning(false), shutdownLatch(1)
    {
        context = std::make_unique<ModuleContext>(this->modules, this->eventBuses);
    }

    ModuleManager::~ModuleManager()
    {
        if (isRunning.load())
            Shutdown();
    }

    void ModuleManager::Start()
    {
        if (bool expected = false; !isRunning.compare_exchange_strong(expected, true))
        {
            logger->warn("ModuleManager is already running!");
            return;
        }

        logger->info("Starting {} event bus(es)...", eventBuses.size());

        for (const auto& bus : eventBuses | std::views::values)
        {
            bus->Start();
            bus->SetDeferred(true);
        }

        moduleOrder = ResolveModuleOrder();

        logger->info("Registering {} top-level module(s)...", moduleOrder.size());

        for (const auto& typeIndex : moduleOrder)
            modules[typeIndex]->OnRegistration(*context);

        for (const auto& bus : eventBuses | std::views::values)
            bus->FlushDeferred();

        if (int32_t totalModules = context->GetTotalModuleCount(); totalModules > static_cast<int32_t>(modules.size()))
            logger->info("ModuleManager started successfully ({} modules total, including sub-modules)", totalModules);
        else
            logger->info("ModuleManager started successfully");
    }

    std::vector<std::type_index> ModuleManager::ResolveModuleOrder()
    {
        std::unordered_map<std::type_index, size_t> registrationIndex;
        for (size_t i = 0; i < moduleOrder.size(); ++i)
            registrationIndex[moduleOrder[i]] = i;

        std::set<std::type_index> registered(moduleOrder.begin(), moduleOrder.end());

        std::unordered_map<std::type_index, std::vector<std::type_index>> successors;
        std::unordered_map<std::type_index, int> inDegree;

        for (const auto& typeIndex : moduleOrder)
            inDegree[typeIndex] = 0;

        for (const auto& typeIndex : moduleOrder)
        {
            auto deps = modules[typeIndex]->GetDependencies();

            for (const auto& dep : deps)
            {
                if (!registered.contains(dep))
                {
                    logger->error("Module dependency not registered: module requires an unregistered dependency");
                    continue;
                }

                successors[dep].push_back(typeIndex);
                inDegree[typeIndex]++;
            }
        }

        auto cmp = [&registrationIndex](const std::type_index& a, const std::type_index& b)
        {
            return registrationIndex[a] > registrationIndex[b];
        };

        std::priority_queue<std::type_index, std::vector<std::type_index>, decltype(cmp)> ready(cmp);

        for (const auto& typeIndex : moduleOrder)
        {
            if (inDegree[typeIndex] == 0)
                ready.push(typeIndex);
        }

        std::vector<std::type_index> sorted;
        sorted.reserve(moduleOrder.size());

        while (!ready.empty())
        {
            auto current = ready.top();
            ready.pop();
            sorted.push_back(current);

            if (auto it = successors.find(current); it != successors.end())
            {
                for (const auto& successor : it->second)
                {
                    if (--inDegree[successor] == 0)
                        ready.push(successor);
                }
            }
        }

        if (sorted.size() != moduleOrder.size())
            logger->error("Circular dependency detected among modules! {} of {} modules resolved", sorted.size(), moduleOrder.size());

        return sorted;
    }

    void ModuleManager::RunMainLoop()
    {
        Event::IEventBus* mainThreadBus = nullptr;

        for (const auto& bus : eventBuses | std::views::values)
        {
            if (bus->RunsOnMainThread() && bus->HasTickHandler())
            {
                mainThreadBus = bus.get();
                break;
            }
        }

        if (mainThreadBus == nullptr)
        {
            for (const auto& bus : eventBuses | std::views::values)
            {
                if (bus->RunsOnMainThread())
                {
                    mainThreadBus = bus.get();
                    break;
                }
            }
        }

        if (mainThreadBus != nullptr)
        {
            logger->info("Running main thread event bus...");

            mainThreadBus->Run();

            logger->info("Main thread event bus exited, shutting down...");

            Shutdown();
        }
        else
        {
            logger->info("No main thread event bus, awaiting shutdown signal...");
            shutdownLatch.wait();
        }
    }

    void ModuleManager::Shutdown()
    {
        if (bool expected = true; !isRunning.compare_exchange_strong(expected, false))
        {
            logger->warn("ModuleManager is not running!");
            return;
        }

        logger->info("Shutting down ModuleManager...");

        for (const auto& bus : eventBuses | std::views::values)
            bus->Shutdown();

        for (const auto& typeIndex : moduleOrder)
            modules[typeIndex]->OnPrepareShutdown();

        logger->info("Shutting down {} module(s) in reverse order...", moduleOrder.size());

        for (auto it = moduleOrder.rbegin(); it != moduleOrder.rend(); ++it)
            modules[*it]->OnShutdown();

        shutdownLatch.count_down();

        logger->info("ModuleManager shut down successfully");
    }

    bool ModuleManager::IsRunning() const
    {
        return isRunning.load();
    }

    ModuleContext& ModuleManager::GetContext() const
    {
        return *context;
    }
}
