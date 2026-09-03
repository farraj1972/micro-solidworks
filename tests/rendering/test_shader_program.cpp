#include "rendering/ShaderProgram.h"
#include "rendering/LineRenderer.h"
#include "viewer/ReferenceAxes.h"
#include "viewer/OrbitCamera.h"
#include "viewer/ViewProjection.h"
#include "rendering/OpenGLContext.h"
#include "app/window/ApplicationWindow.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

#include <array>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace
{

using microsw::rendering::ShaderProgram;
using microsw::math::Matrix4;
using microsw::math::Vector3;

constexpr std::string_view colorFragmentSource = R"(#version 330 core
uniform vec3 uColor;
uniform vec3 uInactive;
out vec4 FragColor;
void main() { FragColor = vec4(uColor, 1.0); }
)";

constexpr std::string_view vertexSource = R"(#version 330 core
layout(location = 0) in vec3 aPosition;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
void main()
{
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
}
)";

constexpr std::string_view fragmentSource = R"(#version 330 core
out vec4 FragColor;
void main() { FragColor = vec4(1.0); }
)";

static_assert(!std::is_copy_constructible_v<ShaderProgram>);
static_assert(!std::is_copy_assignable_v<ShaderProgram>);
static_assert(std::is_nothrow_move_constructible_v<ShaderProgram>);
static_assert(std::is_nothrow_move_assignable_v<ShaderProgram>);

class ShaderProgramTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Real driver/context, not mocks. No production window API change is needed.
        window = std::make_unique<microsw::ApplicationWindow>(64, 64, "Shader infrastructure tests");
        glfwHideWindow(static_cast<GLFWwindow*>(window->nativeHandle()));
        context = std::make_unique<microsw::OpenGLContext>(*window);
        ASSERT_EQ(glGetError(), GL_NO_ERROR);
    }

    void TearDown() override
    {
        if (context)
        {
            glUseProgram(0);
            EXPECT_EQ(glGetError(), GL_NO_ERROR);
        }
        context.reset();
        window.reset();
    }

    static GLuint currentProgram()
    {
        GLint current{};
        glGetIntegerv(GL_CURRENT_PROGRAM, &current);
        return static_cast<GLuint>(current);
    }

    std::unique_ptr<microsw::ApplicationWindow> window;
    std::unique_ptr<microsw::OpenGLContext> context;
};

TEST_F(ShaderProgramTest, ValidStagesLinkBindAndReleaseStageObjects)
{
    ShaderProgram program{vertexSource, fragmentSource};
    program.bind();
    const GLuint id = currentProgram();
    ASSERT_NE(id, 0u);
    GLint linked{};
    glGetProgramiv(id, GL_LINK_STATUS, &linked);
    EXPECT_EQ(linked, GL_TRUE);
    GLint attached{};
    glGetProgramiv(id, GL_ATTACHED_SHADERS, &attached);
    EXPECT_EQ(attached, 0);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(ShaderProgramTest, InvalidVertexReportsStageAndDriverLog)
{
    try
    {
        ShaderProgram program{"#version 330 core\nvoid main() { this is invalid; }", fragmentSource};
        FAIL() << "Expected vertex compilation failure";
    }
    catch (const std::runtime_error& error)
    {
        const std::string message = error.what();
        const std::string prefix = "vertex shader compilation failed: ";
        EXPECT_EQ(message.find(prefix), 0u);
        EXPECT_GT(message.size(), prefix.size());
    }
}

TEST_F(ShaderProgramTest, InvalidFragmentReportsStageAndDriverLog)
{
    try
    {
        ShaderProgram program{vertexSource, "#version 330 core\nvoid main() { this is invalid; }"};
        FAIL() << "Expected fragment compilation failure";
    }
    catch (const std::runtime_error& error)
    {
        const std::string message = error.what();
        const std::string prefix = "fragment shader compilation failed: ";
        EXPECT_EQ(message.find(prefix), 0u);
        EXPECT_GT(message.size(), prefix.size());
    }
}

TEST_F(ShaderProgramTest, IncompatibleStagesReportLinkLog)
{
    // An active uniform shared by stages must have the same type.
    // Varying vec3/vec4 mismatches were accepted by the validation driver.
    const std::string_view vertex = R"(#version 330 core
uniform mat4 sharedValue;
void main() { gl_Position = sharedValue * vec4(1.0); }
)";
    const std::string_view fragment = R"(#version 330 core
uniform vec4 sharedValue;
out vec4 FragColor;
void main() { FragColor = sharedValue; }
)";
    try
    {
        ShaderProgram program{vertex, fragment};
        FAIL() << "Expected link failure";
    }
    catch (const std::runtime_error& error)
    {
        const std::string message = error.what();
        const std::string prefix = "Shader program link failed: ";
        EXPECT_EQ(message.find(prefix), 0u);
        EXPECT_GT(message.size(), prefix.size());
    }
}

TEST_F(ShaderProgramTest, EmptySourcesFailExplicitly)
{
    EXPECT_THROW((ShaderProgram{"", fragmentSource}), std::invalid_argument);
    EXPECT_THROW((ShaderProgram{vertexSource, ""}), std::invalid_argument);
}

TEST_F(ShaderProgramTest, SourceViewsNeedNotBeNullTerminated)
{
    const std::string vertex = std::string{vertexSource} + "invalid trailing text";
    const std::string fragment = std::string{fragmentSource} + "invalid trailing text";
    ShaderProgram program{
        std::string_view{vertex.data(), vertexSource.size()},
        std::string_view{fragment.data(), fragmentSource.size()}};
    EXPECT_NO_THROW(program.bind());
}

TEST_F(ShaderProgramTest, MoveConstructionTransfersProgram)
{
    ShaderProgram source{vertexSource, fragmentSource};
    source.bind();
    const auto id = currentProgram();
    ShaderProgram destination{std::move(source)};
    destination.bind();
    EXPECT_EQ(currentProgram(), id);
    EXPECT_THROW(source.bind(), std::logic_error);
    EXPECT_THROW(source.setMatrix4("uModel", Matrix4::identity()), std::logic_error);
    EXPECT_NO_THROW(destination.setMatrix4("uModel", Matrix4::identity()));
}

TEST_F(ShaderProgramTest, MoveAssignmentReleasesPreviousProgram)
{
    ShaderProgram source{vertexSource, fragmentSource};
    source.bind();
    const auto sourceId = currentProgram();
    ShaderProgram destination{vertexSource, fragmentSource};
    destination.bind();
    const auto oldId = currentProgram();
    destination = std::move(source);
    EXPECT_EQ(glIsProgram(oldId), GL_FALSE);
    destination.bind();
    EXPECT_EQ(currentProgram(), sourceId);
    EXPECT_THROW(source.bind(), std::logic_error);
    EXPECT_NO_THROW(destination.setMatrix4("uModel", Matrix4::identity()));
}

TEST_F(ShaderProgramTest, DestructionDeletesBoundProgram)
{
    GLuint id{};
    {
        ShaderProgram program{vertexSource, fragmentSource};
        program.bind();
        id = currentProgram();
    }
    EXPECT_EQ(currentProgram(), 0u);
    EXPECT_EQ(glIsProgram(id), GL_FALSE);
}

TEST_F(ShaderProgramTest, DestructionPreservesAnotherBoundProgram)
{
    ShaderProgram survivor{vertexSource, fragmentSource};
    GLuint survivorId{};
    GLuint deletedId{};
    {
        ShaderProgram temporary{vertexSource, fragmentSource};
        temporary.bind();
        deletedId = currentProgram();
        survivor.bind();
        survivorId = currentProgram();
    }
    EXPECT_EQ(currentProgram(), survivorId);
    EXPECT_EQ(glIsProgram(deletedId), GL_FALSE);
}

TEST_F(ShaderProgramTest, MatrixUploadPreservesAllRowsAndColumns)
{
    ShaderProgram program{vertexSource, fragmentSource};
    program.bind();
    // Asymmetric values reveal accidental transposition; fractions exercise conversion.
    const Matrix4 matrix{
        1.1, 2.2, 3.3, 4.4,
        5.5, 6.6, 7.7, 8.8,
        9.9, 10.1, 11.2, 12.3,
        13.4, 14.5, 15.6, 16.7};
    for (const char* name : {"uModel", "uView", "uProjection"})
    {
        program.setMatrix4(name, matrix);
        const GLint location = glGetUniformLocation(currentProgram(), name);
        ASSERT_GE(location, 0);
        std::array<GLfloat, 16> uploaded{};
        glGetUniformfv(currentProgram(), location, uploaded.data());
        for (std::size_t column = 0; column < 4; ++column)
        {
            for (std::size_t row = 0; row < 4; ++row)
            {
                // GL readback is column-major; compare against the explicit float conversion.
                EXPECT_FLOAT_EQ(uploaded[column*4 + row], static_cast<float>(matrix(row, column)));
            }
        }
        EXPECT_EQ(glGetError(), GL_NO_ERROR);
    }
}

TEST_F(ShaderProgramTest, MissingUniformFailsExplicitly)
{
    ShaderProgram program{vertexSource, fragmentSource};
    program.bind();
    EXPECT_THROW(program.setMatrix4("uMissing", Matrix4::identity()), std::invalid_argument);
    EXPECT_THROW(program.setMatrix4("", Matrix4::identity()), std::invalid_argument);
    const std::string badName{"uModel\0suffix", 13};
    EXPECT_THROW(program.setMatrix4(badName, Matrix4::identity()), std::invalid_argument);
}

TEST_F(ShaderProgramTest, UploadRequiresThisProgramBound)
{
    ShaderProgram first{vertexSource, fragmentSource};
    ShaderProgram second{vertexSource, fragmentSource};
    EXPECT_THROW(first.setMatrix4("uModel", Matrix4::identity()), std::logic_error);
    second.bind();
    const auto id = currentProgram();
    EXPECT_THROW(first.setMatrix4("uModel", Matrix4::identity()), std::logic_error);
    EXPECT_EQ(currentProgram(), id);
}

TEST_F(ShaderProgramTest, InvalidMatrixConversionDoesNotUpload)
{
    ShaderProgram program{vertexSource, fragmentSource};
    program.bind();
    program.setMatrix4("uModel", Matrix4::identity());
    for (const double invalid : {std::numeric_limits<double>::infinity(),
                                std::numeric_limits<double>::quiet_NaN(),
                                std::numeric_limits<double>::max()})
    {
        const Matrix4 matrix{
            invalid, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0};
        EXPECT_THROW(program.setMatrix4("uModel", matrix), std::invalid_argument);
    }
    std::array<GLfloat, 16> uploaded{};
    glGetUniformfv(currentProgram(), glGetUniformLocation(currentProgram(), "uModel"), uploaded.data());
    EXPECT_FLOAT_EQ(uploaded[0], 1.0F);
}

TEST_F(ShaderProgramTest, VectorUploadPreservesComponentsAndAcceptsNameViews)
{
    ShaderProgram program{vertexSource, colorFragmentSource};
    program.bind();
    const Vector3 value{0.123456789, -0.7654321, 0.987654321};
    program.setVector3(std::string_view{"uColorTrailing", 6}, value);
    std::array<GLfloat, 3> uploaded{};
    glGetUniformfv(currentProgram(), glGetUniformLocation(currentProgram(), "uColor"), uploaded.data());
    EXPECT_FLOAT_EQ(uploaded[0], static_cast<float>(value.x()));
    EXPECT_FLOAT_EQ(uploaded[1], static_cast<float>(value.y()));
    EXPECT_FLOAT_EQ(uploaded[2], static_cast<float>(value.z()));
}

TEST_F(ShaderProgramTest, VectorUploadRejectsMissingInactiveAndInvalidNames)
{
    ShaderProgram program{vertexSource, colorFragmentSource};
    program.bind();
    EXPECT_THROW(program.setVector3("uMissing", {}), std::invalid_argument);
    EXPECT_THROW(program.setVector3("uInactive", {}), std::invalid_argument);
    EXPECT_THROW(program.setVector3("", {}), std::invalid_argument);
    const std::string badName{"uColor\0suffix", 13};
    EXPECT_THROW(program.setVector3(badName, {}), std::invalid_argument);
}

TEST_F(ShaderProgramTest, VectorUploadRequiresThisProgramBound)
{
    ShaderProgram first{vertexSource, colorFragmentSource};
    ShaderProgram second{vertexSource, colorFragmentSource};
    EXPECT_THROW(first.setVector3("uColor", {}), std::logic_error);
    second.bind();
    const auto id = currentProgram();
    EXPECT_THROW(first.setVector3("uColor", {}), std::logic_error);
    EXPECT_EQ(currentProgram(), id);
}

TEST_F(ShaderProgramTest, VectorUploadRejectsMovedFromProgram)
{
    ShaderProgram source{vertexSource, colorFragmentSource};
    ShaderProgram destination{std::move(source)};
    destination.bind();
    EXPECT_THROW(source.setVector3("uColor", {}), std::logic_error);
    EXPECT_NO_THROW(destination.setVector3("uColor", {1.0, 0.0, 0.0}));
}

TEST_F(ShaderProgramTest, InvalidVectorConversionDoesNotUpload)
{
    ShaderProgram program{vertexSource, colorFragmentSource};
    program.bind();
    program.setVector3("uColor", {0.25, 0.5, 0.75});
    for (const double invalid : {std::numeric_limits<double>::quiet_NaN(),
                                std::numeric_limits<double>::infinity(),
                                -std::numeric_limits<double>::infinity(),
                                std::numeric_limits<double>::max(),
                                -std::numeric_limits<double>::max()})
    {
        for (const Vector3 value : {Vector3{invalid, 0.0, 0.0},
                                   Vector3{0.0, invalid, 0.0},
                                   Vector3{0.0, 0.0, invalid}})
        {
            EXPECT_THROW(program.setVector3("uColor", value), std::invalid_argument);
            std::array<GLfloat, 3> uploaded{};
            glGetUniformfv(currentProgram(), glGetUniformLocation(currentProgram(), "uColor"), uploaded.data());
            EXPECT_FLOAT_EQ(uploaded[0], 0.25F);
            EXPECT_FLOAT_EQ(uploaded[1], 0.5F);
            EXPECT_FLOAT_EQ(uploaded[2], 0.75F);
        }
    }
}

TEST_F(ShaderProgramTest, VectorUploadAcceptsFloatRangeLimits)
{
    ShaderProgram program{vertexSource, colorFragmentSource};
    program.bind();
    const double limit = std::numeric_limits<float>::max();
    program.setVector3("uColor", {limit, -limit, 0.0});
    std::array<GLfloat, 3> uploaded{};
    glGetUniformfv(currentProgram(), glGetUniformLocation(currentProgram(), "uColor"), uploaded.data());
    EXPECT_FLOAT_EQ(uploaded[0], std::numeric_limits<float>::max());
    EXPECT_FLOAT_EQ(uploaded[1], -std::numeric_limits<float>::max());
    EXPECT_FLOAT_EQ(uploaded[2], 0.0F);
}

TEST_F(ShaderProgramTest, ReferenceAxisBatchesDrawWithoutOpenGLErrors)
{
    const microsw::viewer::ReferenceAxes axes;
    std::array<microsw::rendering::LineRenderer, 3> batches;
    batches[0].setVertices(axes.xAxis());
    batches[1].setVertices(axes.yAxis());
    batches[2].setVertices(axes.zAxis());
    const std::array<Vector3, 3> colors{
        Vector3{1.0, 0.0, 0.0}, Vector3{0.0, 1.0, 0.0}, Vector3{0.0, 0.0, 1.0}};
    ShaderProgram program{vertexSource, colorFragmentSource};
    program.bind();
    program.setMatrix4("uModel", Matrix4::identity());
    program.setMatrix4("uView", microsw::viewer::viewMatrix(microsw::viewer::OrbitCamera{}));
    program.setMatrix4("uProjection", microsw::viewer::perspective(1.0, 1.0, 0.1, 100.0));
    glViewport(0, 0, 64, 64);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ASSERT_EQ(glGetError(), GL_NO_ERROR);
    for (std::size_t index = 0; index < batches.size(); ++index)
    {
        SCOPED_TRACE(index);
        program.setVector3("uColor", colors[index]);
        batches[index].draw();
        EXPECT_EQ(glGetError(), GL_NO_ERROR);
        EXPECT_EQ(glIsEnabled(GL_DEPTH_TEST), GL_TRUE);
        std::array<GLfloat, 3> uploaded{};
        glGetUniformfv(currentProgram(), glGetUniformLocation(currentProgram(), "uColor"), uploaded.data());
        EXPECT_FLOAT_EQ(uploaded[0], static_cast<float>(colors[index].x()));
        EXPECT_FLOAT_EQ(uploaded[1], static_cast<float>(colors[index].y()));
        EXPECT_FLOAT_EQ(uploaded[2], static_cast<float>(colors[index].z()));
    }
}

TEST_F(ShaderProgramTest, RepeatedFailureDoesNotPreventSubsequentSuccess)
{
    for (int attempt = 0; attempt < 3; ++attempt)
    {
        EXPECT_THROW((ShaderProgram{vertexSource, "#version 330 core\ninvalid"}), std::runtime_error);
        ShaderProgram program{vertexSource, fragmentSource};
        program.bind();
        program.setMatrix4("uModel", Matrix4::identity());
    }
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

}
