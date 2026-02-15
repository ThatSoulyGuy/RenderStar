#include <gtest/gtest.h>
#include "RenderStar/Common/Scene/EntityIdRemapper.hpp"

using namespace RenderStar::Common::Scene;
using namespace RenderStar::Common::Component;

TEST(EntityIdRemapperTest, RecordAndRemap)
{
    EntityIdRemapper remapper;
    remapper.RecordMapping(1, GameObject{10});

    auto result = remapper.Remap(1);
    EXPECT_EQ(result.id, 10);
}

TEST(EntityIdRemapperTest, RemapUnrecordedReturnsInvalid)
{
    EntityIdRemapper remapper;
    auto result = remapper.Remap(999);
    EXPECT_EQ(result.id, GameObject::INVALID_ID);
    EXPECT_FALSE(result.IsValid());
}

TEST(EntityIdRemapperTest, HasMapping)
{
    EntityIdRemapper remapper;
    remapper.RecordMapping(5, GameObject{50});

    EXPECT_TRUE(remapper.HasMapping(5));
    EXPECT_FALSE(remapper.HasMapping(6));
}

TEST(EntityIdRemapperTest, GetAllMappings)
{
    EntityIdRemapper remapper;
    remapper.RecordMapping(1, GameObject{10});
    remapper.RecordMapping(2, GameObject{20});

    auto& mappings = remapper.GetAllMappings();
    EXPECT_EQ(mappings.size(), 2);
    EXPECT_EQ(mappings.at(1).id, 10);
    EXPECT_EQ(mappings.at(2).id, 20);
}

TEST(EntityIdRemapperTest, MultipleMappings)
{
    EntityIdRemapper remapper;
    remapper.RecordMapping(0, GameObject{100});
    remapper.RecordMapping(1, GameObject{101});
    remapper.RecordMapping(2, GameObject{102});

    EXPECT_EQ(remapper.Remap(0).id, 100);
    EXPECT_EQ(remapper.Remap(1).id, 101);
    EXPECT_EQ(remapper.Remap(2).id, 102);
}

TEST(EntityIdRemapperTest, OverwriteMapping)
{
    EntityIdRemapper remapper;
    remapper.RecordMapping(1, GameObject{10});
    remapper.RecordMapping(1, GameObject{20});

    EXPECT_EQ(remapper.Remap(1).id, 20);
}
