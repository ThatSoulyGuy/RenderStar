#include <gtest/gtest.h>
#include "RenderStar/Common/Utility/TypeName.hpp"

using RenderStar::Common::Utility::TypeName;

namespace
{
    class SimpleClass {};

    struct SimpleStruct {};

    namespace Nested
    {
        class NestedClass {};

        namespace Deeply
        {
            struct DeeplyNestedStruct {};
        }
    }

    template<typename T>
    class TemplatedClass {};
}

TEST(TypeNameTest, GetSimpleClassName)
{
    std::string name = TypeName::Get<SimpleClass>();
    EXPECT_EQ(name, "SimpleClass");
}

TEST(TypeNameTest, GetSimpleStructName)
{
    std::string name = TypeName::Get<SimpleStruct>();
    EXPECT_EQ(name, "SimpleStruct");
}

TEST(TypeNameTest, GetNestedClassName)
{
    std::string name = TypeName::Get<Nested::NestedClass>();
    EXPECT_EQ(name, "NestedClass");
}

TEST(TypeNameTest, GetDeeplyNestedStructName)
{
    std::string name = TypeName::Get<Nested::Deeply::DeeplyNestedStruct>();
    EXPECT_EQ(name, "DeeplyNestedStruct");
}

TEST(TypeNameTest, GetFullNameIncludesNamespace)
{
    std::string fullName = TypeName::GetFull<Nested::NestedClass>();
    EXPECT_TRUE(fullName.find("Nested") != std::string::npos);
    EXPECT_TRUE(fullName.find("NestedClass") != std::string::npos);
}

TEST(TypeNameTest, GetFullNameForDeeplyNested)
{
    std::string fullName = TypeName::GetFull<Nested::Deeply::DeeplyNestedStruct>();
    EXPECT_TRUE(fullName.find("Nested") != std::string::npos);
    EXPECT_TRUE(fullName.find("Deeply") != std::string::npos);
    EXPECT_TRUE(fullName.find("DeeplyNestedStruct") != std::string::npos);
}

TEST(TypeNameTest, GetPrimitiveTypeName)
{
    std::string intName = TypeName::Get<int>();
    EXPECT_EQ(intName, "int");
}

TEST(TypeNameTest, FromTypeInfoMatchesTemplateVersion)
{
    std::string templateName = TypeName::Get<SimpleClass>();
    std::string typeInfoName = TypeName::FromTypeInfo(typeid(SimpleClass));
    EXPECT_EQ(templateName, typeInfoName);
}

TEST(TypeNameTest, GetFullNameFromTypeInfo)
{
    std::string fullName = TypeName::GetFullName(typeid(Nested::NestedClass));
    EXPECT_TRUE(fullName.find("NestedClass") != std::string::npos);
}

TEST(TypeNameTest, PointerTypeContainsAsterisk)
{
    std::string name = TypeName::GetFull<int*>();
    EXPECT_TRUE(name.find("int") != std::string::npos);
}

TEST(TypeNameTest, ReferenceTypeHandled)
{
    std::string name = TypeName::GetFull<int&>();
    EXPECT_TRUE(name.find("int") != std::string::npos);
}

TEST(TypeNameTest, ConstTypeHandled)
{
    std::string name = TypeName::GetFull<const int>();
    EXPECT_TRUE(name.find("int") != std::string::npos);
}
