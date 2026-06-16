#pragma once

#include <windows.h>

#ifdef WPFBRIDGE_EXPORTS
#define WPFBRIDGE_API __declspec(dllexport)
#else
#define WPFBRIDGE_API __declspec(dllimport)
#endif

// Generic dialog dispatch. dialogId is matched case-insensitively against the registry.
// Currently registered IDs: "About"
// Add entries to GetDialogRegistry() in Bridge.cpp to register additional dialogs.
extern "C" WPFBRIDGE_API void ShowDialog(const wchar_t* dialogId, HWND parentHwnd);

// Backwards-compatible alias; equivalent to ShowDialog(L"About", parentHwnd).
extern "C" WPFBRIDGE_API void ShowWpfAboutDialog(HWND parentHwnd);

namespace WpfBridge {
    public ref class Bridge
    {
    public:
        static System::String^ GetMessage();
    };
}
