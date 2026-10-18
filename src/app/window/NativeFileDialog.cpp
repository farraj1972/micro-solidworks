#include "app/window/NativeFileDialog.h"

#include "app/window/ApplicationWindow.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <commdlg.h>

#include <array>
#include <system_error>

namespace microsw
{
namespace
{
constexpr wchar_t fileFilter[] = L"Micro SolidWorks Parts (*.microsw)\0*.microsw\0All Files (*.*)\0*.*\0\0";

std::optional<std::filesystem::path> choose(const ApplicationWindow& owner,
    bool save, const std::optional<std::filesystem::path>& currentPath)
{
    std::array<wchar_t, 32768> fileName{};
    if (currentPath)
    {
        const auto value = currentPath->wstring();
        std::copy_n(value.c_str(), (std::min)(value.size(), fileName.size() - 1), fileName.data());
    }
    OPENFILENAMEW dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = static_cast<HWND>(owner.nativeHandle());
    dialog.lpstrFilter = fileFilter;
    dialog.lpstrFile = fileName.data();
    dialog.nMaxFile = static_cast<DWORD>(fileName.size());
    dialog.lpstrDefExt = L"microsw";
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
}
