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

TEST(EntityIdRemapperTest, GetServerIdForLocalId)
{
    EntityIdRemapper remapper;
    remapper.RecordMapping(100, GameObject{5});

    EXPECT_EQ(remapper.GetServerIdForLocalId(5), 100);
}

TEST(EntityIdRemapperTest, GetServerIdForLocalIdNotFound)
{
    EntityIdRemapper remapper;
    EXPECT_EQ(remapper.GetServerIdForLocalId(999), -1);
}

TEST(EntityIdRemapperTest, GetServerIdForLocalIdMultipleMappings)
{
    EntityIdRemapper remapper;
    remapper.RecordMapping(10, GameObject{0});
    remapper.RecordMapping(20, GameObject{1});
    remapper.RecordMapping(30, GameObject{2});

    EXPECT_EQ(remapper.GetServerIdForLocalId(0), 10);
    EXPECT_EQ(remapper.GetServerIdForLocalId(1), 20);
    EXPECT_EQ(remapper.GetServerIdForLocalId(2), 30);
    EXPECT_EQ(remapper.GetServerIdForLocalId(3), -1);
}

TEST(EntityIdRemapperTest, ReverseAndForwardConsistent)
{
    EntityIdRemapper remapper;
    remapper.RecordMapping(50, GameObject{200});

    auto forward = remapper.Remap(50);
    EXPECT_EQ(forward.id, 200);

    auto reverse = remapper.GetServerIdForLocalId(200);
    EXPECT_EQ(reverse, 50);
}

TEST(EntityIdRemapperTest, OverwriteMappingUpdatesReverse)
{
    EntityIdRemapper remapper;
    remapper.RecordMapping(1, GameObject{10});
    remapper.RecordMapping(1, GameObject{20});

    EXPECT_EQ(remapper.GetServerIdForLocalId(20), 1);
}

TEST(EntityIdRemapperTest, HasMappingAfterMultipleRecords)
{
    EntityIdRemapper remapper;
    remapper.RecordMapping(1, GameObject{10});
    remapper.RecordMapping(2, GameObject{20});

    EXPECT_TRUE(remapper.HasMapping(1));
    EXPECT_TRUE(remapper.HasMapping(2));
    EXPECT_FALSE(remapper.HasMapping(3));
}

TEST(EntityIdRemapperTest, EmptyRemapperHasNoMappings)
{
    EntityIdRemapper remapper;
    EXPECT_FALSE(remapper.HasMapping(0));
    EXPECT_EQ(remapper.GetServerIdForLocalId(0), -1);
    EXPECT_FALSE(remapper.Remap(0).IsValid());
    EXPECT_TRUE(remapper.GetAllMappings().empty());
}
