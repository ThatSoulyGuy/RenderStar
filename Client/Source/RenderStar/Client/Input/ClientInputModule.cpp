#include "RenderStar/Client/Input/ClientInputModule.hpp"
#include "RenderStar/Client/Core/ClientWindowModule.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace RenderStar::Client::Input
{
    static ClientInputModule* s_instance = nullptr;

    static void GlfwKeyCallback(GLFWwindow*, int key, int, int action, int)
    {
        if (s_instance != nullptr && action != GLFW_REPEAT)
            s_instance->OnKey(key, action);
    }

    static void GlfwMouseButtonCallback(GLFWwindow*, int button, int action, int)
    {
        if (s_instance != nullptr)
            s_instance->OnMouseButton(button, action);
    }

    static void GlfwCursorPosCallback(GLFWwindow*, double xPos, double yPos)
    {
        if (s_instance != nullptr)
            s_instance->OnCursorPos(xPos, yPos);
    }

    static void GlfwScrollCallback(GLFWwindow*, double xOffset, double yOffset)
    {
        if (s_instance != nullptr)
            s_instance->OnScroll(xOffset, yOffset);
    }

    ClientInputModule::ClientInputModule() = default;

    ClientInputModule::~ClientInputModule()
    {
        if (s_instance == this)
            s_instance = nullptr;
    }

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

        s_instance = this;
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
            RemoveCallbacks(windowHandle);
            windowHandle = nullptr;
        }

        if (s_instance == this)
            s_instance = nullptr;

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

        for (auto& [name, triggered] : comboTriggered)
            triggered = false;

        for (const auto& [name, combo] : combos)
        {
            const auto it = keyStates.find(combo.key);

            if (it != keyStates.end() && it->second == KeyState::PRESSED)
            {
                const KeyModifier currentMods = GetCurrentModifiers();

                if (HasModifier(currentMods, combo.modifiers))
                    comboTriggered[name] = true;
            }
        }

        std::erase_if(keyStates, [](const auto& pair)
        {
            return pair.second == KeyState::RELEASED;
        });

        for (auto& [key, state] : keyStates)
        {
            if (state == KeyState::PRESSED)
                state = KeyState::HELD;
        }

        std::erase_if(mouseStates, [](const auto& pair)
        {
            return pair.second == MouseState::RELEASED;
        });

        for (auto& [button, state] : mouseStates)
        {
            if (state == MouseState::PRESSED)
                state = MouseState::HELD;
        }

        scrollDelta = glm::dvec2(0.0, 0.0);
    }

    bool ClientInputModule::IsKeyPressed(KeyCode key) const
    {
        const auto it = keyStates.find(key);
        return it != keyStates.end() && it->second == KeyState::PRESSED;
    }

    bool ClientInputModule::IsKeyHeld(KeyCode key) const
    {
        const auto it = keyStates.find(key);
        return it != keyStates.end() && it->second == KeyState::HELD;
    }

    bool ClientInputModule::IsKeyReleased(KeyCode key) const
    {
        const auto it = keyStates.find(key);
        return it != keyStates.end() && it->second == KeyState::RELEASED;
    }

    bool ClientInputModule::IsMouseButtonPressed(MouseCode button) const
    {
        const auto it = mouseStates.find(button);
        return it != mouseStates.end() && it->second == MouseState::PRESSED;
    }

    bool ClientInputModule::IsMouseButtonHeld(MouseCode button) const
    {
        const auto it = mouseStates.find(button);
        return it != mouseStates.end() && it->second == MouseState::HELD;
    }

    bool ClientInputModule::IsMouseButtonReleased(MouseCode button) const
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

    void ClientInputModule::RegisterCombo(const std::string& name, KeyCombo combo)
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
        const auto it = comboTriggered.find(name);
        return it != comboTriggered.end() && it->second;
    }

    void ClientInputModule::OnKey(int32_t glfwKey, int32_t action)
    {
        const auto key = static_cast<KeyCode>(glfwKey);

        if (action == GLFW_PRESS)
            keyStates[key] = KeyState::PRESSED;
        else if (action == GLFW_RELEASE)
            keyStates[key] = KeyState::RELEASED;
    }

    void ClientInputModule::OnMouseButton(int32_t glfwButton, int32_t action)
    {
        const auto button = static_cast<MouseCode>(glfwButton);

        if (action == GLFW_PRESS)
            mouseStates[button] = MouseState::PRESSED;
        else if (action == GLFW_RELEASE)
            mouseStates[button] = MouseState::RELEASED;
    }

    void ClientInputModule::OnCursorPos(double xPos, double yPos)
    {
        cursorPosition = glm::dvec2(xPos, yPos);
    }

    void ClientInputModule::OnScroll(double xOffset, double yOffset)
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
        KeyModifier mods = KeyModifier::NONE;

        auto isDown = [this](KeyCode key) -> bool
        {
            const auto it = keyStates.find(key);
            return it != keyStates.end() &&
                   (it->second == KeyState::PRESSED || it->second == KeyState::HELD);
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
