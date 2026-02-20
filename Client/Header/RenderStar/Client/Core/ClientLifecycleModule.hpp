#pragma once

#include "RenderStar/Client/Render/Resource/IShaderProgram.hpp"
#include "RenderStar/Client/Render/Resource/IUniformBindingHandle.hpp"
#include "RenderStar/Client/Render/Resource/Mesh.hpp"
#include "RenderStar/Common/Component/GameObject.hpp"
#include "RenderStar/Common/Module/AbstractModule.hpp"
#include <memory>
#include <vector>

namespace RenderStar::Client::Render
{
    class IRenderBackend;
    class IBufferManager;
    class IUniformManager;
}

namespace RenderStar::Common::Event
{
    struct EventResult;
}

namespace RenderStar::Client::Core
{
    struct UniformSlot
    {
        std::unique_ptr<Render::IBufferHandle> buffer;
        std::unique_ptr<Render::IUniformBindingHandle> binding;
    };

    class ClientLifecycleModule final : public Common::Module::AbstractModule
    {

    public:

        [[nodiscard]]
        std::vector<std::type_index> GetDependencies() const override;

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;
        void OnCleanup() override;

    private:

        Common::Event::EventResult OnRenderInitializeEvent(Common::Module::ModuleContext&, Render::IRenderBackend*);
        Common::Event::EventResult OnRenderFrameEvent(Render::IRenderBackend*);

        UniformSlot& AcquireUniformSlot();
        void BuildSceneMeshes();

        void SetupGameplayLogic(Common::Module::ModuleContext& context);
        void SetupMainLoop() const;

        std::unique_ptr<Render::IShaderProgram> testShader;
        std::unique_ptr<Render::Resource::Mesh> testMesh;

        std::vector<UniformSlot> uniformPool;
        size_t uniformPoolIndex = 0;
        Render::IBufferManager* cachedBufferManager = nullptr;
        Render::IUniformManager* cachedUniformManager = nullptr;

        float testRotationAngle = 0.0f;

        std::vector<std::unique_ptr<Render::Resource::Mesh>> sceneMeshes;

        Common::Component::GameObject playerEntity = Common::Component::GameObject::Invalid();
    };
}
