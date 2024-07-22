#pragma once

#include <stdexcept>
#include <xhash>
#include "RenderStar/Util/Core/Map.hpp"
#include "RenderStar/Util/Core/String.hpp"
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::Util;
using namespace RenderStar::Util::Core;

namespace std 
{
    template<>
    struct hash<String>
    {
        std::size_t operator()(const String& s) const noexcept
        {
            std::string raw = s;
            return std::hash<std::string>()(raw);
        }
    };
}

namespace RenderStar
{
	namespace Core
	{
        class Settings
        {

        public:

            Settings(const Settings&) = delete;
            Settings& operator=(const Settings&) = delete;

            template<typename T>
            void Set(const String& key, const T& value)
            {
                settings[key] = std::make_shared<Holder<T>>(value);
            }

            bool Contains(const String& key) const
			{
				return settings.Contains(key);
			}

            template<typename T>
            T Get(const String& key) const
            {
                if (!settings.Contains(key))
                    return T();

                auto holder = std::dynamic_pointer_cast<Holder<T>>(settings[key]);

                if (!holder)
                    throw std::runtime_error("Failed to cast value to type '" + std::string(typeid(T).name()) + "'");

                return holder->value;
            }

            void Remove(const String& key)
            {
                settings.Remove(key);
            }

            static Shared<Settings> GetInstance()
            {
                class Enabled : public Settings { };
                static Shared<Settings> instance = std::make_shared<Enabled>();
                return instance;
            }

        private:

            Settings() = default;

            struct HolderBase
            {
                virtual ~HolderBase() = default;
            };

            template<typename T>
            struct Holder : HolderBase
            {
                Holder(const T& value) : value(value) {}

                T value;
            };

            Map<String, Shared<HolderBase>> settings;
        };
	}
}