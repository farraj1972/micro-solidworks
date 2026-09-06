#include "rendering/PointRenderer.h"
#include "rendering/ShaderProgram.h"
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
#include <string_view>
#include <type_traits>
#include <utility>

namespace
{
using microsw::math::Matrix4;
using microsw::math::Vector3;
using microsw::rendering::PointRenderer;
using microsw::rendering::ShaderProgram;

constexpr std::string_view vertexSource = R"(#version 330 core
layout(location = 0) in vec3 aPosition;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
void main() { gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0); }
)";
constexpr std::string_view fragmentSource = R"(#version 330 core
out vec4 FragColor;
void main() { FragColor = vec4(1.0); }
)";

static_assert(!std::is_copy_constructible_v<PointRenderer>);
static_assert(!std::is_copy_assignable_v<PointRenderer>);
static_assert(std::is_nothrow_move_constructible_v<PointRenderer>);
static_assert(std::is_nothrow_move_assignable_v<PointRenderer>);

class PointRendererTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        window = std::make_unique<microsw::ApplicationWindow>(64, 64, "Point rendering tests");
        glfwHideWindow(static_cast<GLFWwindow*>(window->nativeHandle()));
        context = std::make_unique<microsw::OpenGLContext>(*window);
        ASSERT_EQ(glGetError(), GL_NO_ERROR);
    }
    void TearDown() override
    {
        if (context)
        {
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glUseProgram(0);
            EXPECT_EQ(glGetError(), GL_NO_ERROR);
        }
        context.reset();
        window.reset();
    }
    static ShaderProgram boundProgram()
    {
        ShaderProgram program{vertexSource, fragmentSource};
        program.bind();
        for (const char* name : {"uModel", "uView", "uProjection"})
            program.setMatrix4(name, Matrix4::identity());
        return program;
    }
    static GLuint currentVao()
    {
        GLint value{};
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &value);
        return static_cast<GLuint>(value);
    }
    static GLuint attributeBuffer()
    {
        GLint value{};
        glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &value);
        return static_cast<GLuint>(value);
    }
    std::unique_ptr<microsw::ApplicationWindow> window;
    std::unique_ptr<microsw::OpenGLContext> context;
};

TEST_F(PointRendererTest, ConstructionAndEmptyUploadDrawAreSafe)
{
    PointRenderer points;
    EXPECT_EQ(points.vertexCount(), 0U);
    points.draw();
    points.setVertices({});
    points.draw();
    EXPECT_EQ(currentVao(), 0U);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(PointRendererTest, SingleAndMultiplePointsDrawWithGlPointsWithoutErrors)
{
    auto program = boundProgram();
    PointRenderer points;
    const std::array<Vector3, 1> single{Vector3{0, 0, 0}};
    points.setVertices(single);
    EXPECT_EQ(points.vertexCount(), 1U);
    points.draw();
    const std::array<Vector3, 3> multiple{Vector3{-1, 0, 0}, Vector3{0, 1, 0}, Vector3{1, 0, 0}};
    points.setVertices(multiple);
    EXPECT_EQ(points.vertexCount(), 3U);
    points.draw();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(PointRendererTest, EmptyUploadClearsPreviousData)
{
    PointRenderer points;
    const std::array<Vector3, 1> vertex{Vector3{1, 2, 3}};
    points.setVertices(vertex);
    points.setVertices({});
    EXPECT_EQ(points.vertexCount(), 0U);
    points.draw();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(PointRendererTest, RejectsNonFiniteAndFloatOverflowWithoutReplacingData)
{
    PointRenderer points;
    const std::array<Vector3, 1> valid{Vector3{1, 2, 3}};
    points.setVertices(valid);
    for (double value : {std::numeric_limits<double>::quiet_NaN(),
                         std::numeric_limits<double>::infinity(),
                         -std::numeric_limits<double>::infinity(),
                         std::numeric_limits<double>::max(),
                         -std::numeric_limits<double>::max()})
        for (const auto invalid : {Vector3{value, 0, 0}, Vector3{0, value, 0}, Vector3{0, 0, value}})
        {
            const std::array<Vector3, 1> vertices{invalid};
            EXPECT_THROW(points.setVertices(vertices), std::invalid_argument);
            EXPECT_EQ(points.vertexCount(), 1U);
        }
}

TEST_F(PointRendererTest, UploadUsesPackedExplicitFloatConversionAndNoIndexBuffer)
{
    auto program = boundProgram();
    PointRenderer points;
    const std::array<Vector3, 2> vertices{Vector3{1.1, 2.2, 3.3}, Vector3{-4.4, 5.5, -6.6}};
    points.setVertices(vertices);
    points.draw();
    const GLuint buffer = attributeBuffer();
    ASSERT_NE(buffer, 0U);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    GLint size{}, enabled{}, components{}, type{}, stride{}, normalized{}, elementBuffer{};
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_SIZE, &components);
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_TYPE, &type);
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &normalized);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementBuffer);
    EXPECT_EQ(size, 6 * sizeof(GLfloat));
    EXPECT_EQ(enabled, GL_TRUE);
    EXPECT_EQ(components, 3);
    EXPECT_EQ(type, GL_FLOAT);
    EXPECT_EQ(stride, 3 * sizeof(GLfloat));
    EXPECT_EQ(normalized, GL_FALSE);
    EXPECT_EQ(elementBuffer, 0);
    std::array<GLfloat, 6> data{};
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data.data());
    const std::array<double, 6> expected{1.1, 2.2, 3.3, -4.4, 5.5, -6.6};
    for (std::size_t i = 0; i < data.size(); ++i)
        EXPECT_FLOAT_EQ(data[i], static_cast<float>(expected[i]));
}

TEST_F(PointRendererTest, ConstructionUploadAndDrawPreserveRequiredExternalState)
{
    auto program = boundProgram();
    PointRenderer external;
    const std::array<Vector3, 1> vertex{Vector3{1, 2, 3}};
    external.setVertices(vertex);
    external.draw();
    const auto vao = currentVao();
    const auto buffer = attributeBuffer();
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glPointSize(3.0F);
    const auto depth = glIsEnabled(GL_DEPTH_TEST);
    const auto blend = glIsEnabled(GL_BLEND);
    GLint shader{};
    glGetIntegerv(GL_CURRENT_PROGRAM, &shader);
    {
        PointRenderer points;
        EXPECT_EQ(currentVao(), vao);
        points.setVertices(vertex);
        EXPECT_EQ(currentVao(), vao);
        points.draw();
        GLfloat restored{};
        glGetFloatv(GL_POINT_SIZE, &restored);
        EXPECT_FLOAT_EQ(restored, 3.0F);
    }
    GLint currentShader{}, currentBuffer{};
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentShader);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentBuffer);
    EXPECT_EQ(currentShader, shader);
    EXPECT_EQ(static_cast<GLuint>(currentBuffer), buffer);
    EXPECT_EQ(depth, glIsEnabled(GL_DEPTH_TEST));
    EXPECT_EQ(blend, glIsEnabled(GL_BLEND));
}

TEST_F(PointRendererTest, MoveTransfersOwnershipAndDestructionReleasesResources)
{
    auto program = boundProgram();
    const std::array<Vector3, 1> vertex{Vector3{1, 2, 3}};
    GLuint vao{}, buffer{};
    {
        PointRenderer source;
        source.setVertices(vertex);
        source.draw();
        vao = currentVao();
        buffer = attributeBuffer();
        PointRenderer destination{std::move(source)};
        EXPECT_EQ(source.vertexCount(), 0U);
        EXPECT_NO_THROW(source.draw());
        EXPECT_THROW(source.setVertices(vertex), std::logic_error);
        EXPECT_EQ(destination.vertexCount(), 1U);
        destination.draw();
        EXPECT_EQ(currentVao(), vao);
    }
    EXPECT_EQ(glIsVertexArray(vao), GL_FALSE);
    EXPECT_EQ(glIsBuffer(buffer), GL_FALSE);
}

TEST_F(PointRendererTest, MoveAssignmentReleasesPreviousResources)
{
    auto program = boundProgram();
    const std::array<Vector3, 1> vertex{Vector3{1, 2, 3}};
    PointRenderer source;
    source.setVertices(vertex);
    PointRenderer destination;
    destination.setVertices(vertex);
    destination.draw();
    const auto oldVao = currentVao();
    const auto oldBuffer = attributeBuffer();
    destination = std::move(source);
    EXPECT_EQ(glIsVertexArray(oldVao), GL_FALSE);
    EXPECT_EQ(glIsBuffer(oldBuffer), GL_FALSE);
    EXPECT_EQ(destination.vertexCount(), 1U);
}
}
