#include "RenderStar/Client/Core/ClientLifecycleModule.hpp"
#include "RenderStar/Client/Core/ClientWindowModule.hpp"
#include "RenderStar/Client/Render/RendererModule.hpp"
#include "RenderStar/Client/Event/Buses/ClientCoreEventBus.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Time/TimeModule.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"

namespace RenderStar::Client::Core
{
    void ClientLifecycleModule::OnInitialize(Common::Module::ModuleContext& moduleContext)
    {
        SetupMainLoop();
        logger->info("ClientLifecycleModule initialized");
    }

    void ClientLifecycleModule::SetupMainLoop()
    {
        const auto eventBus = context->GetEventBus<Event::ClientCoreEventBus>();

        if (!eventBus.has_value())
        {
            logger->error("ClientCoreEventBus not found");
            return;
        }

        logger->info("Setting up tick handler on ClientCoreEventBus");

        eventBus->get().SetTickHandler([this]()
        {
            auto windowModule = context->GetModule<ClientWindowModule>();
            auto timeModule = context->GetModule<Common::Time::TimeModule>();
            auto componentModule = context->GetModule<Common::Component::ComponentModule>();
            auto rendererModule = context->GetModule<Render::RendererModule>();

            if (!windowModule.has_value())
            {
                logger->error("WindowModule not found in tick handler");
                return;
            }

            static int frameCount = 0;
            if (frameCount++ % 60 == 0)
                logger->debug("Tick handler running, frame {}", frameCount);

            windowModule->get().Tick();

            if (windowModule->get().ShouldClose())
            {
                auto eventBus = context->GetEventBus<Event::ClientCoreEventBus>();

                if (eventBus.has_value())
                    eventBus->get().Shutdown();

                return;
            }

            if (timeModule.has_value())
                timeModule->get().Tick();

            if (componentModule.has_value())
                componentModule->get().RunSystems();

            if (rendererModule.has_value())
                rendererModule->get().RenderFrame();
            else
                logger->warn("RendererModule not found in tick handler");
        });
    }
}
