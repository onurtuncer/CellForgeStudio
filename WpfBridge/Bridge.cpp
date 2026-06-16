#include "pch.h"
#include "Bridge.h"
#include <vcclr.h>

using namespace System;
using namespace WpfBridge;

// ── helpers ───────────────────────────────────────────────────────────────────

static void LogDebug(String^ msg)
{
    String^ line = msg + "\n";
    pin_ptr<const wchar_t> p = PtrToStringChars(line);
    ::OutputDebugStringW(p);
}

static String^ ResolveAssemblyPath(String^ dllName)
{
    String^ next = IO::Path::Combine(
        IO::Path::GetDirectoryName(Reflection::Assembly::GetExecutingAssembly()->Location),
        dllName);
    if (IO::File::Exists(next)) return next;

    String^ base = Reflection::Assembly::GetEntryAssembly()
        ? IO::Path::GetDirectoryName(Reflection::Assembly::GetEntryAssembly()->Location)
        : Environment::CurrentDirectory;
    String^ alt = IO::Path::Combine(base, dllName);
    if (IO::File::Exists(alt)) return alt;

    return nullptr;
}

// ── dialog host ───────────────────────────────────────────────────────────────

// Shared state for WM_CLOSE (title-bar X) and the WPF control's CloseRequested event.
// Using a ref class avoids native-variable capture issues in managed delegates.
ref class DialogHost sealed
{
    IntPtr _parent;
    IntPtr _dlg;

public:
    bool Closed;

    DialogHost(HWND parent, HWND dlg)
        : _parent(IntPtr(parent)), _dlg(IntPtr(dlg)), Closed(false) {}

    void OnCloseRequested()
    {
        Closed = true;
        ::PostMessage((HWND)_dlg.ToPointer(), WM_NULL, 0, 0);  // wake GetMessage
    }

    // Intercepts WM_CLOSE so we control window lifetime (not DefWindowProc)
    IntPtr HwndHook(IntPtr /*hwnd*/, int msg,
                    IntPtr /*wParam*/, IntPtr /*lParam*/, bool% handled)
    {
        if (msg == WM_CLOSE)
        {
            OnCloseRequested();
            handled = true;
        }
        return IntPtr::Zero;
    }
};

// ── core dialog host function ─────────────────────────────────────────────────

// Shows a WPF UserControl in a modal window where HwndSource IS the root HWND.
// No SetParent re-parenting: WPF owns the full HWND, so Tab / Escape /
// accelerators route correctly through WPF's IKeyboardInputSink without
// any IsDialogMessage calls in the pump.
//
// Fully-qualified System::Windows:: names are required because .NET 10's WPF
// assemblies also project a top-level 'Windows' namespace (WinRT interop), which
// makes the unqualified 'Windows::' ambiguous for the C++/CLI compiler.
static void ShowControlDialog(Type^ controlType, HWND parentHwnd)
{
    auto control = safe_cast<System::Windows::Controls::UserControl^>(
        Activator::CreateInstance(controlType));

    // Prefer the control's explicit Width/Height; fall back to measured DesiredSize
    control->Measure(System::Windows::Size(2048.0, 2048.0));
    double cw = (!Double::IsNaN(control->Width)  && control->Width  > 0) ? control->Width
              : (control->DesiredSize.Width  > 0  ? control->DesiredSize.Width  : 400.0);
    double ch = (!Double::IsNaN(control->Height) && control->Height > 0) ? control->Height
              : (control->DesiredSize.Height > 0  ? control->DesiredSize.Height : 240.0);

    // WS_POPUP (not WS_CHILD) makes HwndSource the root window.
    // WPF then owns all keyboard routing; no re-parenting needed.
    System::Windows::Interop::HwndSourceParameters sourceParams("WpfDialogHost");
    sourceParams.WindowStyle         = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN;
    sourceParams.ExtendedWindowStyle = WS_EX_DLGMODALFRAME;
    sourceParams.ParentWindow        = IntPtr(parentHwnd);
    sourceParams.Width               = (int)cw;
    sourceParams.Height              = (int)ch;

    auto source  = gcnew System::Windows::Interop::HwndSource(sourceParams);

    // Force software rendering BEFORE setting RootVisual so WPF never creates a
    // DirectX swap chain.  A DX swap chain causes DWM to re-composite all windows,
    // which corrupts the GDI-rendered menu bar of the native parent window.
    // Software rendering is imperceptible for a simple dialog.
    safe_cast<System::Windows::Interop::HwndTarget^>(source->CompositionTarget)
        ->RenderMode = System::Windows::Interop::RenderMode::SoftwareOnly;

    // RootVisual removed from HwndSourceParameters in .NET 5+ — set after construction
    source->RootVisual = control;
    HWND dlgHwnd = (HWND)source->Handle.ToPointer();

    // Measure actual chrome so client area == content size (varies by DPI / theme)
    RECT wr{}, cr{};
    ::GetWindowRect(dlgHwnd, &wr);
    ::GetClientRect(dlgHwnd, &cr);
    int totalW = (int)cw + (wr.right - wr.left) - cr.right;
    int totalH = (int)ch + (wr.bottom - wr.top) - cr.bottom;

    // Center over parent
    int posX = 0, posY = 0;
    RECT pr{};
    if (parentHwnd && ::GetWindowRect(parentHwnd, &pr))
    {
        posX = pr.left + (pr.right  - pr.left - totalW) / 2;
        posY = pr.top  + (pr.bottom - pr.top  - totalH) / 2;
    }

    auto host = gcnew DialogHost(parentHwnd, dlgHwnd);

    // Handle title-bar X click
    source->AddHook(gcnew System::Windows::Interop::HwndSourceHook(host, &DialogHost::HwndHook));

    // Handle WPF control's CloseRequested event if it exposes one
    auto ev = controlType->GetEvent("CloseRequested");
    if (ev != nullptr)
        ev->AddEventHandler(control, gcnew Action(host, &DialogHost::OnCloseRequested));

    // Show the dialog.  We deliberately do NOT call EnableWindow(FALSE) on the
    // parent: that call grays out the menu bar, which is exactly the visual
    // corruption the user sees.  The blocking message pump below already prevents
    // re-entrancy on this thread (any WM_COMMAND dispatched while we're in here
    // would hit the s_active guard at the call site).
    ::SetWindowPos(dlgHwnd, HWND_TOP, posX, posY, totalW, totalH, SWP_SHOWWINDOW);
    ::SetForegroundWindow(dlgHwnd);

    // Standard pump — WPF routes WM_KEYDOWN/UP internally via DispatchMessage.
    // Re-post WM_QUIT so the outer loop also exits if the app is shutting down.
    MSG msg{};
    while (!host->Closed)
    {
        BOOL r = ::GetMessage(&msg, nullptr, 0, 0);
        if (r == 0) { ::PostQuitMessage((int)msg.wParam); break; }
        if (r < 0)  break;
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    if (parentHwnd)
        ::SetFocus(parentHwnd);
    delete source;  // C++/CLI destructor syntax for IDisposable::Dispose()
}

// ── dialog registry ───────────────────────────────────────────────────────────

// Returns a fresh registry each call.  ShowDialog is UI-triggered so this is
// not a hot path; the allocation is negligible vs. the dialog itself.
// A managed static global would require gcroot<> and complicates lifetime.
static Collections::Generic::Dictionary<String^, String^>^ GetDialogRegistry()
{
    auto reg = gcnew Collections::Generic::Dictionary<String^, String^>(
        StringComparer::OrdinalIgnoreCase);
    // registry is intentionally empty — all dialogs are now out-of-process exes
    return reg;
}

// ── public exports ────────────────────────────────────────────────────────────

String^ Bridge::GetMessage() { return "Hello from WpfBridge"; }

// Prevents nested ShowDialog calls dispatched through the dialog's own pump.
static bool s_dialogActive = false;

extern "C" WPFBRIDGE_API void ShowDialog(const wchar_t* dialogId, HWND parentHwnd)
{
    if (s_dialogActive)
    {
        LogDebug("[WpfBridge] ShowDialog ignored: a dialog is already open");
        return;
    }
    s_dialogActive = true;
    try
    {
        String^ id = gcnew String(dialogId ? dialogId : L"");
        String^ typeName;
        if (!GetDialogRegistry()->TryGetValue(id, typeName))
        {
            LogDebug("[WpfBridge] Unknown dialogId: \"" + id + "\"");
            return;
        }

        String^ asmPath = ResolveAssemblyPath("WpfControls.dll");
        if (asmPath == nullptr)
        {
            LogDebug("[WpfBridge] WpfControls.dll not found next to WpfBridge.dll or entry assembly");
            return;
        }

        auto wpfAsm = Reflection::Assembly::LoadFrom(asmPath);
        auto type   = wpfAsm->GetType(typeName);
        if (type == nullptr)
        {
            LogDebug("[WpfBridge] Type not found in assembly: " + typeName);
            return;
        }

        ShowControlDialog(type, parentHwnd);
    }
    catch (Exception^ ex)
    {
        LogDebug("[WpfBridge] ShowDialog(\"" + gcnew String(dialogId ? dialogId : L"") + "\") failed:\n" + ex->ToString());
    }
    s_dialogActive = false;
}

// Backwards-compatible alias kept so existing callers don't need updating.
extern "C" WPFBRIDGE_API void ShowWpfAboutDialog(HWND parentHwnd)
{
    ShowDialog(L"About", parentHwnd);
}
