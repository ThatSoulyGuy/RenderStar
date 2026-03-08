#include <gtest/gtest.h>
#include "RenderStar/Client/Gameplay/PlayerController.hpp"

using namespace RenderStar::Client::Gameplay;

TEST(PlayerControllerTest, DefaultValues)
{
    PlayerController pc;
    EXPECT_FLOAT_EQ(pc.lookSensitivity, 0.15f);
    EXPECT_FLOAT_EQ(pc.yaw, -90.0f);
    EXPECT_FLOAT_EQ(pc.pitch, 0.0f);
}

TEST(PlayerControllerTest, ModifyFields)
{
    PlayerController pc;
    pc.lookSensitivity = 0.3f;
    pc.yaw = 45.0f;
    pc.pitch = 30.0f;

    EXPECT_FLOAT_EQ(pc.lookSensitivity, 0.3f);
    EXPECT_FLOAT_EQ(pc.yaw, 45.0f);
    EXPECT_FLOAT_EQ(pc.pitch, 30.0f);
}

TEST(PlayerControllerTest, ReasonableDefaults)
{
    PlayerController pc;
    EXPECT_GT(pc.lookSensitivity, 0.0f);
    EXPECT_LT(pc.lookSensitivity, 1.0f);
}

TEST(PlayerControllerTest, CopySemantics)
{
    PlayerController a;
    a.lookSensitivity = 0.3f;
    PlayerController b = a;
    EXPECT_FLOAT_EQ(b.lookSensitivity, 0.3f);
}
