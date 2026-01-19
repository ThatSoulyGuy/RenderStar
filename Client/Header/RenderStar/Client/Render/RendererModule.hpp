#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"
#include "RenderStar/Client/Render/Backend/RenderBackend.hpp"
#include <memory>
#include <cstdint>

namespace RenderStar::Client::Render
{
    class IRenderBackend;
    class IShaderProgram;
    class IMesh;
    class IUniformBindingHandle;
    class IBufferHandle;

    namespace OpenGL
    {
        class OpenGLShaderProgram;
        class OpenGLMesh;
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

        void InitializeOpenGLTestGeometry();
        void RenderOpenGLTestGeometry();

        void InitializeVulkanTestGeometry();
        void RenderVulkanTestGeometry();

        std::unique_ptr<IRenderBackend> backend;
        RenderBackend backendType;

        std::unique_ptr<OpenGL::OpenGLShaderProgram> openglTestShader;
        std::unique_ptr<OpenGL::OpenGLMesh> openglTestMesh;
        uint32_t testVertexBuffer;
        uint32_t testIndexBuffer;

        std::unique_ptr<IShaderProgram> vulkanTestShader;
        std::unique_ptr<IMesh> vulkanTestMesh;
        std::unique_ptr<IUniformBindingHandle> vulkanUniformBinding;
        std::unique_ptr<IBufferHandle> vulkanUniformBuffer;

        bool testGeometryInitialized;
        float rotationAngle;
        float aspectRatio;
    };
}
