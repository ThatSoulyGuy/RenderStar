#pragma once

#include <string>
#include <typeinfo>

#ifdef _MSC_VER
#else
#include <cxxabi.h>
#endif

namespace RenderStar::Common::Utility
{
    class TypeName
    {
    public:

        template <typename T>
        static std::string Get()
        {
            return FromTypeInfo(typeid(T));
        }

        static std::string FromTypeInfo(const std::type_info& typeInfo)
        {
            std::string name = Demangle(typeInfo.name());
            return ExtractSimpleName(name);
        }

        static std::string GetFullName(const std::type_info& typeInfo)
        {
            return Demangle(typeInfo.name());
        }

        template <typename T>
        static std::string GetFull()
        {
            return Demangle(typeid(T).name());
        }

    private:

        static std::string Demangle(const char* mangledName)
        {
#ifdef _MSC_VER
            std::string name = mangledName;

            if (name.starts_with("class "))
                name = name.substr(6);
            else if (name.starts_with("struct "))
                name = name.substr(7);

            return name;
#else
            int status = 0;
            char* demangled = abi::__cxa_demangle(mangledName, nullptr, nullptr, &status);

            if (status == 0 && demangled != nullptr)
            {
                std::string name(demangled);
                free(demangled);
                return name;
            }

            return mangledName;
#endif
        }

        static std::string ExtractSimpleName(const std::string& fullName)
        {
            size_t pos = fullName.rfind("::");

            if (pos != std::string::npos)
                return fullName.substr(pos + 2);

            return fullName;
        }
    };
}
