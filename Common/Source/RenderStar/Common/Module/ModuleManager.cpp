#include "RenderStar/Common/Module/ModuleManager.hpp"
#include "RenderStar/Common/Event/IEventBus.hpp"
#include "RenderStar/Common/Module/IModule.hpp"
#include <memory>
#include <ranges>

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

    void ModuleManager::Start()
    {
        if (bool expected = false; !isRunning.compare_exchange_strong(expected, true))
        {
            logger->warn("ModuleManager is already running!");
            return;
        }

        logger->info("Starting {} event bus(es)...", eventBuses.size());

        for (const auto& bus : eventBuses | std::views::values)
            bus->Start();

        logger->info("Registering {} top-level module(s)...", moduleOrder.size());

        for (const auto& typeIndex : moduleOrder)
            modules[typeIndex]->OnRegistration(*context);

        if (int32_t totalModules = context->GetTotalModuleCount(); totalModules > static_cast<int32_t>(modules.size()))
            logger->info("ModuleManager started successfully ({} modules total, including sub-modules)", totalModules);
        else
            logger->info("ModuleManager started successfully");
    }

    void ModuleManager::RunMainLoop()
    {
        Event::IEventBus* mainThreadBus = nullptr;

        for (const auto& bus : eventBuses | std::views::values)
        {
            if (bus->RunsOnMainThread())
            {
                mainThreadBus = bus.get();
                break;
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
