#include "RenderStar/Client/Core/ClientWindowModule.hpp"
#include "RenderStar/Client/Render/Backend/BackendFactory.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Configuration/ConfigurationModule.hpp"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace RenderStar::Client::Core
{
    ClientWindowModule::ClientWindowModule() : window(nullptr), title("RenderStar"), width(800), height(600), preferredBackend(Render::RenderBackend::VULKAN), forceBackend(false) { }

    ClientWindowModule::~ClientWindowModule()
    {
        DestroyWindow();
    }

    void ClientWindowModule::OnCleanup()
    {
        DestroyWindow();
    }

    void ClientWindowModule::Tick()
    {
        glfwPollEvents();
    }

    bool ClientWindowModule::ShouldClose() const
    {
        return window != nullptr && glfwWindowShouldClose(window);
    }

    void ClientWindowModule::Close()
    {
        if (window != nullptr)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    GLFWwindow* ClientWindowModule::GetWindowHandle() const
    {
        return window;
    }

    uint32_t ClientWindowModule::GetWidth() const
    {
        return width;
    }

    uint32_t ClientWindowModule::GetHeight() const
    {
        return height;
    }

    uint32_t ClientWindowModule::GetFramebufferWidth() const
    {
        int framebufferWidth = 0;
        int framebufferHeight = 0;
        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
        return framebufferWidth;
    }

    uint32_t ClientWindowModule::GetFramebufferHeight() const
    {
        int framebufferWidth = 0;
        int framebufferHeight = 0;
        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
        return framebufferHeight;
    }

    Render::RenderBackend ClientWindowModule::GetPreferredBackend() const
    {
        return preferredBackend;
    }

    void ClientWindowModule::OnInitialize(Common::Module::ModuleContext& moduleContext)
    {
        LoadConfiguration(moduleContext);

        if (!glfwInit())
        {
            logger->error("Failed to initialize GLFW");
            return;
        }

        CreateWindow();

        logger->info("ClientWindowModule initialized with {}x{} window", width, height);
    }

    void ClientWindowModule::CreateWindow()
    {
        if (preferredBackend == Render::RenderBackend::VULKAN)
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        else
        {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
        }

        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

        if (window == nullptr)
        {
            logger->error("Failed to create GLFW window");
            return;
        }

        if (preferredBackend == Render::RenderBackend::OPENGL)
            glfwMakeContextCurrent(window);
    }

    void ClientWindowModule::DestroyWindow()
    {
        if (window != nullptr)
        {
            glfwDestroyWindow(window);
            window = nullptr;
        }

        glfwTerminate();
    }

    void ClientWindowModule::LoadConfiguration(Common::Module::ModuleContext& moduleContext)
    {
        auto configModule = moduleContext.GetModule<Common::Configuration::ConfigurationModule>();

        if (!configModule.has_value())
        {
            logger->error("ConfigurationModule not found");
            return;
        }

        auto configOpt = configModule->get().For<ClientWindowModule>("render_star");

        if (!configOpt)
        {
            preferredBackend = Render::BackendFactory::DetectBestBackend();
            logger->info("No configuration found, using defaults with auto-detected backend: {}",
                preferredBackend == Render::RenderBackend::VULKAN ? "VULKAN" : "OPENGL");
            return;
        }

        auto& config = *configOpt;

        if (auto titleOptional = config->GetString("window_title"))
            title = *titleOptional;

        if (auto dimensionsOptional = config->GetStringList("window_dimensions"); dimensionsOptional.size() >= 2)
        {
            try
            {
                width = std::stoi(dimensionsOptional[0]);
                height = std::stoi(dimensionsOptional[1]);
            }
            catch (const std::exception&)
            {
                logger->warn("Invalid window_dimensions in configuration, using defaults");
            }
        }

        if (auto backendOptional = config->GetString("render_backend"))
        {
            std::string backend = *backendOptional;

            const size_t start = backend.find_first_not_of(" \t\r\n");

            if (const size_t end = backend.find_last_not_of(" \t\r\n"); start != std::string::npos && end != std::string::npos)
                backend = backend.substr(start, end - start + 1);

            logger->info("Configuration render_backend value: '{}'", backend);

            if (backend == "OPENGL" || backend == "opengl" || backend == "OpenGL")
                preferredBackend = Render::RenderBackend::OPENGL;
            else if (backend == "VULKAN" || backend == "vulkan" || backend == "Vulkan")
                preferredBackend = Render::RenderBackend::VULKAN;
        }
        else
        {
            preferredBackend = Render::BackendFactory::DetectBestBackend();
            logger->info("No render_backend in configuration, auto-detected: {}",
                preferredBackend == Render::RenderBackend::VULKAN ? "VULKAN" : "OPENGL");
        }

        if (auto forceOptional = config->GetBoolean("force_render_backend"))
            forceBackend = *forceOptional;

        logger->debug("Loaded configuration: title='{}', dimensions={}x{}, backend={}, force={}", title, width, height, preferredBackend == Render::RenderBackend::VULKAN ? "VULKAN" : "OPENGL", forceBackend);
    }
}
