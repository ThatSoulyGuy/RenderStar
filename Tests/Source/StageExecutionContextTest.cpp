#include <gtest/gtest.h>
#include "RenderStar/Client/Render/Platform/StageExecutionContext.hpp"

using namespace RenderStar::Client::Render;
using namespace RenderStar::Client::Render::Platform;

TEST(StageExecutionContextTest, GetTargetReturnsMatchingTarget)
{
    auto* fakeTarget = reinterpret_cast<IRenderTarget*>(0x1234);
    std::unordered_map<std::string, IRenderTarget*> targets = {{"scene", fakeTarget}};
    StageExecutionContext ctx(nullptr, targets, 0);

    EXPECT_EQ(ctx.GetTarget("scene"), fakeTarget);
}

TEST(StageExecutionContextTest, GetTargetReturnsNullForMissing)
{
    auto* fakeTarget = reinterpret_cast<IRenderTarget*>(0x1234);
    std::unordered_map<std::string, IRenderTarget*> targets = {{"scene", fakeTarget}};
    StageExecutionContext ctx(nullptr, targets, 0);

    EXPECT_EQ(ctx.GetTarget("nonexistent"), nullptr);
}

TEST(StageExecutionContextTest, GetTargetEmptyMap)
{
    std::unordered_map<std::string, IRenderTarget*> targets;
    StageExecutionContext ctx(nullptr, targets, 0);

    EXPECT_EQ(ctx.GetTarget("anything"), nullptr);
}

TEST(StageExecutionContextTest, GetBackendReturnsStoredPointer)
{
    auto* fakeBackend = reinterpret_cast<IRenderingPlatformBackend*>(0x5678);
    std::unordered_map<std::string, IRenderTarget*> targets;
    StageExecutionContext ctx(fakeBackend, targets, 0);

    EXPECT_EQ(ctx.GetBackend(), fakeBackend);
}

TEST(StageExecutionContextTest, GetFrameIndexReturnsStoredValue)
{
    std::unordered_map<std::string, IRenderTarget*> targets;
    StageExecutionContext ctx(nullptr, targets, 42);

    EXPECT_EQ(ctx.GetFrameIndex(), 42);
}

TEST(StageExecutionContextTest, GetFrameIndexZero)
{
    std::unordered_map<std::string, IRenderTarget*> targets;
    StageExecutionContext ctx(nullptr, targets, 0);

    EXPECT_EQ(ctx.GetFrameIndex(), 0);
}

TEST(StageExecutionContextTest, GetFrameIndexNegative)
{
    std::unordered_map<std::string, IRenderTarget*> targets;
    StageExecutionContext ctx(nullptr, targets, -1);

    EXPECT_EQ(ctx.GetFrameIndex(), -1);
}

TEST(StageExecutionContextTest, DrawCommandsInitiallyEmpty)
{
    std::unordered_map<std::string, IRenderTarget*> targets;
    StageExecutionContext ctx(nullptr, targets, 0);

    EXPECT_TRUE(ctx.GetDrawCommands().empty());
}

TEST(StageExecutionContextTest, SubmitDrawCommandsAccumulates)
{
    std::unordered_map<std::string, IRenderTarget*> targets;
    StageExecutionContext ctx(nullptr, targets, 0);

    DrawCommand cmd1;
    cmd1.shader = reinterpret_cast<IShaderProgram*>(0x1);
    ctx.SubmitDrawCommands({cmd1});

    DrawCommand cmd2;
    cmd2.shader = reinterpret_cast<IShaderProgram*>(0x2);
    DrawCommand cmd3;
    cmd3.shader = reinterpret_cast<IShaderProgram*>(0x3);
    ctx.SubmitDrawCommands({cmd2, cmd3});

    EXPECT_EQ(ctx.GetDrawCommands().size(), 3);
}

TEST(StageExecutionContextTest, SubmitEmptyDrawCommands)
{
    std::unordered_map<std::string, IRenderTarget*> targets;
    StageExecutionContext ctx(nullptr, targets, 0);

    ctx.SubmitDrawCommands({});

    EXPECT_TRUE(ctx.GetDrawCommands().empty());
}

TEST(StageExecutionContextTest, DrawCommandPreservesPointers)
{
    std::unordered_map<std::string, IRenderTarget*> targets;
    StageExecutionContext ctx(nullptr, targets, 0);

    auto* fakeShader = reinterpret_cast<IShaderProgram*>(0xA);
    auto* fakeUniform = reinterpret_cast<IUniformBindingHandle*>(0xB);
    auto* fakeMesh = reinterpret_cast<IMesh*>(0xC);

    DrawCommand cmd;
    cmd.shader = fakeShader;
    cmd.uniformBinding = fakeUniform;
    cmd.mesh = fakeMesh;
    ctx.SubmitDrawCommands({cmd});

    const auto& commands = ctx.GetDrawCommands();
    EXPECT_EQ(commands.size(), 1);
    EXPECT_EQ(commands[0].shader, fakeShader);
    EXPECT_EQ(commands[0].uniformBinding, fakeUniform);
    EXPECT_EQ(commands[0].mesh, fakeMesh);
}
