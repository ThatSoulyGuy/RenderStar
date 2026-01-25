#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"
#include "RenderStar/Client/Render/Backend/RenderBackend.hpp"
#include <string>

struct GLFWwindow;

namespace RenderStar::Client::Core
{
    class ClientWindowModule final : public Common::Module::AbstractModule
    {
    public:

        ClientWindowModule();

        ~ClientWindowModule() override;

        void Tick();

        bool ShouldClose() const;

        void Close();

        GLFWwindow* GetWindowHandle() const;

        uint32_t GetWidth() const;

        uint32_t GetHeight() const;

        uint32_t GetFramebufferWidth() const;

        uint32_t GetFramebufferHeight() const;

        Render::RenderBackend GetPreferredBackend() const;

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        void CreateWindow();

        void DestroyWindow();

        void LoadConfiguration(Common::Module::ModuleContext& context);

        GLFWwindow* window;
        std::string title;
        int32_t width;
        int32_t height;
        Render::RenderBackend preferredBackend;
        bool forceBackend;
    };
}
