#pragma once

#include <Windows.h>
#include "RenderStar/Math/Vector2.hpp"
#include "RenderStar/Util/Core/Map.hpp"
#include "RenderStar/Util/Core/Vector.hpp"
#include "RenderStar/Util/Typedefs.hpp"

#undef XBUTTON1
#undef XBUTTON2
#undef DELETE

using namespace RenderStar::Math;   
using namespace RenderStar::Util;
using namespace RenderStar::Util::Core;

namespace RenderStar
{
	namespace Core
	{
        enum class KeyCode
        {
            CANCEL = 0x03,
            XBUTTON1 = 0x05,
            XBUTTON2 = 0x06,

            BACK = 0x08,
            TAB = 0x09,
            CLEAR = 0x0C,
            RETURN = 0x0D,
            SHIFT = 0x10,
            CONTROL = 0x11,
            MENU = 0x12,
            PAUSE = 0x13,
            CAPITAL = 0x14,
            KANA = 0x15,
            HANGUEL = 0x15,
            HANGUL = 0x15,
            JUNJA = 0x17,
            FINAL = 0x18,
            HANJA = 0x19,
            KANJI = 0x19,
            ESCAPE = 0x1B,
            CONVERT = 0x1C,
            NONCONVERT = 0x1D,
            ACCEPT = 0x1E,
            MODECHANGE = 0x1F,
            SPACE = 0x20,
            PRIOR = 0x21,
            NEXT = 0x22,
            END = 0x23,
            HOME = 0x24,
            LEFT = 0x25,
            UP = 0x26,
            RIGHT = 0x27,
            DOWN = 0x28,
            SELECT = 0x29,
            PRINT = 0x2A,
            EXECUTE = 0x2B,
            SNAPSHOT = 0x2C,
            INSERT = 0x2D,
            DELETE = 0x2E,
            HELP = 0x2F,

            ZERO = 0x30,
            ONE = 0x31,
            TWO = 0x32,
            THREE = 0x33,
            FOUR = 0x34,
            FIVE = 0x35,
            SIX = 0x36,
            SEVEN = 0x37,
            EIGHT = 0x38,
            NINE = 0x39,

            A = 0x41,
            B = 0x42,
            C = 0x43,
            D = 0x44,
            E = 0x45,
            F = 0x46,
            G = 0x47,
            H = 0x48,
            I = 0x49,
            J = 0x4A,
            K = 0x4B,
            L = 0x4C,
            M = 0x4D,
            N = 0x4E,
            O = 0x4F,
            P = 0x50,
            Q = 0x51,
            R = 0x52,
            S = 0x53,
            T = 0x54,
            U = 0x55,
            V = 0x56,
            W = 0x57,
            X = 0x58,
            Y = 0x59,
            Z = 0x5A,

            NUMPAD_ZERO = 0x60,
            NUMPAD_ONE = 0x61,
            NUMPAD_TWO = 0x62,
            NUMPAD_THREE = 0x63,
            NUMPAD_FOUR = 0x64,
            NUMPAD_FIVE = 0x65,
            NUMPAD_SIX = 0x66,
            NUMPAD_SEVEN = 0x67,
            NUMPAD_EIGHT = 0x68,
            NUMPAD_NINE = 0x69,
            MULTIPLY = 0x6A,
            ADD = 0x6B,
            SEPARATOR = 0x6C,
            SUBTRACT = 0x6D,
            DECIMAL = 0x6E,
            DIVIDE = 0x6F,

            F1 = 0x70,
            F2 = 0x71,
            F3 = 0x72,
            F4 = 0x73,
            F5 = 0x74,
            F6 = 0x75,
            F7 = 0x76,
            F8 = 0x77,
            F9 = 0x78,
            F10 = 0x79,
            F11 = 0x7A,
            F12 = 0x7B,
            F13 = 0x7C,
            F14 = 0x7D,
            F15 = 0x7E,
            F16 = 0x7F,
            F17 = 0x80,
            F18 = 0x81,
            F19 = 0x82,
            F20 = 0x83,
            F21 = 0x84,
            F22 = 0x85,
            F23 = 0x86,
            F24 = 0x87,

            NUMLOCK = 0x90,
            SCROLL = 0x91,

            OEM_NEC_EQUAL = 0x92,

            OEM_FJ_JISHO = 0x92,
            OEM_FJ_MASSHOU = 0x93,
            OEM_FJ_TOUROKU = 0x94,
            OEM_FJ_LOYA = 0x95,
            OEM_FJ_ROYA = 0x96,

            LSHIFT = 0xA0,
            RSHIFT = 0xA1,
            LCONTROL = 0xA2,
            RCONTROL = 0xA3,
            LMENU = 0xA4,
            RMENU = 0xA5,

            BROWSER_BACK = 0xA6,
            BROWSER_FORWARD = 0xA7,
            BROWSER_REFRESH = 0xA8,
            BROWSER_STOP = 0xA9,
            BROWSER_SEARCH = 0xAA,
            BROWSER_FAVORITES = 0xAB,
            BROWSER_HOME = 0xAC,

            VOLUME_MUTE = 0xAD,
            VOLUME_DOWN = 0xAE,
            VOLUME_UP = 0xAF,

            MEDIA_NEXT_TRACK = 0xB0,
            MEDIA_PREV_TRACK = 0xB1,
            MEDIA_STOP = 0xB2,
            MEDIA_PLAY_PAUSE = 0xB3,

            LAUNCH_MAIL = 0xB4,
            LAUNCH_MEDIA_SELECT = 0xB5,
            LAUNCH_APP1 = 0xB6,
            LAUNCH_APP2 = 0xB7,

            OEM_1 = 0xBA,
            OEM_PLUS = 0xBB,
            OEM_COMMA = 0xBC,
            OEM_MINUS = 0xBD,
            OEM_PERIOD = 0xBE,
            OEM_2 = 0xBF,
            OEM_3 = 0xC0,

            OEM_4 = 0xDB,
            OEM_5 = 0xDC,
            OEM_6 = 0xDD,
            OEM_7 = 0xDE,
            OEM_8 = 0xDF,

            OEM_AX = 0xE1,
            OEM_102 = 0xE2,
            ICO_HELP = 0xE3,
            ICO_00 = 0xE4,
            PROCESSKEY = 0xE5,
            ICO_CLEAR = 0xE6,
            PACKET = 0xE7,
            OEM_RESET = 0xE9,
            OEM_JUMP = 0xEA,
            OEM_PA1 = 0xEB,
            OEM_PA2 = 0xEC,
            OEM_PA3 = 0xED,
            OEM_WSCTRL = 0xEE,
            OEM_CUSEL = 0xEF,
            OEM_ATTN = 0xF0,
            OEM_FINISH = 0xF1,
            OEM_COPY = 0xF2,
            OEM_AUTO = 0xF3,
            OEM_ENLW = 0xF4,
            OEM_BACKTAB = 0xF5,

            ATTN = 0xF6,
            CRSEL = 0xF7,
            EXSEL = 0xF8,
            EREOF = 0xF9,
            PLAY = 0xFA,
            ZOOM = 0xFB,
            NONAME = 0xFC,
            PA1 = 0xFD,
            OEM_CLEAR = 0xFE
        };

        enum class KeyState
		{
			PRESSED,
            HELD,
			RELEASED
		};

        enum class MouseCode
        {
            LEFT = 0x01,
            RIGHT = 0x02,
            MIDDLE = 0x04,
        };

        enum class MouseState
		{
			PRESSED,
            HELD,
			RELEASED
		};

        class InputManager 
        {

        public:

            InputManager(const InputManager&) = delete;
            InputManager& operator=(const InputManager&) = delete;

            void Update() 
            {
                Vector<KeyCode> keysToRemove;
                Vector<MouseCode> buttonsToRemove;

                for (auto& [key, state] : keyStates)
                {
                    if (state == KeyState::PRESSED)
                        keyStates[key] = KeyState::HELD;

                    if (state == KeyState::RELEASED)
                        keysToRemove += key;
                }

                for (auto& [button, state] : mouseButtonStates)
                {
                    if (state == MouseState::PRESSED)
                        mouseButtonStates[button] = MouseState::HELD;

                    if (state == MouseState::RELEASED)
                        buttonsToRemove += button;
                }

                for (auto key : keysToRemove)
                {
                    keyStates -= key;
                }

                for (auto button : buttonsToRemove)
                {
                    mouseButtonStates -= button;
                }

                POINT newMousePosition;
                GetCursorPos(&newMousePosition);

                mouseDelta = Vector2i{ newMousePosition.x - mousePosition.x, newMousePosition.y - mousePosition.y };
                mousePosition = Vector2i{ newMousePosition.x, newMousePosition.y };
            }

            void SetMousePosition(const Vector2i& position) 
            {
                mousePosition = Vector2i{ position.x, position.y };
            }

            void SetKeyState(KeyCode key, KeyState state) 
            {
                keyStates[key] = state;
            }

            void SetMouseButtonState(MouseCode button, MouseState state) 
            {
                mouseButtonStates[button] = MouseState::PRESSED;
            }

            bool GetKeyState(KeyCode key, KeyState state) const 
            {
                return keyStates.Count(key) && keyStates[key] == state;
            }

            bool GetMouseButtonState(MouseCode button, MouseState state) const 
            {
                return mouseButtonStates.Count(button) && mouseButtonStates[button] == state;
            }

            Vector2i GetMousePosition() const 
            {
                return mousePosition;
            }

            Vector2i GetMouseDelta() const 
            {
                return mouseDelta;
            }

            static Shared<InputManager> GetInstance()
            {
                class Enabled : public InputManager { };
                static Shared<InputManager> instance = std::make_shared<Enabled>();

                return instance;
            }

        private:

            InputManager() 
            {
                mousePosition = Vector2i{ 0, 0 };
                mouseDelta = Vector2i{ 0, 0 };

                POINT position = { mousePosition.x, mousePosition.y };

                GetCursorPos(&position);

                mousePosition = Vector2i{ position.x, position.y };
            }

            Map<KeyCode, KeyState> keyStates;
            Map<MouseCode, MouseState> mouseButtonStates;

            Vector2i mousePosition;
            Vector2i mouseDelta;
        };
	}
}