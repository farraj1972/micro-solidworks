#include "ui/ImGuiLayer.h"

#include "app/logging/Logger.h"
#include "app/window/ApplicationWindow.h"
#include "rendering/OpenGLContext.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cstdlib>
#include <filesystem>
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
        // The built-in pixel font is too small for a desktop CAD inspector.
        // Load the installed Windows UI font at runtime; do not redistribute it.
        ImFont* uiFont = nullptr;
        if (const char* windowsDirectory = std::getenv("WINDIR"))
        {
            const auto fontPath = std::filesystem::path{windowsDirectory} / "Fonts" / "segoeui.ttf";
            if (std::filesystem::exists(fontPath))
                uiFont = ImGui::GetIO().Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 17.0F);
        }
        if (uiFont == nullptr)
        {
            ImFontConfig fallback;
            fallback.SizePixels = 17.0F;
            uiFont = ImGui::GetIO().Fonts->AddFontDefault(&fallback);
        }
        if (uiFont == nullptr)
            throw std::runtime_error{"Failed to load the UI font"};
        ImGui::StyleColorsDark();
        auto& style = ImGui::GetStyle();
        style.WindowPadding = {10.0F, 8.0F};
        style.FramePadding = {8.0F, 5.0F};
        style.ItemSpacing = {7.0F, 6.0F};
        style.WindowRounding = 5.0F;
        style.FrameRounding = 4.0F;
        style.PopupRounding = 5.0F;
        style.GrabRounding = 4.0F;
        style.Colors[ImGuiCol_WindowBg] = {0.075F, 0.090F, 0.112F, 1.0F};
        style.Colors[ImGuiCol_ChildBg] = {0.085F, 0.104F, 0.132F, 1.0F};
        style.Colors[ImGuiCol_MenuBarBg] = {0.082F, 0.103F, 0.133F, 1.0F};
        style.Colors[ImGuiCol_FrameBg] = {0.13F, 0.16F, 0.20F, 1.0F};
        style.Colors[ImGuiCol_FrameBgHovered] = {0.18F, 0.23F, 0.30F, 1.0F};
        style.Colors[ImGuiCol_Button] = {0.15F, 0.20F, 0.27F, 1.0F};
        style.Colors[ImGuiCol_ButtonHovered] = {0.20F, 0.37F, 0.56F, 1.0F};
        style.Colors[ImGuiCol_ButtonActive] = {0.18F, 0.45F, 0.73F, 1.0F};
        style.Colors[ImGuiCol_Header] = {0.16F, 0.32F, 0.51F, 1.0F};
        style.Colors[ImGuiCol_HeaderHovered] = {0.21F, 0.43F, 0.68F, 1.0F};
        style.Colors[ImGuiCol_HeaderActive] = {0.20F, 0.48F, 0.78F, 1.0F};
        style.Colors[ImGuiCol_CheckMark] = {0.35F, 0.67F, 1.0F, 1.0F};
        style.Colors[ImGuiCol_SliderGrab] = {0.32F, 0.63F, 0.95F, 1.0F};
        style.Colors[ImGuiCol_Separator] = {0.22F, 0.28F, 0.36F, 1.0F};

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
