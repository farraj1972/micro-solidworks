#include "ui/ImGuiLayer.h"

#include "app/logging/Logger.h"
#include "app/window/ApplicationWindow.h"
#include "rendering/OpenGLContext.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <stdexcept>

namespace microsw
{
ImGuiLayer::ImGuiLayer(ApplicationWindow& window, OpenGLContext& openGLContext)
{
    static_cast<void>(openGLContext);
    Logger::info("Initializing Dear ImGui");

    try
    {
        IMGUI_CHECKVERSION();
        if (ImGui::CreateContext() == nullptr)
        {
            Logger::error("Failed to create Dear ImGui context");
            throw std::runtime_error{"Dear ImGui context creation failed"};
        }
        contextCreated_ = true;
        Logger::info("Dear ImGui context created");

        ImGui::GetIO().IniFilename = nullptr;
        ImGui::StyleColorsDark();

        Logger::info("Initializing ImGui GLFW backend");
        auto* nativeWindow = static_cast<GLFWwindow*>(window.nativeHandle());
        if (!ImGui_ImplGlfw_InitForOpenGL(nativeWindow, true))
        {
            Logger::error("Failed to initialize ImGui GLFW backend");
            throw std::runtime_error{"ImGui GLFW backend initialization failed"};
        }
        glfwBackendInitialized_ = true;

        Logger::info("Initializing ImGui OpenGL backend");
        if (!ImGui_ImplOpenGL3_Init("#version 330"))
        {
            Logger::error("Failed to initialize ImGui OpenGL backend");
            throw std::runtime_error{"ImGui OpenGL backend initialization failed"};
        }
        openGLBackendInitialized_ = true;

        Logger::info("Dear ImGui initialized");
    }
    catch (...)
    {
        shutdown();
        throw;
    }
}

ImGuiLayer::~ImGuiLayer()
{
    try
    {
        Logger::info("Shutting down Dear ImGui");
    }
    catch (...)
    {
    }

    shutdown();
}

void ImGuiLayer::beginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::drawDiagnosticPanel()
{
    ImGui::SetNextWindowSize(ImVec2{390.0F, 190.0F}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Diagnostics", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::TextUnformatted("Micro SolidWorks");
    ImGui::TextUnformatted("Dear ImGui integration operational");
    ImGui::TextUnformatted("OpenGL 3.3 Core");
    ImGui::Separator();
    ImGui::InputText("Keyboard input", diagnosticInput_.data(), diagnosticInput_.size());

    if (ImGui::Button("Test mouse input"))
    {
        diagnosticButtonClicked_ = true;
        Logger::info("Dear ImGui diagnostic input received");
    }

    if (diagnosticButtonClicked_)
    {
        ImGui::SameLine();
        ImGui::TextUnformatted("Input received");
    }

    ImGui::End();
}

void ImGuiLayer::endFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiLayer::shutdown() noexcept
{
    if (openGLBackendInitialized_)
    {
        ImGui_ImplOpenGL3_Shutdown();
        openGLBackendInitialized_ = false;
    }

    if (glfwBackendInitialized_)
    {
        ImGui_ImplGlfw_Shutdown();
        glfwBackendInitialized_ = false;
    }

    if (contextCreated_)
    {
        ImGui::DestroyContext();
        contextCreated_ = false;
    }
}
}
