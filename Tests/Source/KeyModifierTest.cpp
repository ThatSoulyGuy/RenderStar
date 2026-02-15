#include <gtest/gtest.h>
#include "RenderStar/Client/Input/KeyModifier.hpp"

using namespace RenderStar::Client::Input;

TEST(KeyModifierTest, NoneIsZero)
{
    EXPECT_EQ(static_cast<uint8_t>(KeyModifier::NONE), 0x00);
}

TEST(KeyModifierTest, BitwiseOrCombines)
{
    auto combined = KeyModifier::SHIFT | KeyModifier::CTRL;
    EXPECT_EQ(static_cast<uint8_t>(combined), 0x03);
}

TEST(KeyModifierTest, BitwiseAndExtracts)
{
    auto combined = KeyModifier::SHIFT | KeyModifier::CTRL | KeyModifier::ALT;
    auto extracted = combined & KeyModifier::CTRL;
    EXPECT_EQ(extracted, KeyModifier::CTRL);
}

TEST(KeyModifierTest, HasModifierSingle)
{
    auto flags = KeyModifier::SHIFT | KeyModifier::CTRL;
    EXPECT_TRUE(HasModifier(flags, KeyModifier::SHIFT));
    EXPECT_TRUE(HasModifier(flags, KeyModifier::CTRL));
    EXPECT_FALSE(HasModifier(flags, KeyModifier::ALT));
    EXPECT_FALSE(HasModifier(flags, KeyModifier::SUPER));
}

TEST(KeyModifierTest, AllValuesDistinct)
{
    EXPECT_NE(KeyModifier::SHIFT, KeyModifier::CTRL);
    EXPECT_NE(KeyModifier::SHIFT, KeyModifier::ALT);
    EXPECT_NE(KeyModifier::SHIFT, KeyModifier::SUPER);
    EXPECT_NE(KeyModifier::CTRL, KeyModifier::ALT);
    EXPECT_NE(KeyModifier::CTRL, KeyModifier::SUPER);
    EXPECT_NE(KeyModifier::ALT, KeyModifier::SUPER);
}

TEST(KeyModifierTest, HasModifierNoneAlwaysTrue)
{
    EXPECT_TRUE(HasModifier(KeyModifier::SHIFT, KeyModifier::NONE));
    EXPECT_TRUE(HasModifier(KeyModifier::NONE, KeyModifier::NONE));
}
