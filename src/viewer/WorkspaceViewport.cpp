#include "viewer/WorkspaceViewport.h"

#include "viewer/OrbitCamera.h"
#include "viewer/ViewProjection.h"
#include "rendering/ShaderProgram.h"
#include "rendering/LineRenderer.h"

#include <glad/gl.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

namespace microsw::viewer
{

math::Scalar ViewportRect::aspectRatio() const noexcept
{
    return width > 0 && height > 0 ? static_cast<math::Scalar>(width) / height : 0.0;
}

ViewportRect framebufferRect(const WorkspaceLayout& layout, int framebufferWidth, int framebufferHeight)
{
    for (double value : {layout.x, layout.y, layout.width, layout.height,
                         layout.displayWidth, layout.displayHeight})
    {
        if (!std::isfinite(value))
            return {};
    }
    if (framebufferWidth <= 0 || framebufferHeight <= 0
        || layout.width <= 0 || layout.height <= 0
        || layout.displayWidth <= 0 || layout.displayHeight <= 0)
        return {};

    // Clip in logical units before scaling, avoiding integer overflow and UI bleed.
    const double left = std::clamp(layout.x, 0.0, layout.displayWidth);
    const double top = std::clamp(layout.y, 0.0, layout.displayHeight);
    const double right = std::clamp(layout.x + layout.width, 0.0, layout.displayWidth);
    const double bottom = std::clamp(layout.y + layout.height, 0.0, layout.displayHeight);
    const int x0 = static_cast<int>(std::ceil((left / layout.displayWidth) * framebufferWidth));
    const int y0 = static_cast<int>(std::ceil((top / layout.displayHeight) * framebufferHeight));
    const int x1 = static_cast<int>(std::floor((right / layout.displayWidth) * framebufferWidth));
    const int y1 = static_cast<int>(std::floor((bottom / layout.displayHeight) * framebufferHeight));
    if (x1 <= x0 || y1 <= y0)
        return {};
    return {x0, framebufferHeight - y1, x1 - x0, y1 - y0};
}

namespace
{

constexpr const char* vertexSource = R"(#version 330 core
layout(location = 0) in vec3 aPosition;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
void main()
{
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
}
)";
constexpr const char* fragmentSource = R"(#version 330 core
out vec4 FragColor;
void main() { FragColor = vec4(1.0); }
)";

// Local pass guard, including exception paths; not a general OpenGL state stack.
struct PassState
{
    GLint viewport[4]{}, scissor[4]{}, depthFunc{}, program{}, vao{};
    GLboolean scissorEnabled{}, depthEnabled{}, blendEnabled{}, depthMask{}, colorMask[4]{};
    GLfloat clearColor[4]{};
    GLdouble clearDepth{};

    PassState()
    {
        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetIntegerv(GL_SCISSOR_BOX, scissor);
        glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
        glGetIntegerv(GL_CURRENT_PROGRAM, &program);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
        scissorEnabled = glIsEnabled(GL_SCISSOR_TEST);
        depthEnabled = glIsEnabled(GL_DEPTH_TEST);
        blendEnabled = glIsEnabled(GL_BLEND);
        glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
        glGetBooleanv(GL_COLOR_WRITEMASK, colorMask);
        glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColor);
        glGetDoublev(GL_DEPTH_CLEAR_VALUE, &clearDepth);
    }

    ~PassState()
    {
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
        if (scissorEnabled) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
        if (depthEnabled) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        if (blendEnabled) glEnable(GL_BLEND); else glDisable(GL_BLEND);
        glDepthFunc(static_cast<GLenum>(depthFunc));
        glDepthMask(depthMask);
        glColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);
        glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
        glClearDepth(clearDepth);
        glUseProgram(static_cast<GLuint>(program));
        glBindVertexArray(static_cast<GLuint>(vao));
    }
};

}

class WorkspaceViewport::Impl
{
public:
    Impl() : shader{vertexSource, fragmentSource}
    {
        const std::array<math::Vector3, 2> vertices{
            math::Vector3{-1.0, 0.0, 0.0}, math::Vector3{1.0, 0.0, 0.0}};
        line.setVertices(vertices);
    }

    OrbitCamera camera;
    const math::Scalar verticalFov = std::numbers::pi_v<math::Scalar> / 3.0;
    const math::Scalar nearPlane = 0.1;
    const math::Scalar farPlane = 100.0;
    rendering::ShaderProgram shader;
    rendering::LineRenderer line;
};

WorkspaceViewport::WorkspaceViewport() : impl_{std::make_unique<Impl>()} {}
WorkspaceViewport::~WorkspaceViewport() = default;

void WorkspaceViewport::render(const WorkspaceLayout& layout, int framebufferWidth, int framebufferHeight)
{
    const auto rect = framebufferRect(layout, framebufferWidth, framebufferHeight);
    if (rect.width <= 0 || rect.height <= 0)
        return;

    const PassState saved;
    glViewport(rect.x, rect.y, rect.width, rect.height);
    glScissor(rect.x, rect.y, rect.width, rect.height);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDisable(GL_BLEND);
    glClearColor(0.12F, 0.12F, 0.14F, 1.0F);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    impl_->shader.bind();
    impl_->shader.setMatrix4("uModel", math::Matrix4::identity());
    impl_->shader.setMatrix4("uView", viewMatrix(impl_->camera));
    impl_->shader.setMatrix4("uProjection",
        perspective(impl_->verticalFov, rect.aspectRatio(), impl_->nearPlane, impl_->farPlane));
    impl_->line.draw();
}

}
