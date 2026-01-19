#pragma once

#include <typeindex>
#include <string_view>

namespace RenderStar::Common::Event
{
    class IEvent
    {
    public:

        virtual ~IEvent() = default;

        virtual std::type_index GetTypeIndex() const = 0;

        virtual std::string_view GetName() const = 0;
    };

    template <typename DerivedType>
    class Event : public IEvent
    {
    public:

        std::type_index GetTypeIndex() const final
        {
            return std::type_index(typeid(DerivedType));
        }
    };
}
