#include <gtest/gtest.h>
#include "RenderStar/Common/Utility/TypeName.hpp"
#include <vector>

using namespace RenderStar::Common::Utility;

namespace TestNamespace
{
    struct MyStruct {};
}

TEST(TypeNameTest, PrimitiveType)
{
    auto name = TypeName::Get<int>();
    EXPECT_FALSE(name.empty());
    EXPECT_TRUE(name.find("int") != std::string::npos);
}

TEST(TypeNameTest, ClassType)
{
    auto name = TypeName::Get<TestNamespace::MyStruct>();
    EXPECT_EQ(name, "MyStruct");
}

TEST(TypeNameTest, FullNameContainsNamespace)
{
    auto name = TypeName::GetFull<TestNamespace::MyStruct>();
    EXPECT_TRUE(name.find("TestNamespace") != std::string::npos);
    EXPECT_TRUE(name.find("MyStruct") != std::string::npos);
}

TEST(TypeNameTest, ConsistentResults)
{
    auto first = TypeName::Get<float>();
    auto second = TypeName::Get<float>();
    EXPECT_EQ(first, second);
}

TEST(TypeNameTest, DifferentTypesProduceDifferentNames)
{
    auto intName = TypeName::Get<int>();
    auto floatName = TypeName::Get<float>();
    EXPECT_NE(intName, floatName);
}

TEST(TypeNameTest, FromTypeInfo)
{
    auto name = TypeName::FromTypeInfo(typeid(TestNamespace::MyStruct));
    EXPECT_EQ(name, "MyStruct");
}
