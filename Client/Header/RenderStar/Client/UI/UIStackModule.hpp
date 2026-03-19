#pragma once

#include "RenderStar/Client/UI/UibinScene.hpp"
#include "RenderStar/Client/UI/FontAtlas.hpp"
#include "RenderStar/Common/Module/AbstractModule.hpp"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace RenderStar::Client::Render
{
    class IRenderBackend;
    class IBufferManager;
    class IBufferHandle;
    class IUniformManager;
    class IUniformBindingHandle;
    class ITextureManager;
    class ITextureHandle;
    class IShaderProgram;

    namespace Resource { class Mesh; }
}

namespace RenderStar::Client::Input
{
    class ClientInputModule;
}

namespace RenderStar::Client::UI
{

    class UIStackModule final : public Common::Module::AbstractModule
    {
    public:

        void PushLayer(UibinScene scene);
        void PopLayer();
        void ClearLayers();

        UIElement* FindById(const std::string& id);
        UIElement* FindByName(const std::string& name);

        size_t GetLayerCount() const { return layerStack.size(); }

        void Render(Render::IRenderBackend* backend);

        void SetupRenderState(Render::IBufferManager* bm, Render::IUniformManager* um, Render::ITextureManager* tm);
        void SetShader(std::unique_ptr<Render::IShaderProgram> shader);
        void SetDefaultFont(std::vector<uint8_t> fontData);
        void Cleanup();

        void ProcessInput(Input::ClientInputModule* input);

        void OnClick(const std::string& elementId, std::function<void()> callback);
        void OnToggle(const std::string& elementId, std::function<void(bool)> callback);
        void OnTextChanged(const std::string& elementId, std::function<void(const std::string&)> callback);

        bool IsInputConsumed() const { return inputConsumed; }

        [[nodiscard]]
        std::vector<std::type_index> GetDependencies() const override;

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        struct UniformSlot
        {
            std::vector<std::unique_ptr<Render::IBufferHandle>> buffers; // one per frame-in-flight
            std::unique_ptr<Render::IUniformBindingHandle> binding;
        };

        struct UILayer
        {
            UibinScene scene;
            std::unordered_map<std::string, std::unique_ptr<Render::ITextureHandle>> textures;
        };

        struct ComputedRect
        {
            float x, y, w, h;
        };

        void RenderElement(const UIElement& element, Render::IRenderBackend* backend,
            float parentX, float parentY, float parentW, float parentH);

        void ComputeRect(const TransformData& t,
            float parentX, float parentY, float parentW, float parentH,
            float& outX, float& outY, float& outW, float& outH) const;

        void DrawQuad(Render::IRenderBackend* backend,
            float x, float y, float w, float h,
            float u0, float v0, float u1, float v1,
            const UibinColor& tint, Render::ITextureHandle* texture,
            float rotation = 0.0f, float cornerRadius = 0.0f);

        void DrawNineSlice(Render::IRenderBackend* backend,
            float x, float y, float w, float h,
            int32_t sliceL, int32_t sliceT, int32_t sliceR, int32_t sliceB,
            Render::ITextureHandle* texture, const UibinColor& tint, float cornerRadius = 0.0f);

        Render::ITextureHandle* GetOrUploadTexture(const std::string& assetKey, UILayer& layer);

        void RenderText(Render::IRenderBackend* backend,
            const TextData& textData, float x, float y, float w, float h,
            UILayer& layer);

        UIElement* HitTest(double cursorX, double cursorY);
        UIElement* HitTestElement(UIElement& element, double cursorX, double cursorY);

        UniformSlot& AcquireUniformSlot();

        void DrawArcSlice(Render::IRenderBackend* backend,
            float cx, float cy, float outerR, Render::Resource::Mesh* arcMesh,
            const UibinColor& tint);

        struct ArcMeshSet
        {
            std::vector<std::unique_ptr<Render::Resource::Mesh>> slices;
            std::unique_ptr<Render::Resource::Mesh> outerRing;
            std::unique_ptr<Render::Resource::Mesh> innerRing;
            int sliceCount = 0;
            float radiusRatio = 0.0f;
        };

        ArcMeshSet& GetOrCreateArcMeshes(int sliceCount, float innerR, float outerR);

        std::unique_ptr<Render::Resource::Mesh> quadMesh;
        std::unique_ptr<Render::ITextureHandle> whiteTexture;
        std::unique_ptr<Render::IShaderProgram> shader;

        std::vector<UniformSlot> uniformPool;
        size_t uniformPoolIndex = 0;

        std::vector<UILayer> layerStack;
        UILayer* currentRenderLayer = nullptr;

        std::unordered_map<std::string, std::unique_ptr<FontAtlas>> fontCache;

        Render::IBufferManager* bufferManager = nullptr;
        Render::IUniformManager* uniformManager = nullptr;
        Render::ITextureManager* textureManager = nullptr;

        static constexpr float DEFAULT_DESIGN_WIDTH = 1920.0f;
        static constexpr float DEFAULT_DESIGN_HEIGHT = 1080.0f;

        float screenW = 1280.0f;
        float screenH = 720.0f;
        float scaleX = 1.0f;
        float scaleY = 1.0f;

        // Input state
        std::unordered_map<std::string, ComputedRect> computedRects;
        std::unordered_map<std::string, std::function<void()>> clickHandlers;
        std::unordered_map<std::string, std::function<void(bool)>> toggleHandlers;
        std::unordered_map<std::string, std::function<void(const std::string&)>> textChangedHandlers;
        std::string focusedElementId;
        std::string hoveredElementId;
        std::string expandedDropdownId;
        bool inputConsumed = false;

        // Scroll state
        std::unordered_map<std::string, float> scrollOffsets;
        std::unordered_map<std::string, float> scrollContentSizes;
        std::unordered_map<std::string, ComputedRect> scrollbarThumbRects;

        // Scrollbar drag state
        std::string draggingScrollbarId;
        float dragStartOffset = 0.0f;
        float dragStartMouse = 0.0f;
        float dragTrackSize = 0.0f;
        float dragContentSize = 0.0f;

        // Default fallback font (loaded from disk, used when fontPath is empty)
        std::vector<uint8_t> defaultFontData;

        // Frame-in-flight tracking for per-frame uniform buffers
        int32_t currentFrameIndex = 0;
        int32_t maxFramesInFlight = 2;

        // Cached arc meshes for radial menus (keyed by element id)
        std::unordered_map<std::string, ArcMeshSet> arcMeshCache;
    };
}
