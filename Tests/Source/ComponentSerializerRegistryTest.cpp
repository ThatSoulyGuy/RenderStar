#include <gtest/gtest.h>
#include "RenderStar/Common/Scene/ComponentSerializerRegistry.hpp"

using namespace RenderStar::Common::Scene;

struct SerTestComp
{
    int value;
};

struct OtherSerComp
{
    float data;
};

class ComponentSerializerRegistryTest : public ::testing::Test
{
protected:
    ComponentSerializerRegistry registry;

    ComponentSerializer<SerTestComp> MakeSerializer()
    {
        return ComponentSerializer<SerTestComp>{
            [](const SerTestComp&, pugi::xml_node&) {},
            [](const pugi::xml_node&) -> SerTestComp { return {0}; },
            nullptr
        };
    }
};

TEST_F(ComponentSerializerRegistryTest, RegisterAndFindByTag)
{
    registry.Register<SerTestComp>("test_comp", MakeSerializer());
    auto* entry = registry.FindByXmlTag("test_comp");
    ASSERT_NE(entry, nullptr);
    EXPECT_EQ(entry->xmlTagName, "test_comp");
}

TEST_F(ComponentSerializerRegistryTest, FindByTagUnknown)
{
    auto* entry = registry.FindByXmlTag("nonexistent");
    EXPECT_EQ(entry, nullptr);
}

TEST_F(ComponentSerializerRegistryTest, FindByTypeIndex)
{
    registry.Register<SerTestComp>("test_comp", MakeSerializer());
    auto* entry = registry.FindByTypeIndex(std::type_index(typeid(SerTestComp)));
    ASSERT_NE(entry, nullptr);
    EXPECT_EQ(entry->xmlTagName, "test_comp");
}

TEST_F(ComponentSerializerRegistryTest, GetAllSerializers)
{
    registry.Register<SerTestComp>("test_comp", MakeSerializer());
    auto& entries = registry.GetSerializers();
    EXPECT_EQ(entries.size(), 1);
}

TEST_F(ComponentSerializerRegistryTest, MultipleSerializers)
{
    registry.Register<SerTestComp>("test_comp", MakeSerializer());
    registry.Register<OtherSerComp>("other_comp", ComponentSerializer<OtherSerComp>{
        [](const OtherSerComp&, pugi::xml_node&) {},
        [](const pugi::xml_node&) -> OtherSerComp { return {0.0f}; },
        nullptr
    });

    EXPECT_EQ(registry.GetSerializers().size(), 2);
    EXPECT_NE(registry.FindByXmlTag("test_comp"), nullptr);
    EXPECT_NE(registry.FindByXmlTag("other_comp"), nullptr);
}

TEST_F(ComponentSerializerRegistryTest, TypeIndexForDifferentTypes)
{
    registry.Register<SerTestComp>("test_comp", MakeSerializer());

    EXPECT_NE(registry.FindByTypeIndex(std::type_index(typeid(SerTestComp))), nullptr);
    EXPECT_EQ(registry.FindByTypeIndex(std::type_index(typeid(OtherSerComp))), nullptr);
}
