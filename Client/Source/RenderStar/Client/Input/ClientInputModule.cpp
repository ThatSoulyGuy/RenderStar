#include "RenderStar/Client/Input/ClientInputModule.hpp"
#include "RenderStar/Client/Core/ClientWindowModule.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace RenderStar::Client::Input
{
    static void GlfwKeyCallback(GLFWwindow* window, int key, int, int action, int)
    {
        if (auto* module = static_cast<ClientInputModule*>(glfwGetWindowUserPointer(window)); module != nullptr && action != GLFW_REPEAT)
            module->OnKey(key, action);
    }

    static void GlfwMouseButtonCallback(GLFWwindow* window, int button, int action, int)
    {
        if (auto* module = static_cast<ClientInputModule*>(glfwGetWindowUserPointer(window)); module != nullptr)
            module->OnMouseButton(button, action);
    }

    static void GlfwCursorPosCallback(GLFWwindow* window, double xPos, double yPos)
    {
        if (auto* module = static_cast<ClientInputModule*>(glfwGetWindowUserPointer(window)); module != nullptr)
            module->OnCursorPos(xPos, yPos);
    }

    static void GlfwScrollCallback(GLFWwindow* window, const double xOffset, const double yOffset)
    {
        if (auto* module = static_cast<ClientInputModule*>(glfwGetWindowUserPointer(window)); module != nullptr)
            module->OnScroll(xOffset, yOffset);
    }

    ClientInputModule::ClientInputModule() = default;

    ClientInputModule::~ClientInputModule() = default;

    void ClientInputModule::OnInitialize(Common::Module::ModuleContext& context)
    {
        const auto windowModule = context.GetModule<Core::ClientWindowModule>();

        if (!windowModule.has_value())
        {
            logger->error("ClientWindowModule not found — input will not function");
            return;
        }

        windowHandle = windowModule->get().GetWindowHandle();

        if (windowHandle == nullptr)
        {
            logger->error("Window handle is null — input will not function");
            return;
        }

        glfwSetWindowUserPointer(windowHandle, this);
        InstallCallbacks(windowHandle);

        double x, y;

        glfwGetCursorPos(windowHandle, &x, &y);

        cursorPosition = glm::dvec2(x, y);
        lastCursorPosition = cursorPosition;

        logger->info("ClientInputModule initialized");
    }

    void ClientInputModule::OnCleanup()
    {
        if (windowHandle != nullptr)
        {
            UnlockCursor();
            RemoveCallbacks(windowHandle);
            glfwSetWindowUserPointer(windowHandle, nullptr);

            windowHandle = nullptr;
        }

        keyStates.clear();
        mouseStates.clear();
        combos.clear();
        comboTriggered.clear();

        logger->info("ClientInputModule cleaned up");
    }

    void ClientInputModule::Tick()
    {
        cursorDelta = cursorPosition - lastCursorPosition;
        lastCursorPosition = cursorPosition;

        for (auto& triggered : comboTriggered | std::views::values)
            triggered = false;

        for (const auto& [name, combo] : combos)
        {
            if (const auto iterator = keyStates.find(combo.key); iterator != keyStates.end() && iterator->second == KeyState::PRESSED)
            {
                if (const KeyModifier currentMods = GetCurrentModifiers(); HasModifier(currentMods, combo.modifiers))
                    comboTriggered[name] = true;
            }
        }

        scrollDelta = glm::dvec2(0.0, 0.0);
    }

    void ClientInputModule::EndFrame()
    {
        std::erase_if(keyStates, [](const auto& pair)
        {
            return pair.second == KeyState::RELEASED;
        });

        for (auto& state : keyStates | std::views::values)
        {
            if (state == KeyState::PRESSED)
                state = KeyState::HELD;
        }

        std::erase_if(mouseStates, [](const auto& pair)
        {
            return pair.second == MouseState::RELEASED;
        });

        for (auto& state : mouseStates | std::views::values)
        {
            if (state == MouseState::PRESSED)
                state = MouseState::HELD;
        }
    }

    bool ClientInputModule::IsKeyPressed(const KeyCode key) const
    {
        const auto it = keyStates.find(key);

        return it != keyStates.end() && it->second == KeyState::PRESSED;
    }

    bool ClientInputModule::IsKeyHeld(const KeyCode key) const
    {
        const auto it = keyStates.find(key);

        return it != keyStates.end() && it->second == KeyState::HELD;
    }

    bool ClientInputModule::IsKeyReleased(const KeyCode key) const
    {
        const auto it = keyStates.find(key);

        return it != keyStates.end() && it->second == KeyState::RELEASED;
    }

    bool ClientInputModule::IsMouseButtonPressed(const MouseCode button) const
    {
        const auto it = mouseStates.find(button);

        return it != mouseStates.end() && it->second == MouseState::PRESSED;
    }

    bool ClientInputModule::IsMouseButtonHeld(const MouseCode button) const
    {
        const auto it = mouseStates.find(button);

        return it != mouseStates.end() && it->second == MouseState::HELD;
    }

    bool ClientInputModule::IsMouseButtonReleased(const MouseCode button) const
    {
        const auto it = mouseStates.find(button);

        return it != mouseStates.end() && it->second == MouseState::RELEASED;
    }

    glm::dvec2 ClientInputModule::GetCursorPosition() const
    {
        return cursorPosition;
    }

    glm::dvec2 ClientInputModule::GetCursorDelta() const
    {
        return cursorDelta;
    }

    glm::dvec2 ClientInputModule::GetScrollDelta() const
    {
        return scrollDelta;
    }

    void ClientInputModule::LockCursor()
    {
        if (windowHandle == nullptr || cursorLocked)
            return;

        glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        cursorLocked = true;
    }

    void ClientInputModule::UnlockCursor()
    {
        if (windowHandle == nullptr || !cursorLocked)
            return;

        glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        cursorLocked = false;
    }

    bool ClientInputModule::IsCursorLocked() const
    {
        return cursorLocked;
    }

    void ClientInputModule::RegisterCombo(const std::string& name, const KeyCombo combo)
    {
        combos[name] = combo;
        comboTriggered[name] = false;

        logger->debug("Registered key combo '{}'", name);
    }

    void ClientInputModule::UnregisterCombo(const std::string& name)
    {
        combos.erase(name);
        comboTriggered.erase(name);
    }

    bool ClientInputModule::IsComboTriggered(const std::string& name) const
    {
        const auto iterator = comboTriggered.find(name);

        return iterator != comboTriggered.end() && iterator->second;
    }

    void ClientInputModule::OnKey(int32_t glfwKey, int32_t action)
    {
        const auto key = static_cast<KeyCode>(glfwKey);

        if (action == GLFW_PRESS)
            keyStates[key] = KeyState::PRESSED;
        else if (action == GLFW_RELEASE)
            keyStates[key] = KeyState::RELEASED;
    }

    void ClientInputModule::OnMouseButton(int32_t glfwButton, const int32_t action)
    {
        const auto button = static_cast<MouseCode>(glfwButton);

        if (action == GLFW_PRESS)
            mouseStates[button] = MouseState::PRESSED;
        else if (action == GLFW_RELEASE)
            mouseStates[button] = MouseState::RELEASED;
    }

    void ClientInputModule::OnCursorPos(const double xPos, const double yPos)
    {
        cursorPosition = glm::dvec2(xPos, yPos);
    }

    void ClientInputModule::OnScroll(const double xOffset, const double yOffset)
    {
        scrollDelta += glm::dvec2(xOffset, yOffset);
    }

    void ClientInputModule::InstallCallbacks(GLFWwindow* window)
    {
        glfwSetKeyCallback(window, GlfwKeyCallback);
        glfwSetMouseButtonCallback(window, GlfwMouseButtonCallback);
        glfwSetCursorPosCallback(window, GlfwCursorPosCallback);
        glfwSetScrollCallback(window, GlfwScrollCallback);
    }

    void ClientInputModule::RemoveCallbacks(GLFWwindow* window)
    {
        glfwSetKeyCallback(window, nullptr);
        glfwSetMouseButtonCallback(window, nullptr);
        glfwSetCursorPosCallback(window, nullptr);
        glfwSetScrollCallback(window, nullptr);
    }

    KeyModifier ClientInputModule::GetCurrentModifiers() const
    {
        auto mods = KeyModifier::NONE;

        auto isDown = [this](const KeyCode key) -> bool
        {
            const auto iterator = keyStates.find(key);

            return iterator != keyStates.end() && (iterator->second == KeyState::PRESSED || iterator->second == KeyState::HELD);
        };

        if (isDown(KeyCode::LEFT_SHIFT) || isDown(KeyCode::RIGHT_SHIFT))
            mods = mods | KeyModifier::SHIFT;

        if (isDown(KeyCode::LEFT_CONTROL) || isDown(KeyCode::RIGHT_CONTROL))
            mods = mods | KeyModifier::CTRL;

        if (isDown(KeyCode::LEFT_ALT) || isDown(KeyCode::RIGHT_ALT))
            mods = mods | KeyModifier::ALT;

        if (isDown(KeyCode::LEFT_SUPER) || isDown(KeyCode::RIGHT_SUPER))
            mods = mods | KeyModifier::SUPER;

        return mods;
    }
}
