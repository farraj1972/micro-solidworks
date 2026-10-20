#include "app/window/NativeFileDialog.h"

#include "app/window/ApplicationWindow.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <commdlg.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <array>
#include <system_error>

namespace microsw
{
namespace
{
constexpr wchar_t fileFilter[] = L"Micro SolidWorks Parts (*.microsw)\0*.microsw\0All Files (*.*)\0*.*\0\0";
constexpr wchar_t stlFilter[] = L"STL Mesh (*.stl)\0*.stl\0All Files (*.*)\0*.*\0\0";

std::optional<std::filesystem::path> choose(const ApplicationWindow& owner,
    bool save, const std::optional<std::filesystem::path>& currentPath,
    const wchar_t* filter = fileFilter, const wchar_t* extension = L"microsw")
{
    std::array<wchar_t, 32768> fileName{};
    if (currentPath)
    {
        const auto value = currentPath->wstring();
        std::copy_n(value.c_str(), (std::min)(value.size(), fileName.size() - 1), fileName.data());
    }
    OPENFILENAMEW dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = glfwGetWin32Window(static_cast<GLFWwindow*>(owner.nativeHandle()));
    dialog.lpstrFilter = filter;
    dialog.lpstrFile = fileName.data();
    dialog.nMaxFile = static_cast<DWORD>(fileName.size());
    dialog.lpstrDefExt = extension;
    dialog.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_NOCHANGEDIR
        | (save ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST);
    const BOOL accepted = save ? GetSaveFileNameW(&dialog) : GetOpenFileNameW(&dialog);
    if (accepted == TRUE) return std::filesystem::path{fileName.data()};
    const auto error = CommDlgExtendedError();
    if (error != 0) throw std::system_error{static_cast<int>(error), std::system_category(),
        save ? "Save As dialog failed" : "Open dialog failed"};
    return std::nullopt;
}
}

std::optional<std::filesystem::path> chooseMicroswFileToOpen(const ApplicationWindow& owner)
{
    return choose(owner, false, std::nullopt);
}

std::optional<std::filesystem::path> chooseMicroswFileToSave(const ApplicationWindow& owner,
    const std::optional<std::filesystem::path>& currentPath)
{
    return choose(owner, true, currentPath);
}

std::optional<std::filesystem::path> chooseStlFileToExport(const ApplicationWindow& owner)
{
    return choose(owner, true, std::nullopt, stlFilter, L"stl");
}
}
