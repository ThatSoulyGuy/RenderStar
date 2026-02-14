#include "RenderStar/Common/Scene/ComponentSerializerRegistry.hpp"

namespace RenderStar::Common::Scene
{
    const std::vector<ComponentSerializerEntry>& ComponentSerializerRegistry::GetSerializers() const
    {
        return entries;
    }

    const ComponentSerializerEntry* ComponentSerializerRegistry::FindByXmlTag(const std::string& tagName) const
    {
        const auto it = tagLookup.find(tagName);

        if (it == tagLookup.end())
            return nullptr;

        return &entries[it->second];
    }

    const ComponentSerializerEntry* ComponentSerializerRegistry::FindByTypeIndex(const std::type_index typeIndex) const
    {
        const auto it = typeLookup.find(typeIndex);

        if (it == typeLookup.end())
            return nullptr;

        return &entries[it->second];
    }
}
