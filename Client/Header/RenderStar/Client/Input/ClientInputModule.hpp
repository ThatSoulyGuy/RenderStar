#pragma once

#include "RenderStar/Client/Input/KeyCode.hpp"
#include "RenderStar/Client/Input/KeyState.hpp"
#include "RenderStar/Client/Input/MouseCode.hpp"
#include "RenderStar/Client/Input/MouseState.hpp"
#include "RenderStar/Client/Input/KeyModifier.hpp"
#include "RenderStar/Common/Module/AbstractModule.hpp"
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

struct GLFWwindow;

namespace RenderStar::Client::Input
{
    struct KeyCombo
    {
        KeyCode key;
        KeyModifier modifiers;
    };

    class ClientInputModule final : public Common::Module::AbstractModule
    {
    public:

        ClientInputModule();

        ~ClientInputModule() override;

        void Tick();

        void EndFrame();

        [[nodiscard]]
        bool IsKeyPressed(KeyCode key) const;

        [[nodiscard]]
        bool IsKeyHeld(KeyCode key) const;

        [[nodiscard]]
        bool IsKeyReleased(KeyCode key) const;

        [[nodiscard]]
        bool IsMouseButtonPressed(MouseCode button) const;

        [[nodiscard]]
        bool IsMouseButtonHeld(MouseCode button) const;

        [[nodiscard]]
        bool IsMouseButtonReleased(MouseCode button) const;

        [[nodiscard]]
        glm::dvec2 GetCursorPosition() const;

        [[nodiscard]]
        glm::dvec2 GetCursorDelta() const;

        [[nodiscard]]
        glm::dvec2 GetScrollDelta() const;

        void LockCursor();

        void UnlockCursor();

        [[nodiscard]]
        bool IsCursorLocked() const;

        void RegisterCombo(const std::string& name, KeyCombo combo);

        void UnregisterCombo(const std::string& name);

        [[nodiscard]]
        bool IsComboTriggered(const std::string& name) const;

        void OnKey(int32_t glfwKey, int32_t action);
        void OnMouseButton(int32_t glfwButton, int32_t action);
        void OnCursorPos(double xPos, double yPos);
        void OnScroll(double xOffset, double yOffset);

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

        void OnCleanup() override;

    private:

        static void InstallCallbacks(GLFWwindow* window);
        static void RemoveCallbacks(GLFWwindow* window);

        [[nodiscard]]
        KeyModifier GetCurrentModifiers() const;

        std::unordered_map<KeyCode, KeyState> keyStates;
        std::unordered_map<MouseCode, MouseState> mouseStates;

        glm::dvec2 cursorPosition{ 0.0, 0.0 };
        glm::dvec2 lastCursorPosition{ 0.0, 0.0 };
        glm::dvec2 cursorDelta{ 0.0, 0.0 };

        glm::dvec2 scrollDelta{ 0.0, 0.0 };

        std::unordered_map<std::string, KeyCombo> combos;
        std::unordered_map<std::string, bool> comboTriggered;

        GLFWwindow* windowHandle = nullptr;
        bool cursorLocked = false;
    };
}
