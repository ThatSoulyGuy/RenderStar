#include <gtest/gtest.h>
#include "RenderStar/Client/Render/Platform/GeometryStage.hpp"
#include "RenderStar/Client/Render/Platform/FullscreenStage.hpp"
#include "RenderStar/Client/Render/Platform/ComputeStage.hpp"

using namespace RenderStar::Client::Render::Platform;

TEST(PipelineStageTest, GeometryStageGetName)
{
    GeometryStage stage("main_geo", "vs", "fs", "scene_color", true);
    EXPECT_EQ(stage.GetName(), "main_geo");
}

TEST(PipelineStageTest, GeometryStageGetOutputTarget)
{
    GeometryStage stage("main_geo", "vs", "fs", "scene_color", true);
    EXPECT_EQ(stage.GetOutputTargetName(), "scene_color");
}

TEST(PipelineStageTest, GeometryStageInputTargetsEmpty)
{
    GeometryStage stage("main_geo", "vs", "fs", "scene_color", true);
    EXPECT_TRUE(stage.GetInputTargetNames().empty());
}

TEST(PipelineStageTest, GeometryStageEnabledByDefault)
{
    GeometryStage stage("main_geo", "vs", "fs", "scene_color", true);
    EXPECT_TRUE(stage.IsEnabled());
}

TEST(PipelineStageTest, GeometryStageSetEnabled)
{
    GeometryStage stage("main_geo", "vs", "fs", "scene_color", true);
    stage.SetEnabled(false);
    EXPECT_FALSE(stage.IsEnabled());
    stage.SetEnabled(true);
    EXPECT_TRUE(stage.IsEnabled());
}

TEST(PipelineStageTest, GeometryStageGetShaderNullBeforeInit)
{
    GeometryStage stage("main_geo", "vs", "fs", "scene_color", true);
    EXPECT_EQ(stage.GetShader(), nullptr);
}

TEST(PipelineStageTest, FullscreenStageGetName)
{
    FullscreenStage stage("tonemap", "vs", "fs", {"scene_color"}, "SWAPCHAIN");
    EXPECT_EQ(stage.GetName(), "tonemap");
}

TEST(PipelineStageTest, FullscreenStageGetOutputTarget)
{
    FullscreenStage stage("tonemap", "vs", "fs", {"scene_color"}, "SWAPCHAIN");
    EXPECT_EQ(stage.GetOutputTargetName(), "SWAPCHAIN");
}

TEST(PipelineStageTest, FullscreenStageInputTargets)
{
    FullscreenStage stage("tonemap", "vs", "fs", {"scene_color"}, "SWAPCHAIN");
    auto inputs = stage.GetInputTargetNames();
    EXPECT_EQ(inputs.size(), 1);
    EXPECT_EQ(inputs[0], "scene_color");
}

TEST(PipelineStageTest, FullscreenStageMultipleInputs)
{
    FullscreenStage stage("compose", "vs", "fs", {"color", "depth"}, "output");
    auto inputs = stage.GetInputTargetNames();
    EXPECT_EQ(inputs.size(), 2);
    EXPECT_EQ(inputs[0], "color");
    EXPECT_EQ(inputs[1], "depth");
}

TEST(PipelineStageTest, FullscreenStageEnabledByDefault)
{
    FullscreenStage stage("tonemap", "vs", "fs", {"scene_color"}, "SWAPCHAIN");
    EXPECT_TRUE(stage.IsEnabled());
}

TEST(PipelineStageTest, FullscreenStageSetEnabled)
{
    FullscreenStage stage("tonemap", "vs", "fs", {"scene_color"}, "SWAPCHAIN");
    stage.SetEnabled(false);
    EXPECT_FALSE(stage.IsEnabled());
    stage.SetEnabled(true);
    EXPECT_TRUE(stage.IsEnabled());
}

TEST(PipelineStageTest, ComputeStageGetName)
{
    ComputeStage stage("blur", "cs", {"input"}, "output", 16, 16, 1);
    EXPECT_EQ(stage.GetName(), "blur");
}

TEST(PipelineStageTest, ComputeStageGetOutputTarget)
{
    ComputeStage stage("blur", "cs", {"input"}, "output", 16, 16, 1);
    EXPECT_EQ(stage.GetOutputTargetName(), "output");
}

TEST(PipelineStageTest, ComputeStageInputTargets)
{
    ComputeStage stage("blur", "cs", {"input"}, "output", 16, 16, 1);
    auto inputs = stage.GetInputTargetNames();
    EXPECT_EQ(inputs.size(), 1);
    EXPECT_EQ(inputs[0], "input");
}

TEST(PipelineStageTest, ComputeStageEnabledByDefault)
{
    ComputeStage stage("blur", "cs", {"input"}, "output", 16, 16, 1);
    EXPECT_TRUE(stage.IsEnabled());
}

TEST(PipelineStageTest, ComputeStageSetEnabled)
{
    ComputeStage stage("blur", "cs", {"input"}, "output", 16, 16, 1);
    stage.SetEnabled(false);
    EXPECT_FALSE(stage.IsEnabled());
    stage.SetEnabled(true);
    EXPECT_TRUE(stage.IsEnabled());
}
