#include <gtest/gtest.h>
#include "RenderStar/Common/Component/Components/Transform.hpp"

using namespace RenderStar::Common::Component;

TEST(TransformTest, DefaultPosition)
{
    Transform t;
    EXPECT_FLOAT_EQ(t.position.x, 0.0f);
    EXPECT_FLOAT_EQ(t.position.y, 0.0f);
    EXPECT_FLOAT_EQ(t.position.z, 0.0f);
}

TEST(TransformTest, DefaultRotationIsIdentity)
{
    Transform t;
    EXPECT_FLOAT_EQ(t.rotation.w, 1.0f);
    EXPECT_FLOAT_EQ(t.rotation.x, 0.0f);
    EXPECT_FLOAT_EQ(t.rotation.y, 0.0f);
    EXPECT_FLOAT_EQ(t.rotation.z, 0.0f);
}

TEST(TransformTest, DefaultScale)
{
    Transform t;
    EXPECT_FLOAT_EQ(t.scale.x, 1.0f);
    EXPECT_FLOAT_EQ(t.scale.y, 1.0f);
    EXPECT_FLOAT_EQ(t.scale.z, 1.0f);
}

TEST(TransformTest, DefaultLocalMatrixIsIdentity)
{
    Transform t;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            EXPECT_FLOAT_EQ(t.localMatrix[i][j], i == j ? 1.0f : 0.0f);
}

TEST(TransformTest, DefaultWorldMatrixIsIdentity)
{
    Transform t;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            EXPECT_FLOAT_EQ(t.worldMatrix[i][j], i == j ? 1.0f : 0.0f);
}

TEST(TransformTest, ModifyFields)
{
    Transform t;
    t.position = glm::vec3(1.0f, 2.0f, 3.0f);
    t.scale = glm::vec3(2.0f, 2.0f, 2.0f);

    EXPECT_FLOAT_EQ(t.position.x, 1.0f);
    EXPECT_FLOAT_EQ(t.position.y, 2.0f);
    EXPECT_FLOAT_EQ(t.position.z, 3.0f);
    EXPECT_FLOAT_EQ(t.scale.x, 2.0f);
}
