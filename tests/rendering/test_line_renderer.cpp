#include "rendering/LineRenderer.h"
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

using microsw::math::Vector3;
using microsw::math::Matrix4;
using microsw::rendering::LineRenderer;
using microsw::rendering::ShaderProgram;

const std::array<Vector3, 2> segment{Vector3{-0.5, 0.0, 0.0}, Vector3{0.5, 0.0, 0.0}};
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
void main() { FragColor = vec4(1.0, 1.0, 1.0, 1.0); }
)";

static_assert(!std::is_copy_constructible_v<LineRenderer>);
static_assert(!std::is_copy_assignable_v<LineRenderer>);
static_assert(std::is_nothrow_move_constructible_v<LineRenderer>);
static_assert(std::is_nothrow_move_assignable_v<LineRenderer>);

class LineRendererTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        window = std::make_unique<microsw::ApplicationWindow>(64, 64, "Line rendering tests");
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

TEST_F(LineRendererTest, DefaultAndEmptyUploadAreSafeWithoutShader)
{
    LineRenderer lines;
    EXPECT_EQ(lines.vertexCount(), 0u);
    lines.draw();
    lines.setVertices({});
    lines.draw();
    EXPECT_EQ(lines.vertexCount(), 0u);
    EXPECT_EQ(currentVao(), 0u);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(LineRendererTest, TwoVertexUploadDrawsWithIdentityUniforms)
{
    auto program = boundProgram();
    LineRenderer lines;
    lines.setVertices(segment);
    EXPECT_EQ(lines.vertexCount(), 2u);
    lines.draw();
    EXPECT_NE(currentVao(), 0u);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(LineRendererTest, MultipleSegmentsReplacePreviousUpload)
{
    auto program = boundProgram();
    LineRenderer lines;
    const std::array<Vector3, 4> vertices{
        Vector3{-0.8, -0.3, 0.0}, Vector3{0.8, -0.3, 0.0},
        Vector3{-0.2, 0.6, 0.0}, Vector3{0.3, 0.4, 0.0}};
    lines.setVertices(vertices);
    EXPECT_EQ(lines.vertexCount(), 4u);
    lines.draw();
    lines.setVertices(segment);
    EXPECT_EQ(lines.vertexCount(), 2u);
    lines.draw();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(LineRendererTest, EmptyUploadClearsPreviousVertices)
{
    LineRenderer lines;
    lines.setVertices(segment);
    lines.setVertices({});
    EXPECT_EQ(lines.vertexCount(), 0u);
    // Clearing must remain safe even with no shader supplied by the caller.
    lines.draw();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(LineRendererTest, OddVertexCountPreservesPreviousData)
{
    auto program = boundProgram();
    LineRenderer lines;
    lines.setVertices(segment);
    const std::array<Vector3, 1> odd{Vector3{}};
    EXPECT_THROW(lines.setVertices(odd), std::invalid_argument);
    EXPECT_EQ(lines.vertexCount(), 2u);
    lines.draw();
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(LineRendererTest, EveryNonFiniteCoordinateIsRejected)
{
    LineRenderer lines;
    lines.setVertices(segment);
    for (const double value : {std::numeric_limits<double>::quiet_NaN(),
                               std::numeric_limits<double>::infinity(),
                               -std::numeric_limits<double>::infinity()})
    {
        for (const auto invalid : {Vector3{value, 0.0, 0.0},
                                   Vector3{0.0, value, 0.0}, Vector3{0.0, 0.0, value}})
        {
            const std::array<Vector3, 2> vertices{Vector3{}, invalid};
            EXPECT_THROW(lines.setVertices(vertices), std::invalid_argument);
            EXPECT_EQ(lines.vertexCount(), 2u);
        }
    }
}

TEST_F(LineRendererTest, FloatOverflowIsRejectedInEachCoordinate)
{
    LineRenderer lines;
    for (const double value : {std::numeric_limits<double>::max(), -std::numeric_limits<double>::max()})
    {
        for (const auto invalid : {Vector3{value, 0.0, 0.0},
                                   Vector3{0.0, value, 0.0}, Vector3{0.0, 0.0, value}})
        {
            const std::array<Vector3, 2> vertices{invalid, Vector3{}};
            EXPECT_THROW(lines.setVertices(vertices), std::invalid_argument);
            EXPECT_EQ(lines.vertexCount(), 0u);
        }
    }
}

TEST_F(LineRendererTest, GpuDataContainsExplicitFloatConversion)
{
    auto program = boundProgram();
    LineRenderer lines;
    const std::array<Vector3, 2> vertices{Vector3{1.1, 2.2, 3.3}, Vector3{-4.4, 5.5, -6.6}};
    lines.setVertices(vertices);
    lines.draw();
    const GLuint buffer = attributeBuffer();
    ASSERT_NE(buffer, 0u);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    GLint size{}, usage{};
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_USAGE, &usage);
    EXPECT_EQ(size, 6 * sizeof(GLfloat));
    EXPECT_EQ(usage, GL_DYNAMIC_DRAW);
    std::array<GLfloat, 6> data{};
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data.data());
    const std::array<double, 6> expected{1.1, 2.2, 3.3, -4.4, 5.5, -6.6};
    for (std::size_t i = 0; i < data.size(); ++i)
        EXPECT_FLOAT_EQ(data[i], static_cast<float>(expected[i]));
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(LineRendererTest, AttributeZeroIsPackedFloatVec3WithoutIndexBuffer)
{
    auto program = boundProgram();
    LineRenderer lines;
    lines.setVertices(segment);
    lines.draw();
    GLint enabled{}, size{}, type{}, stride{}, normalized{}, elementBuffer{};
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_SIZE, &size);
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_TYPE, &type);
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &normalized);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementBuffer);
    EXPECT_EQ(enabled, GL_TRUE);
    EXPECT_EQ(size, 3);
    EXPECT_EQ(type, GL_FLOAT);
    EXPECT_EQ(stride, 3 * sizeof(GLfloat));
    EXPECT_EQ(normalized, GL_FALSE);
    EXPECT_EQ(elementBuffer, 0);
}

TEST_F(LineRendererTest, ConstructionAndUploadPreserveExternalBindings)
{
    auto program = boundProgram();
    LineRenderer external;
    external.setVertices(segment);
    external.draw();
    const auto vao = currentVao();
    const auto buffer = attributeBuffer();
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    {
        LineRenderer lines;
        EXPECT_EQ(currentVao(), vao);
        lines.setVertices(segment);
        EXPECT_EQ(currentVao(), vao);
        GLint boundBuffer{};
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &boundBuffer);
        EXPECT_EQ(static_cast<GLuint>(boundBuffer), buffer);
    }
    EXPECT_EQ(currentVao(), vao);
}

TEST_F(LineRendererTest, DrawLeavesShaderAndRenderPassStateUnchanged)
{
    auto program = boundProgram();
    GLint shader{};
    glGetIntegerv(GL_CURRENT_PROGRAM, &shader);
    const auto depth = glIsEnabled(GL_DEPTH_TEST);
    const auto blend = glIsEnabled(GL_BLEND);
    GLfloat width{};
    glGetFloatv(GL_LINE_WIDTH, &width);
    LineRenderer lines;
    lines.setVertices(segment);
    lines.draw();
    GLint afterShader{};
    GLfloat afterWidth{};
    glGetIntegerv(GL_CURRENT_PROGRAM, &afterShader);
    glGetFloatv(GL_LINE_WIDTH, &afterWidth);
    EXPECT_EQ(shader, afterShader);
    EXPECT_EQ(depth, glIsEnabled(GL_DEPTH_TEST));
    EXPECT_EQ(blend, glIsEnabled(GL_BLEND));
    EXPECT_FLOAT_EQ(width, afterWidth);
}

TEST_F(LineRendererTest, MoveConstructionTransfersGpuOwnership)
{
    auto program = boundProgram();
    LineRenderer source;
    source.setVertices(segment);
    source.draw();
    const auto vao = currentVao();
    const auto buffer = attributeBuffer();
    LineRenderer destination{std::move(source)};
    EXPECT_EQ(source.vertexCount(), 0u);
    EXPECT_NO_THROW(source.draw());
    EXPECT_THROW(source.setVertices(segment), std::logic_error);
    EXPECT_EQ(destination.vertexCount(), 2u);
    destination.draw();
    EXPECT_EQ(currentVao(), vao);
    EXPECT_EQ(attributeBuffer(), buffer);
}

TEST_F(LineRendererTest, MoveAssignmentDeletesPreviousResources)
{
    auto program = boundProgram();
    LineRenderer source;
    source.setVertices(segment);
    source.draw();
    const auto sourceVao = currentVao();
    LineRenderer destination;
    destination.setVertices(segment);
    destination.draw();
    const auto oldVao = currentVao();
    const auto oldBuffer = attributeBuffer();
    destination = std::move(source);
    EXPECT_EQ(glIsVertexArray(oldVao), GL_FALSE);
    EXPECT_EQ(glIsBuffer(oldBuffer), GL_FALSE);
    EXPECT_EQ(source.vertexCount(), 0u);
    destination.draw();
    EXPECT_EQ(currentVao(), sourceVao);
    EXPECT_EQ(destination.vertexCount(), 2u);
}

TEST_F(LineRendererTest, DestructionDeletesVaoAndVbo)
{
    auto program = boundProgram();
    GLuint vao{}, buffer{};
    {
        LineRenderer lines;
        lines.setVertices(segment);
        lines.draw();
        vao = currentVao();
        buffer = attributeBuffer();
    }
    EXPECT_EQ(glIsVertexArray(vao), GL_FALSE);
    EXPECT_EQ(glIsBuffer(buffer), GL_FALSE);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

}
