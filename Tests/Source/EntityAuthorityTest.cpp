#include <gtest/gtest.h>
#include "RenderStar/Common/Component/EntityAuthority.hpp"
#include "RenderStar/Common/Component/AuthorityContext.hpp"

using namespace RenderStar::Common::Component;

TEST(EntityAuthorityTest, NobodyFactory)
{
    auto auth = EntityAuthority::Nobody();
    EXPECT_EQ(auth.level, AuthorityLevel::NOBODY);
    EXPECT_EQ(auth.ownerId, -1);
}

TEST(EntityAuthorityTest, ServerFactory)
{
    auto auth = EntityAuthority::Server();
    EXPECT_EQ(auth.level, AuthorityLevel::SERVER);
    EXPECT_EQ(auth.ownerId, -1);
}

TEST(EntityAuthorityTest, ClientFactory)
{
    auto auth = EntityAuthority::Client(42);
    EXPECT_EQ(auth.level, AuthorityLevel::CLIENT);
    EXPECT_EQ(auth.ownerId, 42);
}

TEST(EntityAuthorityTest, NobodyAllowsAnyone)
{
    auto auth = EntityAuthority::Nobody();
    EXPECT_TRUE(auth.CanModify(AuthorityLevel::NOBODY, -1));
    EXPECT_TRUE(auth.CanModify(AuthorityLevel::SERVER, -1));
    EXPECT_TRUE(auth.CanModify(AuthorityLevel::CLIENT, 0));
    EXPECT_TRUE(auth.CanModify(AuthorityLevel::CLIENT, 99));
}

TEST(EntityAuthorityTest, ServerAllowsOnlyServer)
{
    auto auth = EntityAuthority::Server();
    EXPECT_TRUE(auth.CanModify(AuthorityLevel::SERVER, -1));
    EXPECT_FALSE(auth.CanModify(AuthorityLevel::NOBODY, -1));
    EXPECT_FALSE(auth.CanModify(AuthorityLevel::CLIENT, 0));
    EXPECT_FALSE(auth.CanModify(AuthorityLevel::CLIENT, 1));
}

TEST(EntityAuthorityTest, ClientAllowsOnlyMatchingClient)
{
    auto auth = EntityAuthority::Client(5);
    EXPECT_TRUE(auth.CanModify(AuthorityLevel::CLIENT, 5));
    EXPECT_FALSE(auth.CanModify(AuthorityLevel::CLIENT, 6));
    EXPECT_FALSE(auth.CanModify(AuthorityLevel::CLIENT, 0));
    EXPECT_FALSE(auth.CanModify(AuthorityLevel::SERVER, -1));
    EXPECT_FALSE(auth.CanModify(AuthorityLevel::NOBODY, -1));
}

TEST(EntityAuthorityTest, ClientWithDifferentOwnerIds)
{
    auto auth0 = EntityAuthority::Client(0);
    auto auth1 = EntityAuthority::Client(1);

    EXPECT_TRUE(auth0.CanModify(AuthorityLevel::CLIENT, 0));
    EXPECT_FALSE(auth0.CanModify(AuthorityLevel::CLIENT, 1));
    EXPECT_TRUE(auth1.CanModify(AuthorityLevel::CLIENT, 1));
    EXPECT_FALSE(auth1.CanModify(AuthorityLevel::CLIENT, 0));
}

TEST(EntityAuthorityTest, ServerCannotModifyClientOwned)
{
    auto auth = EntityAuthority::Client(3);
    EXPECT_FALSE(auth.CanModify(AuthorityLevel::SERVER, -1));
    EXPECT_FALSE(auth.CanModify(AuthorityLevel::SERVER, 3));
}

TEST(EntityAuthorityTest, DefaultIsNobody)
{
    EntityAuthority auth;
    EXPECT_EQ(auth.level, AuthorityLevel::NOBODY);
    EXPECT_EQ(auth.ownerId, -1);
    EXPECT_TRUE(auth.CanModify(AuthorityLevel::SERVER, -1));
}

TEST(AuthorityContextTest, AsServerFactory)
{
    auto ctx = AuthorityContext::AsServer();
    EXPECT_EQ(ctx.level, AuthorityLevel::SERVER);
    EXPECT_EQ(ctx.ownerId, -1);
}

TEST(AuthorityContextTest, AsClientFactory)
{
    auto ctx = AuthorityContext::AsClient(7);
    EXPECT_EQ(ctx.level, AuthorityLevel::CLIENT);
    EXPECT_EQ(ctx.ownerId, 7);
}

TEST(AuthorityContextTest, AsNobodyFactory)
{
    auto ctx = AuthorityContext::AsNobody();
    EXPECT_EQ(ctx.level, AuthorityLevel::NOBODY);
    EXPECT_EQ(ctx.ownerId, -1);
}

TEST(AuthorityContextTest, DefaultIsNobody)
{
    AuthorityContext ctx;
    EXPECT_EQ(ctx.level, AuthorityLevel::NOBODY);
    EXPECT_EQ(ctx.ownerId, -1);
}

TEST(AuthorityContextTest, ContextMatchesAuthorityCheck)
{
    auto serverAuth = EntityAuthority::Server();
    auto clientAuth = EntityAuthority::Client(2);
    auto nobodyAuth = EntityAuthority::Nobody();

    auto serverCtx = AuthorityContext::AsServer();
    auto clientCtx = AuthorityContext::AsClient(2);
    auto wrongClientCtx = AuthorityContext::AsClient(3);
    auto nobodyCtx = AuthorityContext::AsNobody();

    EXPECT_TRUE(serverAuth.CanModify(serverCtx.level, serverCtx.ownerId));
    EXPECT_FALSE(serverAuth.CanModify(clientCtx.level, clientCtx.ownerId));

    EXPECT_TRUE(clientAuth.CanModify(clientCtx.level, clientCtx.ownerId));
    EXPECT_FALSE(clientAuth.CanModify(wrongClientCtx.level, wrongClientCtx.ownerId));

    EXPECT_TRUE(nobodyAuth.CanModify(nobodyCtx.level, nobodyCtx.ownerId));
    EXPECT_TRUE(nobodyAuth.CanModify(serverCtx.level, serverCtx.ownerId));
    EXPECT_TRUE(nobodyAuth.CanModify(clientCtx.level, clientCtx.ownerId));
}
