#include <gtest/gtest.h>
#include "RenderStar/Common/Configuration/Configuration.hpp"
#include <filesystem>

using namespace RenderStar::Common::Configuration;

class ConfigurationTest : public ::testing::Test
{
protected:
    std::shared_ptr<pugi::xml_document> doc;
    std::unique_ptr<Configuration> config;

    void SetUp() override
    {
        doc = std::make_shared<pugi::xml_document>();
        doc->append_child("test_ns").append_child("TestClass");
        config = std::make_unique<Configuration>("test_ns", "TestClass", doc);
    }
};

TEST_F(ConfigurationTest, SetAndGetString)
{
    config->SetString("name", "hello");
    auto result = config->GetString("name");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "hello");
}

TEST_F(ConfigurationTest, SetAndGetInteger)
{
    config->SetInteger("count", 42);
    auto result = config->GetInteger("count");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 42);
}

TEST_F(ConfigurationTest, SetAndGetFloat)
{
    config->SetFloat("ratio", 3.14f);
    auto result = config->GetFloat("ratio");
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(*result, 3.14f, 0.01f);
}

TEST_F(ConfigurationTest, SetAndGetBooleanTrue)
{
    config->SetBoolean("enabled", true);
    auto result = config->GetBoolean("enabled");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(*result);
}

TEST_F(ConfigurationTest, SetAndGetBooleanFalse)
{
    config->SetBoolean("disabled", false);
    auto result = config->GetBoolean("disabled");
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(*result);
}

TEST_F(ConfigurationTest, GetStringList)
{
    config->SetString("items", "a, b, c");
    auto result = config->GetStringList("items");
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "a");
    EXPECT_EQ(result[1], "b");
    EXPECT_EQ(result[2], "c");
}

TEST_F(ConfigurationTest, NestedPath)
{
    config->SetString("section.subsection.key", "value");
    auto result = config->GetString("section.subsection.key");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "value");
}

TEST_F(ConfigurationTest, MissingKeyReturnsNullopt)
{
    EXPECT_FALSE(config->GetString("nonexistent").has_value());
    EXPECT_FALSE(config->GetInteger("nonexistent").has_value());
    EXPECT_FALSE(config->GetFloat("nonexistent").has_value());
    EXPECT_FALSE(config->GetBoolean("nonexistent").has_value());
}

TEST_F(ConfigurationTest, NamespaceAndClassScope)
{
    EXPECT_EQ(config->GetNamespace(), "test_ns");
    EXPECT_EQ(config->GetClassScope(), "TestClass");
}

TEST_F(ConfigurationTest, SaveAndReload)
{
    auto tempPath = std::filesystem::temp_directory_path() / "test_config.xml";
    auto fileDoc = std::make_shared<pugi::xml_document>();
    fileDoc->append_child("test_ns").append_child("TestClass");

    Configuration fileConfig("test_ns", "TestClass", fileDoc, tempPath);
    fileConfig.SetString("key", "original");
    fileConfig.Save();

    fileConfig.SetString("key", "modified");
    EXPECT_EQ(*fileConfig.GetString("key"), "modified");

    fileConfig.Reload();
    EXPECT_EQ(*fileConfig.GetString("key"), "original");

    std::filesystem::remove(tempPath);
}

TEST_F(ConfigurationTest, OverwriteExistingValue)
{
    config->SetInteger("val", 10);
    config->SetInteger("val", 20);
    EXPECT_EQ(*config->GetInteger("val"), 20);
}

TEST_F(ConfigurationTest, EmptyStringList)
{
    auto result = config->GetStringList("nonexistent");
    EXPECT_TRUE(result.empty());
}
