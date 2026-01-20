#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"
#include "RenderStar/Client/Render/Backend/RenderBackend.hpp"
#include <memory>
#include <cstdint>

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

    class RendererModule : public Common::Module::AbstractModule
    {
    public:

        RendererModule();

        ~RendererModule() override;

        void RenderFrame();

        IRenderBackend* GetBackend() const;

        RenderBackend GetBackendType() const;

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        void InitializeTestGeometry(Common::Module::ModuleContext& context);
        void RenderTestGeometry();

        std::unique_ptr<IRenderBackend> backend;
        RenderBackend backendType;

        std::unique_ptr<IShaderProgram> testShader;
        std::unique_ptr<Resource::Mesh> testMesh;
        std::unique_ptr<IUniformBindingHandle> uniformBinding;
        std::unique_ptr<IBufferHandle> uniformBuffer;

        bool testGeometryInitialized;
        float rotationAngle;
        float aspectRatio;
    };
}
