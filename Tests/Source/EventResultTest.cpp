#include <gtest/gtest.h>
#include "RenderStar/Common/Event/EventResult.hpp"

using namespace RenderStar::Common::Event;

TEST(EventResultTest, SuccessFactory)
{
    auto result = EventResult::Success();
    EXPECT_EQ(result.type, EventResultType::SUCCESS);
    EXPECT_TRUE(result.message.empty());
}

TEST(EventResultTest, FailureFactoryWithMessage)
{
    auto result = EventResult::Failure("something went wrong");
    EXPECT_EQ(result.type, EventResultType::FAILURE);
    EXPECT_EQ(result.message, "something went wrong");
}

TEST(EventResultTest, FatalFactoryWithMessage)
{
    auto result = EventResult::Fatal("critical error");
    EXPECT_EQ(result.type, EventResultType::FATAL);
    EXPECT_EQ(result.message, "critical error");
}

TEST(EventResultTest, EnumValuesAreDIstinct)
{
    EXPECT_NE(EventResultType::SUCCESS, EventResultType::FAILURE);
    EXPECT_NE(EventResultType::SUCCESS, EventResultType::FATAL);
    EXPECT_NE(EventResultType::FAILURE, EventResultType::FATAL);
}

TEST(EventResultTest, FailureEmptyMessage)
{
    auto result = EventResult::Failure("");
    EXPECT_EQ(result.type, EventResultType::FAILURE);
    EXPECT_TRUE(result.message.empty());
}
