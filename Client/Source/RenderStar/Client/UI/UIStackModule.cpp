#include "RenderStar/Client/UI/UIStackModule.hpp"
#include "RenderStar/Client/UI/UIVertex.hpp"
#include "RenderStar/Client/UI/UIUniformData.hpp"
#include "RenderStar/Client/UI/FontAtlas.hpp"
#include "RenderStar/Client/Input/ClientInputModule.hpp"
#include "RenderStar/Client/Render/Backend/IRenderBackend.hpp"
#include "RenderStar/Client/Render/Resource/IBufferManager.hpp"
#include "RenderStar/Client/Render/Resource/IShaderProgram.hpp"
#include "RenderStar/Client/Render/Resource/ITextureManager.hpp"
#include "RenderStar/Client/Render/Resource/IUniformManager.hpp"
#include "RenderStar/Client/Render/Resource/Mesh.hpp"
#include "RenderStar/Client/Render/RendererModule.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#include <stb_image.h>

#include <algorithm>
#include <cmath>
#include <sstream>

namespace RenderStar::Client::UI
{
    void UIStackModule::OnInitialize(Common::Module::ModuleContext&)
    {
        logger->info("UIStackModule initialized");
    }

    std::vector<std::type_index> UIStackModule::GetDependencies() const
    {
        return DependsOn<Render::RendererModule>();
    }

    void UIStackModule::SetupRenderState(Render::IBufferManager* bm, Render::IUniformManager* um, Render::ITextureManager* tm)
    {
        bufferManager = bm;
        uniformManager = um;
        textureManager = tm;

        // Build unit quad mesh: (0,0)-(1,1)
        std::vector<UIVertex> vertices = {
            {0.0f, 0.0f, 0.0f, 0.0f},
            {1.0f, 0.0f, 1.0f, 0.0f},
            {1.0f, 1.0f, 1.0f, 1.0f},
            {0.0f, 1.0f, 0.0f, 1.0f}
        };

        std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};

        quadMesh = std::make_unique<Render::Resource::Mesh>(*bufferManager, UIVertex::LAYOUT, Render::PrimitiveType::TRIANGLES);
        quadMesh->SetVertices(vertices);
        quadMesh->SetIndices(indices);

        // Build 1x1 white texture
        uint8_t whitePixels[] = {255, 255, 255, 255};
        Render::TextureDescription whiteDesc;
        whiteDesc.width = 1;
        whiteDesc.height = 1;
        whiteDesc.format = Render::TextureFormat::RGBA8;
        whiteDesc.wrapS = Render::TextureWrapMode::CLAMP_TO_EDGE;
        whiteDesc.wrapT = Render::TextureWrapMode::CLAMP_TO_EDGE;
        whiteDesc.minFilter = Render::TextureFilterMode::NEAREST;
        whiteDesc.magFilter = Render::TextureFilterMode::NEAREST;
        whiteTexture = textureManager->CreateFromMemory(whiteDesc, whitePixels);

        logger->info("UIStackModule render state set up");
    }

    void UIStackModule::SetShader(std::unique_ptr<Render::IShaderProgram> s)
    {
        shader = std::move(s);
    }

    void UIStackModule::SetDefaultFont(std::vector<uint8_t> fontData)
    {
        defaultFontData = std::move(fontData);
        logger->info("Default font set ({} bytes)", defaultFontData.size());
    }

    void UIStackModule::Cleanup()
    {
        uniformPool.clear();
        uniformPoolIndex = 0;
        quadMesh.reset();
        whiteTexture.reset();
        shader.reset();
        layerStack.clear();
        fontCache.clear();
        currentRenderLayer = nullptr;
        bufferManager = nullptr;
        uniformManager = nullptr;
        textureManager = nullptr;
        computedRects.clear();
        clickHandlers.clear();
        toggleHandlers.clear();
        textChangedHandlers.clear();
        scrollOffsets.clear();
        arcMeshCache.clear();
        focusedElementId.clear();
        hoveredElementId.clear();
        inputConsumed = false;
    }

    void UIStackModule::PushLayer(UibinScene scene)
    {
        UILayer layer;
        layer.scene = std::move(scene);
        layerStack.push_back(std::move(layer));
        logger->info("UI layer pushed, stack size={}", layerStack.size());
    }

    void UIStackModule::PopLayer()
    {
        if (!layerStack.empty())
        {
            layerStack.pop_back();
            logger->info("UI layer popped, stack size={}", layerStack.size());
        }
    }

    void UIStackModule::ClearLayers()
    {
        layerStack.clear();
        computedRects.clear();
        scrollOffsets.clear();
    }

    UIElement* UIStackModule::FindById(const std::string& id)
    {
        for (auto it = layerStack.rbegin(); it != layerStack.rend(); ++it)
        {
            if (auto* found = it->scene.FindById(id))
                return found;
        }
        return nullptr;
    }

    UIElement* UIStackModule::FindByName(const std::string& name)
    {
        for (auto it = layerStack.rbegin(); it != layerStack.rend(); ++it)
        {
            if (auto* found = it->scene.FindByName(name))
                return found;
        }
        return nullptr;
    }

    // ─── Callback Registration ───────────────────────────────────────

    void UIStackModule::OnClick(const std::string& elementId, std::function<void()> callback)
    {
        clickHandlers[elementId] = std::move(callback);
    }

    void UIStackModule::OnToggle(const std::string& elementId, std::function<void(bool)> callback)
    {
        toggleHandlers[elementId] = std::move(callback);
    }

    void UIStackModule::OnTextChanged(const std::string& elementId, std::function<void(const std::string&)> callback)
    {
        textChangedHandlers[elementId] = std::move(callback);
    }

    // ─── Input Processing ────────────────────────────────────────────

    void UIStackModule::ProcessInput(Input::ClientInputModule* input)
    {
        if (!input)
            return;

        inputConsumed = false;

        glm::dvec2 cursor = input->GetCursorPosition();
        UIElement* hitElement = HitTest(cursor.x, cursor.y);

        hoveredElementId = hitElement && !hitElement->id.empty() ? hitElement->id : "";

        if (hitElement)
            inputConsumed = true;

        // Expanded dropdown option list also consumes input
        if (!expandedDropdownId.empty() && !inputConsumed)
        {
            auto rectIt = computedRects.find(expandedDropdownId);
            if (rectIt != computedRects.end())
            {
                auto& r = rectIt->second;
                if (cursor.x >= r.x && cursor.x <= r.x + r.w && cursor.y >= r.y)
                    inputConsumed = true;
            }
        }

        // Scrollbar drag handling
        if (!draggingScrollbarId.empty())
        {
            if (input->IsMouseButtonHeld(Input::MouseCode::LEFT))
            {
                float mousePos = cursor.y;
                // Determine direction from scrollContentSizes
                // For simplicity, check if the scrollbar thumb rect is wider than tall (horizontal)
                auto thumbIt = scrollbarThumbRects.find(draggingScrollbarId);
                if (thumbIt != scrollbarThumbRects.end() && thumbIt->second.w > thumbIt->second.h)
                    mousePos = static_cast<float>(cursor.x);
                else
                    mousePos = static_cast<float>(cursor.y);

                float mouseDelta = mousePos - dragStartMouse;
                float scrollDelta = (dragTrackSize > 0.0f) ? mouseDelta * (dragContentSize / dragTrackSize) : 0.0f;

                auto cIt = scrollContentSizes.find(draggingScrollbarId);
                auto rIt = computedRects.find(draggingScrollbarId);
                float maxScroll = 0.0f;
                if (cIt != scrollContentSizes.end() && rIt != computedRects.end())
                {
                    float visibleSize = (thumbIt != scrollbarThumbRects.end() && thumbIt->second.w > thumbIt->second.h)
                        ? rIt->second.w : rIt->second.h;
                    maxScroll = std::max(0.0f, cIt->second - visibleSize);
                }

                scrollOffsets[draggingScrollbarId] = std::clamp(dragStartOffset + scrollDelta, 0.0f, maxScroll);
                inputConsumed = true;
            }
            else
            {
                draggingScrollbarId.clear();
            }
        }

        // Check if clicking on a scrollbar thumb to start drag
        if (input->IsMouseButtonPressed(Input::MouseCode::LEFT) && draggingScrollbarId.empty())
        {
            for (const auto& [id, thumbRect] : scrollbarThumbRects)
            {
                if (cursor.x >= thumbRect.x && cursor.x <= thumbRect.x + thumbRect.w &&
                    cursor.y >= thumbRect.y && cursor.y <= thumbRect.y + thumbRect.h)
                {
                    draggingScrollbarId = id;
                    dragStartOffset = scrollOffsets.count(id) ? scrollOffsets[id] : 0.0f;

                    bool isHorizontal = (thumbRect.w > thumbRect.h);
                    dragStartMouse = isHorizontal ? static_cast<float>(cursor.x) : static_cast<float>(cursor.y);

                    auto rIt = computedRects.find(id);
                    float trackPad = 2.0f * std::min(scaleX, scaleY);
                    if (rIt != computedRects.end())
                        dragTrackSize = isHorizontal
                            ? rIt->second.w - 2 * trackPad
                            : rIt->second.h - 2 * trackPad;
                    else
                        dragTrackSize = 1.0f;

                    auto cIt = scrollContentSizes.find(id);
                    dragContentSize = (cIt != scrollContentSizes.end()) ? cIt->second : 1.0f;
                    inputConsumed = true;
                    break;
                }
            }
        }

        // Click handling
        if (input->IsMouseButtonPressed(Input::MouseCode::LEFT))
        {
            // Check if clicking on an expanded dropdown's option list
            bool handledDropdownOption = false;
            if (!expandedDropdownId.empty())
            {
                // Find the expanded dropdown element
                UIElement* ddElement = nullptr;
                for (auto& layer : layerStack)
                    if (auto* found = layer.scene.root.FindById(expandedDropdownId))
                    { ddElement = found; break; }

                if (ddElement)
                {
                    auto* dropdown = ddElement->GetDropdown();
                    auto rectIt = computedRects.find(expandedDropdownId);
                    if (dropdown && rectIt != computedRects.end())
                    {
                        auto& r = rectIt->second;
                        float optionH = r.h;

                        // Parse options
                        std::vector<std::string> opts;
                        std::istringstream stream(dropdown->options);
                        std::string token;
                        while (std::getline(stream, token, ','))
                            opts.push_back(token);

                        // Check if cursor is in the expanded option list area
                        float listTop = r.y + r.h;
                        float listBottom = listTop + optionH * static_cast<float>(opts.size());
                        if (cursor.x >= r.x && cursor.x <= r.x + r.w &&
                            cursor.y >= listTop && cursor.y <= listBottom)
                        {
                            int clickedIdx = static_cast<int>((cursor.y - listTop) / optionH);
                            if (clickedIdx >= 0 && clickedIdx < static_cast<int>(opts.size()))
                            {
                                dropdown->selectedIndex = clickedIdx;
                                handledDropdownOption = true;
                            }
                        }
                        expandedDropdownId.clear();
                    }
                }
                else
                    expandedDropdownId.clear();
            }

            if (hitElement && !handledDropdownOption)
            {
                // Button click
                if (hitElement->GetButton())
                {
                    auto it = clickHandlers.find(hitElement->id);
                    if (it != clickHandlers.end())
                        it->second();
                }

                // Toggle
                if (auto* toggle = hitElement->GetToggle())
                {
                    toggle->checked = !toggle->checked;
                    auto it = toggleHandlers.find(hitElement->id);
                    if (it != toggleHandlers.end())
                        it->second(toggle->checked);
                }

                // Dropdown expand/collapse
                if (hitElement->GetDropdown())
                {
                    if (expandedDropdownId == hitElement->id)
                        expandedDropdownId.clear();
                    else
                        expandedDropdownId = hitElement->id;
                }
                else
                    expandedDropdownId.clear();

                // TextInput focus
                if (hitElement->GetTextInput())
                    focusedElementId = hitElement->id;
                else
                    focusedElementId.clear();
            }
            else if (!hitElement && !handledDropdownOption)
            {
                focusedElementId.clear();
                expandedDropdownId.clear();
            }
        }

        // Scroll wheel for ScrollBox
        glm::dvec2 scrollDelta = input->GetScrollDelta();
        if ((scrollDelta.x != 0.0 || scrollDelta.y != 0.0) && hitElement)
        {
            // Find the nearest ScrollBox ancestor (or self)
            for (auto it = layerStack.rbegin(); it != layerStack.rend(); ++it)
            {
                // Check if the hovered element or any ancestor has a ScrollBox
                UIElement* scrollElement = nullptr;
                std::function<bool(UIElement&)> findScrollParent = [&](UIElement& el) -> bool
                {
                    if (el.GetScrollBox())
                    {
                        // Check if cursor is inside this element's rect
                        auto rIt = computedRects.find(el.id);
                        if (rIt != computedRects.end())
                        {
                            auto& r = rIt->second;
                            if (cursor.x >= r.x && cursor.x <= r.x + r.w &&
                                cursor.y >= r.y && cursor.y <= r.y + r.h)
                            {
                                scrollElement = &el;
                                return true;
                            }
                        }
                    }
                    for (auto& child : el.children)
                    {
                        if (findScrollParent(child))
                            return true;
                    }
                    return false;
                };
                findScrollParent(it->scene.root);

                if (scrollElement && !scrollElement->id.empty())
                {
                    auto* sb = scrollElement->GetScrollBox();
                    bool isHoriz = (sb && sb->direction == 1);

                    // For vertical scrollboxes, use Y wheel; for horizontal, use X wheel (or Y if no X)
                    float wheelDelta = isHoriz
                        ? (scrollDelta.x != 0.0 ? static_cast<float>(scrollDelta.x) : static_cast<float>(scrollDelta.y))
                        : static_cast<float>(scrollDelta.y);

                    scrollOffsets[scrollElement->id] -= wheelDelta * 30.0f;

                    float maxScroll = 0.0f;
                    auto cIt = scrollContentSizes.find(scrollElement->id);
                    auto rIt = computedRects.find(scrollElement->id);
                    if (cIt != scrollContentSizes.end() && rIt != computedRects.end())
                    {
                        float visibleSize = isHoriz ? rIt->second.w : rIt->second.h;
                        maxScroll = std::max(0.0f, cIt->second - visibleSize);
                    }
                    scrollOffsets[scrollElement->id] = std::clamp(scrollOffsets[scrollElement->id], 0.0f, maxScroll);
                    break;
                }
            }
        }

        // Keyboard for TextInput
        if (!focusedElementId.empty())
        {
            UIElement* focusedElement = FindById(focusedElementId);
            if (focusedElement)
            {
                auto* textInput = focusedElement->GetTextInput();
                if (textInput)
                {
                    bool changed = false;

                    if (input->IsKeyPressed(Input::KeyCode::BACKSPACE) && !textInput->text.empty())
                    {
                        textInput->text.pop_back();
                        changed = true;
                    }

                    if (input->IsKeyPressed(Input::KeyCode::ENTER) || input->IsKeyPressed(Input::KeyCode::ESCAPE))
                    {
                        focusedElementId.clear();
                    }

                    // Printable characters
                    static const Input::KeyCode printableKeys[] = {
                        Input::KeyCode::A, Input::KeyCode::B, Input::KeyCode::C, Input::KeyCode::D,
                        Input::KeyCode::E, Input::KeyCode::F, Input::KeyCode::G, Input::KeyCode::H,
                        Input::KeyCode::I, Input::KeyCode::J, Input::KeyCode::K, Input::KeyCode::L,
                        Input::KeyCode::M, Input::KeyCode::N, Input::KeyCode::O, Input::KeyCode::P,
                        Input::KeyCode::Q, Input::KeyCode::R, Input::KeyCode::S, Input::KeyCode::T,
                        Input::KeyCode::U, Input::KeyCode::V, Input::KeyCode::W, Input::KeyCode::X,
                        Input::KeyCode::Y, Input::KeyCode::Z,
                        Input::KeyCode::NUM_0, Input::KeyCode::NUM_1, Input::KeyCode::NUM_2,
                        Input::KeyCode::NUM_3, Input::KeyCode::NUM_4, Input::KeyCode::NUM_5,
                        Input::KeyCode::NUM_6, Input::KeyCode::NUM_7, Input::KeyCode::NUM_8,
                        Input::KeyCode::NUM_9,
                        Input::KeyCode::SPACE, Input::KeyCode::PERIOD, Input::KeyCode::COMMA,
                        Input::KeyCode::MINUS, Input::KeyCode::EQUAL, Input::KeyCode::SLASH,
                        Input::KeyCode::SEMICOLON, Input::KeyCode::APOSTROPHE
                    };

                    bool shift = input->IsKeyHeld(Input::KeyCode::LEFT_SHIFT) || input->IsKeyHeld(Input::KeyCode::RIGHT_SHIFT);

                    for (auto key : printableKeys)
                    {
                        if (input->IsKeyPressed(key))
                        {
                            char c = static_cast<char>(static_cast<int32_t>(key));
                            if (key >= Input::KeyCode::A && key <= Input::KeyCode::Z)
                                c = shift ? c : static_cast<char>(c + 32);
                            else if (shift)
                            {
                                // Shift variants for common keys
                                switch (key)
                                {
                                    case Input::KeyCode::NUM_1: c = '!'; break;
                                    case Input::KeyCode::NUM_2: c = '@'; break;
                                    case Input::KeyCode::NUM_3: c = '#'; break;
                                    case Input::KeyCode::NUM_4: c = '$'; break;
                                    case Input::KeyCode::NUM_5: c = '%'; break;
                                    case Input::KeyCode::NUM_6: c = '^'; break;
                                    case Input::KeyCode::NUM_7: c = '&'; break;
                                    case Input::KeyCode::NUM_8: c = '*'; break;
                                    case Input::KeyCode::NUM_9: c = '('; break;
                                    case Input::KeyCode::NUM_0: c = ')'; break;
                                    case Input::KeyCode::MINUS: c = '_'; break;
                                    case Input::KeyCode::EQUAL: c = '+'; break;
                                    case Input::KeyCode::SEMICOLON: c = ':'; break;
                                    case Input::KeyCode::APOSTROPHE: c = '"'; break;
                                    case Input::KeyCode::COMMA: c = '<'; break;
                                    case Input::KeyCode::PERIOD: c = '>'; break;
                                    case Input::KeyCode::SLASH: c = '?'; break;
                                    default: break;
                                }
                            }
                            textInput->text += c;
                            changed = true;
                        }
                    }

                    if (changed)
                    {
                        auto it = textChangedHandlers.find(focusedElementId);
                        if (it != textChangedHandlers.end())
                            it->second(textInput->text);
                    }

                    inputConsumed = true;
                }
            }
        }
    }

    UIElement* UIStackModule::HitTest(double cursorX, double cursorY)
    {
        // Traverse layers top-to-bottom (last layer = topmost)
        for (auto it = layerStack.rbegin(); it != layerStack.rend(); ++it)
        {
            UIElement* hit = HitTestElement(it->scene.root, cursorX, cursorY);
            if (hit) return hit;
        }
        return nullptr;
    }

    UIElement* UIStackModule::HitTestElement(UIElement& element, double cursorX, double cursorY)
    {
        // Check children in reverse order (last child = rendered on top)
        for (auto it = element.children.rbegin(); it != element.children.rend(); ++it)
        {
            UIElement* hit = HitTestElement(*it, cursorX, cursorY);
            if (hit) return hit;
        }

        // Check this element if it has an interactive component
        if (!element.id.empty() &&
            (element.GetButton() || element.GetToggle() || element.GetTextInput() || element.GetDropdown() || element.GetScrollBox()))
        {
            auto rIt = computedRects.find(element.id);
            if (rIt != computedRects.end())
            {
                auto& r = rIt->second;
                if (cursorX >= r.x && cursorX <= r.x + r.w &&
                    cursorY >= r.y && cursorY <= r.y + r.h)
                {
                    return &element;
                }
            }
        }

        return nullptr;
    }

    // ─── Rendering ───────────────────────────────────────────────────

    void UIStackModule::Render(Render::IRenderBackend* backend)
    {
        if (!backend || !shader || !shader->IsValid() || !quadMesh || !quadMesh->IsValid())
            return;

        if (layerStack.empty())
            return;

        screenW = static_cast<float>(backend->GetWidth());
        screenH = static_cast<float>(backend->GetHeight());
        currentFrameIndex = backend->GetCurrentFrame();
        maxFramesInFlight = backend->GetMaxFramesInFlight();
        uniformPoolIndex = 0;
        computedRects.clear();
        scrollbarThumbRects.clear();

        for (auto& layer : layerStack)
        {
            currentRenderLayer = &layer;

            float designW = layer.scene.designWidth > 0.0
                ? static_cast<float>(layer.scene.designWidth) : DEFAULT_DESIGN_WIDTH;
            float designH = layer.scene.designHeight > 0.0
                ? static_cast<float>(layer.scene.designHeight) : DEFAULT_DESIGN_HEIGHT;
            scaleX = screenW / designW;
            scaleY = screenH / designH;

            RenderElement(layer.scene.root, backend, 0.0f, 0.0f, screenW, screenH);
        }

        backend->BeginOverlayPass();
        backend->ExecuteDrawCommands();
        backend->EndOverlayPass();
        currentRenderLayer = nullptr;
    }

    void UIStackModule::RenderElement(const UIElement& element, Render::IRenderBackend* backend,
        float parentX, float parentY, float parentW, float parentH)
    {
        const TransformData* t = element.GetTransform();

        float ex, ey, ew, eh;

        if (t)
        {
            ComputeRect(*t, parentX, parentY, parentW, parentH, ex, ey, ew, eh);
        }
        else
        {
            ex = parentX;
            ey = parentY;
            ew = parentW;
            eh = parentH;
        }

        float rotation = t ? static_cast<float>(t->rotationDegrees) : 0.0f;

        // Cache computed rect for hit-testing
        if (!element.id.empty())
            computedRects[element.id] = {ex, ey, ew, eh};

        float uniformScale = std::min(scaleX, scaleY);

        // ── Panel ──
        if (const auto* panel = element.GetPanel())
        {
            float cr = static_cast<float>(panel->cornerRadius) * uniformScale;
            if (panel->borderWidth > 0)
            {
                float bw = static_cast<float>(panel->borderWidth) * uniformScale;
                DrawQuad(backend, ex, ey, ew, eh, 0, 0, 1, 1, panel->borderColor, whiteTexture.get(), rotation, cr);
                DrawQuad(backend, ex + bw, ey + bw, ew - 2 * bw, eh - 2 * bw, 0, 0, 1, 1,
                    panel->backgroundColor, whiteTexture.get(), rotation, std::max(0.0f, cr - bw));
            }
            else
            {
                DrawQuad(backend, ex, ey, ew, eh, 0, 0, 1, 1, panel->backgroundColor, whiteTexture.get(), rotation, cr);
            }
        }

        // ── Image ──
        if (const auto* img = element.GetImage())
        {
            if (!img->imagePath.empty() && currentRenderLayer)
            {
                auto* tex = GetOrUploadTexture(img->imagePath, *currentRenderLayer);
                if (tex)
                    DrawQuad(backend, ex, ey, ew, eh, 0, 0, 1, 1, img->tint, tex, rotation);
            }
        }

        // ── Button ──
        if (const auto* btn = element.GetButton())
        {
            float btnCR = 6.0f * uniformScale;

            if (!btn->imagePath.empty() && currentRenderLayer)
            {
                auto* tex = GetOrUploadTexture(btn->imagePath, *currentRenderLayer);
                if (tex)
                {
                    bool hasSlice = btn->sliceLeft > 0 || btn->sliceTop > 0 || btn->sliceRight > 0 || btn->sliceBottom > 0;
                    if (hasSlice)
                        DrawNineSlice(backend, ex, ey, ew, eh, btn->sliceLeft, btn->sliceTop, btn->sliceRight, btn->sliceBottom, tex, {1, 1, 1, 1}, btnCR);
                    else
                        DrawQuad(backend, ex, ey, ew, eh, 0, 0, 1, 1, {1, 1, 1, 1}, tex, rotation, btnCR);
                }
                else
                {
                    // Procedural rounded button with edge highlight
                    UibinColor highlight = {
                        std::min(1.0f, btn->backgroundColor.r * 1.5f + 0.08f),
                        std::min(1.0f, btn->backgroundColor.g * 1.5f + 0.08f),
                        std::min(1.0f, btn->backgroundColor.b * 1.5f + 0.08f),
                        btn->backgroundColor.a };
                    float edgePx = 1.5f * uniformScale;
                    DrawQuad(backend, ex, ey, ew, eh, 0, 0, 1, 1, highlight, whiteTexture.get(), rotation, btnCR);
                    DrawQuad(backend, ex + edgePx, ey + edgePx, ew - 2 * edgePx, eh - 2 * edgePx, 0, 0, 1, 1,
                        btn->backgroundColor, whiteTexture.get(), rotation, std::max(0.0f, btnCR - edgePx));
                }
            }
            else
            {
                // Procedural rounded button with edge highlight
                UibinColor highlight = {
                    std::min(1.0f, btn->backgroundColor.r * 1.5f + 0.08f),
                    std::min(1.0f, btn->backgroundColor.g * 1.5f + 0.08f),
                    std::min(1.0f, btn->backgroundColor.b * 1.5f + 0.08f),
                    btn->backgroundColor.a };
                float edgePx = 1.5f * uniformScale;
                DrawQuad(backend, ex, ey, ew, eh, 0, 0, 1, 1, highlight, whiteTexture.get(), rotation, btnCR);
                DrawQuad(backend, ex + edgePx, ey + edgePx, ew - 2 * edgePx, eh - 2 * edgePx, 0, 0, 1, 1,
                    btn->backgroundColor, whiteTexture.get(), rotation, std::max(0.0f, btnCR - edgePx));
            }
        }

        // ── ProgressBar ──
        if (const auto* prog = element.GetProgressBar())
        {
            float cr = static_cast<float>(prog->cornerRadius) * uniformScale;
            DrawQuad(backend, ex, ey, ew, eh, 0, 0, 1, 1, prog->backgroundColor, whiteTexture.get(), 0.0f, cr);

            float value = static_cast<float>(std::clamp(prog->value, 0.0, 1.0));
            if (prog->direction == 0)
                DrawQuad(backend, ex, ey, ew * value, eh, 0, 0, 1, 1, prog->fillColor, whiteTexture.get(), 0.0f, cr);
            else
                DrawQuad(backend, ex, ey + eh * (1.0f - value), ew, eh * value, 0, 0, 1, 1, prog->fillColor, whiteTexture.get(), 0.0f, cr);
        }

        // ── Toggle ──
        if (const auto* toggle = element.GetToggle())
        {
            const UibinColor& trackColor = toggle->checked ? toggle->onColor : toggle->offColor;
            float trackCR = eh * 0.5f;
            DrawQuad(backend, ex, ey, ew, eh, 0, 0, 1, 1, trackColor, whiteTexture.get(), 0.0f, trackCR);

            float togglePad = 2.0f * uniformScale;
            float knobSize = eh - 2.0f * togglePad;
            float knobX = toggle->checked ? ex + ew - knobSize - togglePad : ex + togglePad;
            float knobY = ey + togglePad;
            DrawQuad(backend, knobX, knobY, knobSize, knobSize, 0, 0, 1, 1, toggle->knobColor, whiteTexture.get(), 0.0f, knobSize * 0.5f);
        }

        // ── TextInput ──
        if (const auto* textInput = element.GetTextInput())
        {
            float cr = 4.0f * uniformScale;
            float borderPx = 1.0f * uniformScale;
            float insetPx = 6.0f * uniformScale;
            // Border
            DrawQuad(backend, ex, ey, ew, eh, 0, 0, 1, 1, textInput->borderColor, whiteTexture.get(), 0.0f, cr);
            DrawQuad(backend, ex + borderPx, ey + borderPx, ew - 2 * borderPx, eh - 2 * borderPx, 0, 0, 1, 1, textInput->backgroundColor, whiteTexture.get(), 0.0f, std::max(0.0f, cr - borderPx));

            if (currentRenderLayer)
            {
                TextData td;
                td.text = textInput->text.empty() ? textInput->placeholder : textInput->text;
                td.color = textInput->text.empty() ? textInput->placeholderColor : textInput->textColor;
                td.fontFamily = textInput->fontFamily;
                td.pixelSize = textInput->pixelSize;
                td.fontPath = textInput->fontFamily;
                td.alignment = AnchorFlags::LEFT | AnchorFlags::CENTER_Y;
                // Inset text slightly
                RenderText(backend, td, ex + insetPx, ey, ew - 2 * insetPx, eh, *currentRenderLayer);
            }

            // Cursor when focused
            if (focusedElementId == element.id && currentRenderLayer)
            {
                // Simple cursor at end of text
                std::string cacheKey = textInput->fontFamily + ":" + std::to_string(textInput->pixelSize);
                auto cacheIt = fontCache.find(cacheKey);
                float cursorX = ex + insetPx;
                if (cacheIt != fontCache.end())
                {
                    FontAtlas* atlas = cacheIt->second.get();
                    for (char c : textInput->text)
                    {
                        const auto* glyph = atlas->GetGlyph(static_cast<uint32_t>(c));
                        if (glyph) cursorX += glyph->advance;
                    }
                }
                float cursorPad = 4.0f * uniformScale;
                float cursorH = eh - 2 * cursorPad;
                float cursorY = ey + cursorPad;
                DrawQuad(backend, cursorX, cursorY, 1.0f * uniformScale, cursorH, 0, 0, 1, 1, textInput->textColor, whiteTexture.get());
            }
        }

        // ── Dropdown ──
        if (const auto* dropdown = element.GetDropdown())
        {
            float cr = 4.0f * uniformScale;
            float borderPx = 1.0f * uniformScale;
            float ddInset = 8.0f * uniformScale;
            bool isExpanded = (expandedDropdownId == element.id);

            // Main dropdown box
            DrawQuad(backend, ex, ey, ew, eh, 0, 0, 1, 1, dropdown->borderColor, whiteTexture.get(), 0.0f, cr);
            DrawQuad(backend, ex + borderPx, ey + borderPx, ew - 2 * borderPx, eh - 2 * borderPx, 0, 0, 1, 1, dropdown->backgroundColor, whiteTexture.get(), 0.0f, std::max(0.0f, cr - borderPx));

            // Parse all options
            std::vector<std::string> opts;
            {
                std::istringstream stream(dropdown->options);
                std::string token;
                while (std::getline(stream, token, ','))
                    opts.push_back(token);
            }

            if (currentRenderLayer)
            {
                // Selected option text (centered)
                std::string selectedText;
                if (dropdown->selectedIndex >= 0 && dropdown->selectedIndex < static_cast<int>(opts.size()))
                    selectedText = opts[dropdown->selectedIndex];

                if (!selectedText.empty())
                {
                    float uniformScale = std::min(scaleX, scaleY);
                    int32_t effectivePixelSize = std::max(dropdown->pixelSize, static_cast<int32_t>(eh / uniformScale * 0.5f));

                    TextData td;
                    td.text = selectedText;
                    td.color = dropdown->textColor;
                    td.pixelSize = effectivePixelSize;
                    td.alignment = AnchorFlags::CENTER_X | AnchorFlags::CENTER_Y;
                    RenderText(backend, td, ex + ddInset, ey, ew - 2 * ddInset, eh, *currentRenderLayer);
                }

                // Arrow indicator
                {
                    float uniformScale = std::min(scaleX, scaleY);
                    int32_t effectivePixelSize = std::max(dropdown->pixelSize, static_cast<int32_t>(eh / uniformScale * 0.5f));

                    TextData arrow;
                    arrow.text = isExpanded ? "^" : "v";
                    arrow.color = dropdown->textColor;
                    arrow.pixelSize = effectivePixelSize;
                    arrow.alignment = AnchorFlags::RIGHT | AnchorFlags::CENTER_Y;
                    RenderText(backend, arrow, ex, ey, ew - ddInset, eh, *currentRenderLayer);
                }
            }

            // Expanded option list
            if (isExpanded && currentRenderLayer)
            {
                float optionH = eh;
                float listY = ey + eh;

                for (int i = 0; i < static_cast<int>(opts.size()); ++i)
                {
                    float oy = listY + optionH * static_cast<float>(i);
                    UibinColor optBg = (i == dropdown->selectedIndex)
                        ? UibinColor{dropdown->borderColor.r * 1.2f, dropdown->borderColor.g * 1.2f, dropdown->borderColor.b * 1.2f, 1.0f}
                        : dropdown->backgroundColor;

                    // Option background with border
                    DrawQuad(backend, ex, oy, ew, optionH, 0, 0, 1, 1, dropdown->borderColor, whiteTexture.get());
                    DrawQuad(backend, ex + borderPx, oy + borderPx, ew - 2 * borderPx, optionH - 2 * borderPx, 0, 0, 1, 1, optBg, whiteTexture.get());

                    // Option text (centered)
                    {
                        float uniformScale = std::min(scaleX, scaleY);
                        int32_t effectivePixelSize = std::max(dropdown->pixelSize, static_cast<int32_t>(optionH / uniformScale * 0.5f));

                        TextData optTd;
                        optTd.text = opts[i];
                        optTd.color = dropdown->textColor;
                        optTd.pixelSize = effectivePixelSize;
                        optTd.alignment = AnchorFlags::CENTER_X | AnchorFlags::CENTER_Y;
                        RenderText(backend, optTd, ex + ddInset, oy, ew - 2 * ddInset, optionH, *currentRenderLayer);
                    }
                }
            }
        }

        // ── Icon ──
        if (const auto* icon = element.GetIcon())
        {
            if (!icon->imagePath.empty() && currentRenderLayer)
            {
                auto* tex = GetOrUploadTexture(icon->imagePath, *currentRenderLayer);
                float iSize = static_cast<float>(icon->iconSize) * uniformScale;
                float ix = ex + (ew - iSize) * 0.5f;
                float iy = ey + (eh - iSize) * 0.5f;
                if (tex)
                    DrawQuad(backend, ix, iy, iSize, iSize, 0, 0, 1, 1, icon->tintColor, tex, rotation);
            }
        }

        // ── Sprite ──
        if (const auto* sprite = element.GetSprite())
        {
            if (!sprite->imagePath.empty() && currentRenderLayer)
            {
                auto* tex = GetOrUploadTexture(sprite->imagePath, *currentRenderLayer);
                if (tex && sprite->columns > 0)
                {
                    int col = sprite->currentFrame % sprite->columns;
                    int row = sprite->currentFrame / sprite->columns;
                    float fw = static_cast<float>(sprite->frameWidth);
                    float fh = static_cast<float>(sprite->frameHeight);
                    float texW = static_cast<float>(tex->GetWidth());
                    float texH = static_cast<float>(tex->GetHeight());
                    float u0 = (col * fw) / texW;
                    float v0 = (row * fh) / texH;
                    float u1 = ((col + 1) * fw) / texW;
                    float v1 = ((row + 1) * fh) / texH;
                    DrawQuad(backend, ex, ey, ew, eh, u0, v0, u1, v1, {1, 1, 1, 1}, tex, rotation);
                }
            }
        }

        // ── ListRepeater ──
        if (const auto* repeater = element.GetListRepeater())
        {
            float itemH = static_cast<float>(repeater->itemHeight) * scaleY;
            float spacing = static_cast<float>(repeater->spacing) * scaleY;
            int count = repeater->itemCount;

            for (int i = 0; i < count; ++i)
            {
                float iy = (repeater->direction == 0)
                    ? ey + i * (itemH + spacing)
                    : ey;
                float ix = (repeater->direction == 0)
                    ? ex
                    : ex + i * (itemH + spacing);
                float iw = (repeater->direction == 0) ? ew : itemH;
                float ih = (repeater->direction == 0) ? itemH : eh;

                const UibinColor& color = (i % 2 == 0) ? repeater->itemColor : repeater->alternateColor;
                DrawQuad(backend, ix, iy, iw, ih, 0, 0, 1, 1, color, whiteTexture.get(), 0.0f, 0.0f);

                if (repeater->borderColor.a > 0)
                {
                    float bw = 1.0f * uniformScale;
                    DrawQuad(backend, ix, iy, iw, bw, 0, 0, 1, 1, repeater->borderColor, whiteTexture.get());
                    DrawQuad(backend, ix, iy + ih - bw, iw, bw, 0, 0, 1, 1, repeater->borderColor, whiteTexture.get());
                }
            }
        }

        // ── RadialMenu ──
        if (const auto* radial = element.GetRadialMenu())
        {
            float rcx = ex + ew * 0.5f;
            float rcy = ey + eh * 0.5f;
            float outerR = static_cast<float>(radial->outerRadius) * uniformScale;
            float innerR = static_cast<float>(radial->innerRadius) * uniformScale;
            int slices = std::max(2, radial->sliceCount);

            ArcMeshSet& meshes = GetOrCreateArcMeshes(slices, innerR, outerR);

            // Draw each slice with its color
            for (int i = 0; i < slices && i < static_cast<int>(meshes.slices.size()); ++i)
            {
                const UibinColor& color = (i == radial->highlightIndex)
                    ? radial->highlightColor : radial->sliceColor;
                DrawArcSlice(backend, rcx, rcy, outerR, meshes.slices[i].get(), color);
            }

            // Slice separator lines (thin rotated quads from inner to outer edge)
            if (radial->borderColor.a > 0)
            {
                constexpr float PI = 3.14159265358979f;
                constexpr float TAU = 2.0f * PI;
                float sliceAngle = TAU / static_cast<float>(slices);
                float lineW = 2.0f * uniformScale;
                float radialThickness = outerR - innerR;
                float midR = (innerR + outerR) * 0.5f;

                for (int i = 0; i < slices; ++i)
                {
                    float a = static_cast<float>(i) * sliceAngle - PI * 0.5f;
                    float rotDeg = (a - PI * 0.5f) * (180.0f / PI);
                    float lineCx = rcx + midR * std::cos(a);
                    float lineCy = rcy + midR * std::sin(a);

                    DrawQuad(backend, lineCx - lineW * 0.5f, lineCy - radialThickness * 0.5f,
                        lineW, radialThickness, 0, 0, 1, 1,
                        radial->borderColor, whiteTexture.get(), rotDeg, 0.0f);
                }

                // Border rings (custom arc meshes)
                if (meshes.outerRing)
                    DrawArcSlice(backend, rcx, rcy, outerR, meshes.outerRing.get(), radial->borderColor);
                if (meshes.innerRing)
                    DrawArcSlice(backend, rcx, rcy, outerR, meshes.innerRing.get(), radial->borderColor);
            }
        }

        // ── TabContainer ──
        if (const auto* tabs = element.GetTabContainer())
        {
            float tabH = static_cast<float>(tabs->tabHeight) * scaleY;

            // Parse tab names
            std::vector<std::string> tabNames;
            {
                std::istringstream stream(tabs->tabNames);
                std::string token;
                while (std::getline(stream, token, ','))
                    tabNames.push_back(token);
            }

            int tabCount = std::max(1, static_cast<int>(tabNames.size()));
            float tabW = ew / static_cast<float>(tabCount);

            // Draw tab bar
            for (int i = 0; i < tabCount; ++i)
            {
                const UibinColor& color = (i == tabs->activeTab) ? tabs->activeColor : tabs->inactiveColor;
                DrawQuad(backend, ex + i * tabW, ey, tabW, tabH, 0, 0, 1, 1, color, whiteTexture.get());
            }

            // Draw content area
            DrawQuad(backend, ex, ey + tabH, ew, eh - tabH, 0, 0, 1, 1, tabs->backgroundColor, whiteTexture.get());

            // Render only active tab's child
            if (tabs->activeTab >= 0 && tabs->activeTab < static_cast<int>(element.children.size()))
                RenderElement(element.children[tabs->activeTab], backend, ex, ey + tabH, ew, eh - tabH);
            return;
        }

        // ── DragSlot ──
        if (const auto* slot = element.GetDragSlot())
        {
            float slotSz = static_cast<float>(slot->slotSize) * uniformScale;
            float slotX = ex + (ew - slotSz) * 0.5f;
            float slotY = ey + (eh - slotSz) * 0.5f;
            float cr = static_cast<float>(slot->cornerRadius) * uniformScale;

            // Border
            DrawQuad(backend, slotX, slotY, slotSz, slotSz, 0, 0, 1, 1, slot->borderColor, whiteTexture.get(), 0.0f, cr);
            float bw = 1.0f * uniformScale;
            DrawQuad(backend, slotX + bw, slotY + bw, slotSz - 2 * bw, slotSz - 2 * bw, 0, 0, 1, 1,
                slot->backgroundColor, whiteTexture.get(), 0.0f, std::max(0.0f, cr - bw));

            if (slot->isEmpty)
            {
                float inset = 4.0f * uniformScale;
                DrawQuad(backend, slotX + inset, slotY + inset, slotSz - 2 * inset, slotSz - 2 * inset, 0, 0, 1, 1,
                    slot->emptyColor, whiteTexture.get(), 0.0f, std::max(0.0f, cr - inset));
            }
            else if (!slot->iconPath.empty() && currentRenderLayer)
            {
                auto* tex = GetOrUploadTexture(slot->iconPath, *currentRenderLayer);
                if (tex)
                {
                    float inset = 4.0f * uniformScale;
                    DrawQuad(backend, slotX + inset, slotY + inset, slotSz - 2 * inset, slotSz - 2 * inset, 0, 0, 1, 1,
                        {1, 1, 1, 1}, tex, 0.0f, 0.0f);
                }
            }
        }

        // ── Minimap ──
        if (const auto* minimap = element.GetMinimap())
        {
            float cr = (minimap->shape == 1) ? std::min(ew, eh) * 0.5f : 0.0f;
            float bw = static_cast<float>(minimap->borderWidth) * uniformScale;

            DrawQuad(backend, ex, ey, ew, eh, 0, 0, 1, 1, minimap->borderColor, whiteTexture.get(), 0.0f, cr);
            DrawQuad(backend, ex + bw, ey + bw, ew - 2 * bw, eh - 2 * bw, 0, 0, 1, 1,
                minimap->backgroundColor, whiteTexture.get(), 0.0f, std::max(0.0f, cr - bw));

            // Viewport indicator (centered placeholder)
            float vpW = (ew - 2 * bw) * 0.3f;
            float vpH = (eh - 2 * bw) * 0.3f;
            float vpX = ex + (ew - vpW) * 0.5f;
            float vpY = ey + (eh - vpH) * 0.5f;
            DrawQuad(backend, vpX, vpY, vpW, vpH, 0, 0, 1, 1, minimap->viewportColor, whiteTexture.get(), 0.0f, 0.0f);
        }

        // ── Tooltip ──
        if (const auto* tooltip = element.GetTooltip())
        {
            float cr = 4.0f * uniformScale;
            float bw = 1.0f * uniformScale;
            DrawQuad(backend, ex, ey, ew, eh, 0, 0, 1, 1, tooltip->borderColor, whiteTexture.get(), 0.0f, cr);
            DrawQuad(backend, ex + bw, ey + bw, ew - 2 * bw, eh - 2 * bw, 0, 0, 1, 1,
                tooltip->backgroundColor, whiteTexture.get(), 0.0f, std::max(0.0f, cr - bw));

            if (!tooltip->tooltipText.empty() && currentRenderLayer)
            {
                TextData td;
                td.text = tooltip->tooltipText;
                td.color = tooltip->textColor;
                td.fontFamily = tooltip->fontFamily;
                td.pixelSize = tooltip->pixelSize;
                td.fontPath = tooltip->fontFamily;
                td.alignment = AnchorFlags::CENTER_X | AnchorFlags::CENTER_Y;
                RenderText(backend, td, ex + bw, ey + bw, ew - 2 * bw, eh - 2 * bw, *currentRenderLayer);
            }
        }

        // ── Modal ──
        if (const auto* modal = element.GetModal())
        {
            if (!modal->visible)
                return;

            float cr = static_cast<float>(modal->cornerRadius) * uniformScale;
            float modalBorder = 1.0f * uniformScale;
            DrawQuad(backend, parentX, parentY, parentW, parentH, 0, 0, 1, 1, modal->overlayColor, whiteTexture.get());

            float panelW = ew * 0.7f;
            float panelH = eh * 0.7f;
            float panelX = ex + (ew - panelW) * 0.5f;
            float panelY = ey + (eh - panelH) * 0.5f;

            if (modal->borderColor.a > 0)
            {
                DrawQuad(backend, panelX - modalBorder, panelY - modalBorder, panelW + 2 * modalBorder, panelH + 2 * modalBorder, 0, 0, 1, 1, modal->borderColor, whiteTexture.get(), 0.0f, cr);
            }
            DrawQuad(backend, panelX, panelY, panelW, panelH, 0, 0, 1, 1, modal->panelColor, whiteTexture.get(), 0.0f, cr);

            for (const auto& child : element.children)
                RenderElement(child, backend, panelX, panelY, panelW, panelH);
            return;
        }

        // ── Text ──
        if (const auto* text = element.GetText())
        {
            if (!text->text.empty() && currentRenderLayer)
                RenderText(backend, *text, ex, ey, ew, eh, *currentRenderLayer);
        }

        // ── Button text ──
        if (const auto* btn = element.GetButton())
        {
            if (!btn->text.empty() && currentRenderLayer)
            {
                TextData textData;
                textData.text = btn->text;
                textData.fontFamily = btn->fontFamily;
                textData.pixelSize = btn->pixelSize;
                textData.color = btn->textColor;
                textData.fontPath = btn->fontPath;
                textData.alignment = AnchorFlags::CENTER_X | AnchorFlags::CENTER_Y;
                RenderText(backend, textData, ex, ey, ew, eh, *currentRenderLayer);
            }
        }

        // ── Layout Override for Children ────────────────────────────

        const auto* stackLayout = element.GetStackLayout();
        const auto* gridLayout = element.GetGridLayout();
        const auto* scrollBox = element.GetScrollBox();

        if (scrollBox && !element.children.empty())
        {
            bool isHorizontal = (scrollBox->direction == 1);
            float padding = static_cast<float>(scrollBox->padding) * uniformScale;
            float spacing = static_cast<float>(scrollBox->spacing) * (isHorizontal ? scaleX : scaleY);

            // Compute total content size
            float totalContent = padding;
            for (const auto& child : element.children)
            {
                const TransformData* ct = child.GetTransform();
                if (!ct) continue;
                float childSize = isHorizontal
                    ? static_cast<float>(ct->scaleX) * scaleX
                    : static_cast<float>(ct->scaleY) * scaleY;
                totalContent += childSize + spacing;
            }
            totalContent += padding - spacing;

            float visibleSize = isHorizontal ? ew : eh;
            float maxScroll = std::max(0.0f, totalContent - visibleSize);

            // Get and clamp scroll offset
            float scrollOffset = 0.0f;
            if (!element.id.empty())
            {
                auto sIt = scrollOffsets.find(element.id);
                if (sIt != scrollOffsets.end())
                {
                    sIt->second = std::clamp(sIt->second, 0.0f, maxScroll);
                    scrollOffset = sIt->second;
                }
                scrollContentSizes[element.id] = totalContent;
            }

            // Queue scissor before children draws
            backend->SubmitSetScissor(
                static_cast<int32_t>(ex), static_cast<int32_t>(ey),
                static_cast<uint32_t>(ew), static_cast<uint32_t>(eh));

            // Render children (queues draw commands inside the scissor)
            // Subtract child's own x/y offset so layout cursor fully controls positioning
            float cursor = padding - scrollOffset;
            for (const auto& child : element.children)
            {
                const TransformData* ct = child.GetTransform();
                if (!ct) continue;

                float cw = static_cast<float>(ct->scaleX) * scaleX;
                float ch = static_cast<float>(ct->scaleY) * scaleY;
                float childOffX = static_cast<float>(ct->x) * scaleX;
                float childOffY = static_cast<float>(ct->y) * scaleY;

                if (isHorizontal)
                {
                    RenderElement(child, backend, ex + cursor - childOffX, ey + padding - childOffY, cw, eh - 2 * padding);
                    cursor += cw + spacing;
                }
                else
                {
                    RenderElement(child, backend, ex + padding - childOffX, ey + cursor - childOffY, ew - 2 * padding, ch);
                    cursor += ch + spacing;
                }
            }

            // Clear scissor after children
            backend->SubmitClearScissor();

            // Render scrollbar
            if (maxScroll > 0.0f && !element.id.empty())
            {
                bool showScrollbar = (draggingScrollbarId == element.id);
                if (!showScrollbar && !hoveredElementId.empty())
                {
                    auto rIt = computedRects.find(element.id);
                    if (rIt != computedRects.end())
                    {
                        auto& r = rIt->second;
                        auto hIt = computedRects.find(hoveredElementId);
                        if (hIt != computedRects.end())
                        {
                            float hcx = hIt->second.x + hIt->second.w * 0.5f;
                            float hcy = hIt->second.y + hIt->second.h * 0.5f;
                            if (hcx >= r.x && hcx <= r.x + r.w && hcy >= r.y && hcy <= r.y + r.h)
                                showScrollbar = true;
                        }
                        if (hoveredElementId == element.id)
                            showScrollbar = true;
                    }
                }

                if (showScrollbar)
                {
                    float trackPad = 2.0f * uniformScale;
                    float trackThickness = 4.0f * uniformScale;
                    float thumbMinSize = 12.0f * uniformScale;
                    float thumbRatio = visibleSize / totalContent;

                    UibinColor trackColor = {0.3f, 0.3f, 0.3f, 0.3f};
                    UibinColor thumbColor = {0.7f, 0.7f, 0.7f, 0.6f};

                    if (isHorizontal)
                    {
                        float trackX = ex + trackPad;
                        float trackY = ey + eh - trackThickness - trackPad;
                        float trackW = ew - 2 * trackPad;
                        float thumbW = std::max(trackW * thumbRatio, thumbMinSize);
                        float thumbX = trackX + (maxScroll > 0.0f ? (scrollOffset / maxScroll) * (trackW - thumbW) : 0.0f);

                        DrawQuad(backend, trackX, trackY, trackW, trackThickness, 0, 0, 1, 1, trackColor, whiteTexture.get(), 0.0f, trackThickness * 0.5f);
                        DrawQuad(backend, thumbX, trackY, thumbW, trackThickness, 0, 0, 1, 1, thumbColor, whiteTexture.get(), 0.0f, trackThickness * 0.5f);

                        scrollbarThumbRects[element.id] = {thumbX, trackY, thumbW, trackThickness};
                    }
                    else
                    {
                        float trackX = ex + ew - trackThickness - trackPad;
                        float trackY = ey + trackPad;
                        float trackH = eh - 2 * trackPad;
                        float thumbH = std::max(trackH * thumbRatio, thumbMinSize);
                        float thumbY = trackY + (maxScroll > 0.0f ? (scrollOffset / maxScroll) * (trackH - thumbH) : 0.0f);

                        DrawQuad(backend, trackX, trackY, trackThickness, trackH, 0, 0, 1, 1, trackColor, whiteTexture.get(), 0.0f, trackThickness * 0.5f);
                        DrawQuad(backend, trackX, thumbY, trackThickness, thumbH, 0, 0, 1, 1, thumbColor, whiteTexture.get(), 0.0f, trackThickness * 0.5f);

                        scrollbarThumbRects[element.id] = {trackX, thumbY, trackThickness, thumbH};
                    }
                }
            }

            return;
        }

        if (stackLayout && !element.children.empty())
        {
            float padding = static_cast<float>(stackLayout->padding) * uniformScale;
            float spacing = static_cast<float>(stackLayout->spacing) * (stackLayout->direction == 0 ? scaleX : scaleY);
            float cursor = padding;

            for (const auto& child : element.children)
            {
                const TransformData* ct = child.GetTransform();
                if (!ct) continue;

                float cw = static_cast<float>(ct->scaleX) * scaleX;
                float ch = static_cast<float>(ct->scaleY) * scaleY;
                float childOffX = static_cast<float>(ct->x) * scaleX;
                float childOffY = static_cast<float>(ct->y) * scaleY;

                if (stackLayout->direction == 0)
                {
                    RenderElement(child, backend, ex + cursor - childOffX, ey + padding - childOffY, cw, eh - 2 * padding);
                    cursor += cw + spacing;
                }
                else
                {
                    RenderElement(child, backend, ex + padding - childOffX, ey + cursor - childOffY, ew - 2 * padding, ch);
                    cursor += ch + spacing;
                }
            }
            return;
        }

        if (gridLayout && !element.children.empty())
        {
            float padding = static_cast<float>(gridLayout->padding) * uniformScale;
            float spacingH = static_cast<float>(gridLayout->spacingH) * scaleX;
            float spacingV = static_cast<float>(gridLayout->spacingV) * scaleY;
            int cols = std::max(1, gridLayout->columns);

            float cellW = (ew - 2 * padding - (cols - 1) * spacingH) / static_cast<float>(cols);

            // First pass: find tallest child in each row
            std::vector<float> rowHeights;
            for (size_t i = 0; i < element.children.size(); ++i)
            {
                size_t row = i / cols;
                if (row >= rowHeights.size())
                    rowHeights.push_back(0.0f);

                const TransformData* ct = element.children[i].GetTransform();
                float ch = ct ? static_cast<float>(ct->scaleY) * scaleY : 40.0f * scaleY;
                rowHeights[row] = std::max(rowHeights[row], ch);
            }

            // Second pass: render
            float cursorY = padding;
            for (size_t i = 0; i < element.children.size(); ++i)
            {
                int col = static_cast<int>(i % cols);
                size_t row = i / cols;

                if (col == 0 && i > 0)
                    cursorY += rowHeights[row - 1] + spacingV;

                float cx = ex + padding + col * (cellW + spacingH);
                float cy = ey + cursorY;

                RenderElement(element.children[i], backend, cx, cy, cellW, rowHeights[row]);
            }
            return;
        }

        // Default: recurse into children (children render ON TOP — the UIStack principle)
        for (const auto& child : element.children)
            RenderElement(child, backend, ex, ey, ew, eh);
    }

    void UIStackModule::ComputeRect(const TransformData& t,
        float parentX, float parentY, float parentW, float parentH,
        float& outX, float& outY, float& outW, float& outH) const
    {
        bool stretchH = (t.stretch & (AnchorFlags::LEFT | AnchorFlags::RIGHT)) == (AnchorFlags::LEFT | AnchorFlags::RIGHT);
        bool stretchV = (t.stretch & (AnchorFlags::TOP | AnchorFlags::BOTTOM)) == (AnchorFlags::TOP | AnchorFlags::BOTTOM);

        outW = stretchH ? parentW : static_cast<float>(t.scaleX) * scaleX;
        outH = stretchV ? parentH : static_cast<float>(t.scaleY) * scaleY;

        bool anchorRight   = (t.anchors & AnchorFlags::RIGHT) != 0;
        bool anchorBottom  = (t.anchors & AnchorFlags::BOTTOM) != 0;
        bool anchorCenterX = (t.anchors & AnchorFlags::CENTER_X) != 0;
        bool anchorCenterY = (t.anchors & AnchorFlags::CENTER_Y) != 0;

        float baseX = anchorCenterX ? parentX + (parentW - outW) * 0.5f
                     : anchorRight  ? parentX + parentW - outW
                     : parentX;

        float baseY = anchorCenterY ? parentY + (parentH - outH) * 0.5f
                     : anchorBottom ? parentY + parentH - outH
                     : parentY;

        outX = baseX + static_cast<float>(t.x) * scaleX;
        outY = baseY + static_cast<float>(t.y) * scaleY;
    }

    void UIStackModule::DrawQuad(Render::IRenderBackend* backend,
        float x, float y, float w, float h,
        float u0, float v0, float u1, float v1,
        const UibinColor& tint, Render::ITextureHandle* texture,
        float rotation, float cornerRadius)
    {
        if (!texture || w <= 0 || h <= 0) return;

        int32_t frameIndex = backend->GetCurrentFrame();

        UIUniformData uniData{};
        uniData.posX = x;
        uniData.posY = y;
        uniData.width = w;
        uniData.height = h;
        uniData.u0 = u0;
        uniData.v0 = v0;
        uniData.u1 = u1;
        uniData.v1 = v1;
        uniData.tintR = tint.r;
        uniData.tintG = tint.g;
        uniData.tintB = tint.b;
        uniData.tintA = tint.a;
        uniData.screenW = screenW;
        uniData.screenH = screenH;
        uniData.rotation = rotation;
        uniData.cornerRadius = cornerRadius;

        auto& slot = AcquireUniformSlot();
        auto* frameBuffer = slot.buffers[frameIndex].get();
        frameBuffer->SetSubData(&uniData, UIUniformData::Size(), 0);
        slot.binding->UpdateBuffer(0, frameBuffer, UIUniformData::Size(), frameIndex);
        slot.binding->UpdateTexture(1, texture, frameIndex);

        backend->SubmitDrawCommand(shader.get(), slot.binding.get(), frameIndex, quadMesh->GetUnderlyingMesh());
    }

    void UIStackModule::DrawNineSlice(Render::IRenderBackend* backend,
        float x, float y, float w, float h,
        int32_t sliceL, int32_t sliceT, int32_t sliceR, int32_t sliceB,
        Render::ITextureHandle* texture, const UibinColor& tint, float cornerRadius)
    {
        if (!texture || w <= 0 || h <= 0) return;

        float texW = static_cast<float>(texture->GetWidth());
        float texH = static_cast<float>(texture->GetHeight());
        if (texW <= 0 || texH <= 0) return;

        float sl = static_cast<float>(sliceL);
        float st = static_cast<float>(sliceT);
        float sr = static_cast<float>(sliceR);
        float sb = static_cast<float>(sliceB);

        // UV boundaries
        float uL = sl / texW;
        float uR = 1.0f - sr / texW;
        float vT = st / texH;
        float vB = 1.0f - sb / texH;

        // Position boundaries
        float midW = w - sl - sr;
        float midH = h - st - sb;

        // 9 patches: [row][col] where row=top/mid/bot, col=left/mid/right
        struct Patch { float px, py, pw, ph, u0, v0, u1, v1; };
        Patch patches[9] = {
            // Top row
            { x,            y,            sl,   st,   0.0f, 0.0f, uL,   vT   },
            { x + sl,       y,            midW, st,   uL,   0.0f, uR,   vT   },
            { x + sl + midW,y,            sr,   st,   uR,   0.0f, 1.0f, vT   },
            // Middle row
            { x,            y + st,       sl,   midH, 0.0f, vT,   uL,   vB   },
            { x + sl,       y + st,       midW, midH, uL,   vT,   uR,   vB   },
            { x + sl + midW,y + st,       sr,   midH, uR,   vT,   1.0f, vB   },
            // Bottom row
            { x,            y + st + midH,sl,   sb,   0.0f, vB,   uL,   1.0f },
            { x + sl,       y + st + midH,midW, sb,   uL,   vB,   uR,   1.0f },
            { x + sl + midW,y + st + midH,sr,   sb,   uR,   vB,   1.0f, 1.0f },
        };

        for (auto& p : patches)
        {
            if (p.pw > 0 && p.ph > 0)
                DrawQuad(backend, p.px, p.py, p.pw, p.ph, p.u0, p.v0, p.u1, p.v1, tint, texture, 0.0f, cornerRadius);
        }
    }

    Render::ITextureHandle* UIStackModule::GetOrUploadTexture(const std::string& assetKey, UILayer& layer)
    {
        auto it = layer.textures.find(assetKey);
        if (it != layer.textures.end())
            return it->second.get();

        auto assetIt = layer.scene.assets.find(assetKey);
        if (assetIt == layer.scene.assets.end())
            return nullptr;

        const auto& rawBytes = assetIt->second;
        int imgW, imgH, channels;
        unsigned char* pixels = stbi_load_from_memory(rawBytes.data(), static_cast<int>(rawBytes.size()),
            &imgW, &imgH, &channels, 4);

        if (!pixels)
        {
            logger->warn("Failed to decode image asset: {}", assetKey);
            return nullptr;
        }

        Render::TextureDescription desc;
        desc.width = static_cast<uint32_t>(imgW);
        desc.height = static_cast<uint32_t>(imgH);
        desc.format = Render::TextureFormat::RGBA8;
        desc.wrapS = Render::TextureWrapMode::CLAMP_TO_EDGE;
        desc.wrapT = Render::TextureWrapMode::CLAMP_TO_EDGE;
        desc.minFilter = Render::TextureFilterMode::LINEAR;
        desc.magFilter = Render::TextureFilterMode::LINEAR;

        auto handle = textureManager->CreateFromMemory(desc, pixels);
        stbi_image_free(pixels);

        if (!handle || !handle->IsValid())
        {
            logger->warn("Failed to upload texture: {}", assetKey);
            return nullptr;
        }

        auto* rawPtr = handle.get();
        layer.textures[assetKey] = std::move(handle);
        return rawPtr;
    }

    UIStackModule::ArcMeshSet& UIStackModule::GetOrCreateArcMeshes(int sliceCount, float innerR, float outerR)
    {
        // Use a synthetic key combining slice count and radius ratio
        float ratio = innerR / outerR;
        std::string key = std::to_string(sliceCount) + ":" + std::to_string(static_cast<int>(ratio * 1000));

        auto it = arcMeshCache.find(key);
        if (it != arcMeshCache.end())
            return it->second;

        constexpr float PI = 3.14159265358979f;
        constexpr float TAU = 2.0f * PI;

        float normOuterR = 0.5f;
        float normInnerR = 0.5f * ratio;
        float sliceAngle = TAU / static_cast<float>(sliceCount);
        int subdivisions = std::max(16, 64 / sliceCount);

        ArcMeshSet& meshSet = arcMeshCache[key];
        meshSet.sliceCount = sliceCount;
        meshSet.radiusRatio = ratio;

        // Create one mesh per slice
        for (int s = 0; s < sliceCount; ++s)
        {
            float a0 = static_cast<float>(s) * sliceAngle - PI * 0.5f;

            std::vector<UIVertex> vertices;
            std::vector<uint32_t> indices;

            for (int k = 0; k <= subdivisions; ++k)
            {
                float t = static_cast<float>(k) / static_cast<float>(subdivisions);
                float a = a0 + t * sliceAngle;
                float ca = std::cos(a), sa = std::sin(a);

                vertices.push_back({0.5f + normInnerR * ca, 0.5f + normInnerR * sa, 0.0f, 0.0f});
                vertices.push_back({0.5f + normOuterR * ca, 0.5f + normOuterR * sa, 0.0f, 0.0f});
            }

            for (int k = 0; k < subdivisions; ++k)
            {
                uint32_t i0 = static_cast<uint32_t>(k * 2);
                uint32_t i1 = i0 + 1;
                uint32_t i2 = i0 + 2;
                uint32_t i3 = i0 + 3;
                indices.push_back(i0); indices.push_back(i1); indices.push_back(i2);
                indices.push_back(i2); indices.push_back(i1); indices.push_back(i3);
            }

            auto mesh = std::make_unique<Render::Resource::Mesh>(
                *bufferManager, UIVertex::LAYOUT, Render::PrimitiveType::TRIANGLES);
            mesh->SetVertices(vertices);
            mesh->SetIndices(indices);
            meshSet.slices.push_back(std::move(mesh));
        }

        // Create outer ring border mesh (thin arc around the full circle)
        {
            int ringSegs = 64;
            float ringAngle = TAU / static_cast<float>(ringSegs);
            float borderThick = 0.005f; // normalized thickness

            std::vector<UIVertex> vertices;
            std::vector<uint32_t> indices;

            for (int k = 0; k <= ringSegs; ++k)
            {
                float a = static_cast<float>(k) * ringAngle;
                float ca = std::cos(a), sa = std::sin(a);
                vertices.push_back({0.5f + (normOuterR - borderThick) * ca, 0.5f + (normOuterR - borderThick) * sa, 0.0f, 0.0f});
                vertices.push_back({0.5f + normOuterR * ca, 0.5f + normOuterR * sa, 0.0f, 0.0f});
            }
            for (int k = 0; k < ringSegs; ++k)
            {
                uint32_t i0 = static_cast<uint32_t>(k * 2);
                indices.push_back(i0); indices.push_back(i0 + 1); indices.push_back(i0 + 2);
                indices.push_back(i0 + 2); indices.push_back(i0 + 1); indices.push_back(i0 + 3);
            }

            meshSet.outerRing = std::make_unique<Render::Resource::Mesh>(
                *bufferManager, UIVertex::LAYOUT, Render::PrimitiveType::TRIANGLES);
            meshSet.outerRing->SetVertices(vertices);
            meshSet.outerRing->SetIndices(indices);
        }

        // Create inner ring border mesh
        {
            int ringSegs = 64;
            float ringAngle = TAU / static_cast<float>(ringSegs);
            float borderThick = 0.005f;

            std::vector<UIVertex> vertices;
            std::vector<uint32_t> indices;

            for (int k = 0; k <= ringSegs; ++k)
            {
                float a = static_cast<float>(k) * ringAngle;
                float ca = std::cos(a), sa = std::sin(a);
                vertices.push_back({0.5f + normInnerR * ca, 0.5f + normInnerR * sa, 0.0f, 0.0f});
                vertices.push_back({0.5f + (normInnerR + borderThick) * ca, 0.5f + (normInnerR + borderThick) * sa, 0.0f, 0.0f});
            }
            for (int k = 0; k < ringSegs; ++k)
            {
                uint32_t i0 = static_cast<uint32_t>(k * 2);
                indices.push_back(i0); indices.push_back(i0 + 1); indices.push_back(i0 + 2);
                indices.push_back(i0 + 2); indices.push_back(i0 + 1); indices.push_back(i0 + 3);
            }

            meshSet.innerRing = std::make_unique<Render::Resource::Mesh>(
                *bufferManager, UIVertex::LAYOUT, Render::PrimitiveType::TRIANGLES);
            meshSet.innerRing->SetVertices(vertices);
            meshSet.innerRing->SetIndices(indices);
        }

        return meshSet;
    }

    void UIStackModule::DrawArcSlice(Render::IRenderBackend* backend,
        float cx, float cy, float outerR, Render::Resource::Mesh* arcMesh,
        const UibinColor& tint)
    {
        if (!arcMesh || !arcMesh->IsValid()) return;

        int32_t frameIndex = backend->GetCurrentFrame();
        float diameter = outerR * 2.0f;

        UIUniformData uniData{};
        uniData.posX = cx - outerR;
        uniData.posY = cy - outerR;
        uniData.width = diameter;
        uniData.height = diameter;
        uniData.u0 = 0; uniData.v0 = 0; uniData.u1 = 1; uniData.v1 = 1;
        uniData.tintR = tint.r;
        uniData.tintG = tint.g;
        uniData.tintB = tint.b;
        uniData.tintA = tint.a;
        uniData.screenW = screenW;
        uniData.screenH = screenH;
        uniData.rotation = 0.0f;
        uniData.cornerRadius = 0.0f;

        auto& slot = AcquireUniformSlot();
        auto* frameBuffer = slot.buffers[frameIndex].get();
        frameBuffer->SetSubData(&uniData, UIUniformData::Size(), 0);
        slot.binding->UpdateBuffer(0, frameBuffer, UIUniformData::Size(), frameIndex);
        slot.binding->UpdateTexture(1, whiteTexture.get(), frameIndex);

        backend->SubmitDrawCommand(shader.get(), slot.binding.get(), frameIndex, arcMesh->GetUnderlyingMesh());
    }

    void UIStackModule::RenderText(Render::IRenderBackend* backend,
        const TextData& textData, float x, float y, float w, float h,
        UILayer& layer)
    {
        // Scale font pixel size from design-space to screen-space
        float uniformScale = std::min(scaleX, scaleY);
        float scaledPixelSize = static_cast<float>(textData.pixelSize) * uniformScale;
        if (scaledPixelSize < 1.0f) scaledPixelSize = 1.0f;

        // Determine font source: embedded asset or default fallback
        std::string fontKey = textData.fontPath;
        if (fontKey.empty())
            fontKey = "__default__";

        // Cache key uses the scaled pixel size so atlas is built at screen resolution
        int scaledPxInt = static_cast<int>(scaledPixelSize + 0.5f);
        std::string cacheKey = fontKey + ":" + std::to_string(scaledPxInt);
        auto cacheIt = fontCache.find(cacheKey);

        if (cacheIt == fontCache.end())
        {
            const std::vector<uint8_t>* fontData = nullptr;

            if (!textData.fontPath.empty())
            {
                auto assetIt = layer.scene.assets.find(textData.fontPath);
                if (assetIt != layer.scene.assets.end())
                    fontData = &assetIt->second;
            }

            if (!fontData && !defaultFontData.empty())
                fontData = &defaultFontData;

            if (!fontData)
                return;

            auto atlas = std::make_unique<FontAtlas>();
            if (!atlas->Build(*fontData, scaledPixelSize, textureManager))
            {
                logger->warn("Failed to build font atlas for: {}", fontKey);
                return;
            }

            cacheIt = fontCache.emplace(cacheKey, std::move(atlas)).first;
        }

        FontAtlas* atlas = cacheIt->second.get();
        Render::ITextureHandle* atlasTex = atlas->GetTexture();
        if (!atlasTex) return;

        // Measure text width for alignment (metrics are already in screen-space)
        float textWidth = 0.0f;
        for (char c : textData.text)
        {
            const auto* glyph = atlas->GetGlyph(static_cast<uint32_t>(c));
            if (glyph) textWidth += glyph->advance;
        }

        // Compute starting position based on alignment
        float startX = x;
        float startY = y;

        if (textData.alignment & AnchorFlags::CENTER_X)
            startX = x + (w - textWidth) * 0.5f;
        else if (textData.alignment & AnchorFlags::RIGHT)
            startX = x + w - textWidth;

        float lineHeight = atlas->GetLineHeight();
        float visibleHeight = atlas->GetVisibleHeight();
        if (textData.alignment & AnchorFlags::CENTER_Y)
            startY = y + (h - visibleHeight) * 0.5f;
        else if (textData.alignment & AnchorFlags::BOTTOM)
            startY = y + h - visibleHeight;

        float cursorX = startX;
        for (char c : textData.text)
        {
            const auto* glyph = atlas->GetGlyph(static_cast<uint32_t>(c));
            if (!glyph)
            {
                cursorX += scaledPixelSize * 0.5f;
                continue;
            }

            float gx = cursorX + glyph->bearingX;
            float gy = startY + glyph->bearingY;

            DrawQuad(backend, gx, gy, glyph->width, glyph->height,
                glyph->uvX0, glyph->uvY0, glyph->uvX1, glyph->uvY1,
                textData.color, atlasTex);

            cursorX += glyph->advance;
        }
    }

    UIStackModule::UniformSlot& UIStackModule::AcquireUniformSlot()
    {
        if (uniformPoolIndex < uniformPool.size())
            return uniformPool[uniformPoolIndex++];

        UniformSlot slot;
        slot.buffers.resize(maxFramesInFlight);
        for (int32_t i = 0; i < maxFramesInFlight; ++i)
            slot.buffers[i] = bufferManager->CreateUniformBuffer(UIUniformData::Size());

        slot.binding = uniformManager->CreateBindingForShader(shader.get());

        if (slot.binding)
        {
            for (int32_t i = 0; i < maxFramesInFlight; ++i)
            {
                slot.binding->UpdateBuffer(0, slot.buffers[i].get(), UIUniformData::Size(), i);
                if (whiteTexture)
                    slot.binding->UpdateTexture(1, whiteTexture.get(), i);
            }
        }

        uniformPool.push_back(std::move(slot));
        return uniformPool[uniformPoolIndex++];
    }
}
