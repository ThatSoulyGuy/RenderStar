#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"
#include "RenderStar/Client/Render/Backend/RenderBackend.hpp"
#include <memory>

namespace RenderStar::Client::Render
{
    class IRenderBackend;
    class IShaderProgram;
    class IUniformBindingHandle;
    class IBufferHandle;

    namespace Resource
    {
        class Mesh;
    }

    class RendererModule final : public Common::Module::AbstractModule
    {

    public:

        RendererModule();

        ~RendererModule() override;

        [[nodiscard]]
        IRenderBackend* GetBackend() const;

        [[nodiscard]]
        RenderBackend GetBackendType() const;

        [[nodiscard]]
        std::vector<std::type_index> GetDependencies() const override;

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;
        void OnPreCleanup() override;
        void OnCleanup() override;

    private:

        std::unique_ptr<IRenderBackend> backend;
        RenderBackend backendType;
    };
}
