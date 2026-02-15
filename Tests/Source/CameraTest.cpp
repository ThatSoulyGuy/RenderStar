#include <gtest/gtest.h>
#include "RenderStar/Client/Render/Components/Camera.hpp"

using namespace RenderStar::Client::Render::Components;

TEST(CameraTest, DefaultValues)
{
    Camera cam;
    EXPECT_EQ(cam.projectionType, ProjectionType::PERSPECTIVE);
    EXPECT_FLOAT_EQ(cam.fieldOfView, 60.0f);
    EXPECT_FLOAT_EQ(cam.nearPlane, 0.1f);
    EXPECT_FLOAT_EQ(cam.farPlane, 1000.0f);
}

TEST(CameraTest, CreatePerspective)
{
    auto cam = Camera::CreatePerspective(90.0f, 16.0f / 9.0f, 0.1f, 500.0f);
    EXPECT_EQ(cam.projectionType, ProjectionType::PERSPECTIVE);
    EXPECT_FLOAT_EQ(cam.fieldOfView, 90.0f);
    EXPECT_FLOAT_EQ(cam.aspectRatio, 16.0f / 9.0f);
    EXPECT_FLOAT_EQ(cam.nearPlane, 0.1f);
    EXPECT_FLOAT_EQ(cam.farPlane, 500.0f);
}

TEST(CameraTest, CreateOrthographic)
{
    auto cam = Camera::CreateOrthographic(5.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    EXPECT_EQ(cam.projectionType, ProjectionType::ORTHOGRAPHIC);
    EXPECT_FLOAT_EQ(cam.orthographicSize, 5.0f);
    EXPECT_FLOAT_EQ(cam.nearPlane, 0.1f);
    EXPECT_FLOAT_EQ(cam.farPlane, 100.0f);
}

TEST(CameraTest, GetViewMatrix)
{
    Camera cam;
    auto view = cam.GetViewMatrix();
    EXPECT_EQ(view, glm::mat4(1.0f));
}

TEST(CameraTest, GetProjectionMatrix)
{
    auto cam = Camera::CreatePerspective(60.0f, 1.0f, 0.1f, 100.0f);
    auto proj = cam.GetProjectionMatrix();
    EXPECT_NE(proj, glm::mat4(1.0f));
}

TEST(CameraTest, ViewProjectionProduct)
{
    auto cam = Camera::CreatePerspective(60.0f, 1.0f, 0.1f, 100.0f);
    auto vp = cam.GetViewProjectionMatrix();
    auto expected = cam.GetProjectionMatrix() * cam.GetViewMatrix();

    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            EXPECT_FLOAT_EQ(vp[i][j], expected[i][j]);
}

TEST(CameraTest, PerspectiveProjectionNotIdentity)
{
    auto cam = Camera::CreatePerspective(60.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    EXPECT_NE(cam.projectionMatrix, glm::mat4(1.0f));
}

TEST(CameraTest, OrthographicProjectionNotIdentity)
{
    auto cam = Camera::CreateOrthographic(10.0f, 1.0f, 0.1f, 100.0f);
    EXPECT_NE(cam.projectionMatrix, glm::mat4(1.0f));
}
