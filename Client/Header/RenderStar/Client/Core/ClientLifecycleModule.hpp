#pragma once

#include "RenderStar/Client/Render/Resource/IShaderProgram.hpp"
#include "RenderStar/Client/Render/Resource/IUniformBindingHandle.hpp"
#include "RenderStar/Client/Render/Resource/Mesh.hpp"
#include "RenderStar/Common/Module/AbstractModule.hpp"

namespace RenderStar::Client::Render
{
    class IRenderBackend;
}

namespace RenderStar::Common::Event
{
    struct EventResult;
}

namespace RenderStar::Client::Core
{
    class ClientLifecycleModule final : public Common::Module::AbstractModule
    {

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;
        void OnCleanup() override;

    private:

        Common::Event::EventResult OnRenderInitializeEvent(Common::Module::ModuleContext&, Render::IRenderBackend*);
        Common::Event::EventResult OnRenderFrameEvent(Render::IRenderBackend*);

        void SetupGameplayLogic(Common::Module::ModuleContext& context);
        void SetupMainLoop() const;

        std::unique_ptr<Render::IShaderProgram> testShader;
        std::unique_ptr<Render::Resource::Mesh> testMesh;
        std::unique_ptr<Render::IUniformBindingHandle> testUniformBinding;
        std::unique_ptr<Render::IBufferHandle> testUniformBuffer;

        float testRotationAngle = 0.0f;
        float testAspectRatio = 0.0f;
    };
}
