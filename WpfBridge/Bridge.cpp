#include "pch.h"
#include "Bridge.h"

using namespace System;
using namespace WpfBridge;

String^ Bridge::GetMessage()
{
	return "Hello from WpfBridge";
}

// Implementation of exported function that loads WpfControls and shows the About control
// hosted inside a temporary dialog using HwndSource reparenting.
extern "C" WPFBRIDGE_API void ShowWpfAboutDialog(HWND parentHwnd)
{
	try
	{
		// Determine path to WpfControls.dll (next to this DLL or next to executable)
		String^ asmPath = System::IO::Path::Combine(System::IO::Path::GetDirectoryName(System::Reflection::Assembly::GetExecutingAssembly()->Location), "WpfControls.dll");
		if (!System::IO::File::Exists(asmPath))
		{
			String^ exePath = System::Reflection::Assembly::GetEntryAssembly() ? System::IO::Path::GetDirectoryName(System::Reflection::Assembly::GetEntryAssembly()->Location) : System::Environment::CurrentDirectory;
			asmPath = System::IO::Path::Combine(exePath, "WpfControls.dll");
		}

		if (!System::IO::File::Exists(asmPath))
			return;

		auto asm = System::Reflection::Assembly::LoadFrom(asmPath);
		auto aboutType = asm->GetType("WpfControls.AboutControl");
		if (aboutType == nullptr)
			return;

		// Create the WPF UserControl instance
		System::Windows::Controls::UserControl^ control = safe_cast<System::Windows::Controls::UserControl^>(System::Activator::CreateInstance(aboutType));

		// Measure/arrange to determine size
		System::Windows::Size measureSize(400.0, 300.0);
		control->Measure(measureSize);
		control->Arrange(System::Windows::Rect(0, 0, control->DesiredSize.Width > 0 ? control->DesiredSize.Width : 400.0,
											   control->DesiredSize.Height > 0 ? control->DesiredSize.Height : 240.0));
		control->UpdateLayout();

		int w = (int)(System::Double::IsNaN(control->Width) ? control->DesiredSize.Width : control->Width);
		int h = (int)(System::Double::IsNaN(control->Height) ? control->DesiredSize.Height : control->Height);
		if (w <= 0) w = 400;
		if (h <= 0) h = 240;

		// Create HwndSource as a child (initial parent is the provided parentHwnd)
		System::Windows::Interop::HwndSourceParameters^ params = gcnew System::Windows::Interop::HwndSourceParameters("WpfAboutHost");
		params->WindowStyle = WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN;
		params->ParentWindow = System::IntPtr(parentHwnd);
		params->PositionX = 0;
		params->PositionY = 0;
		params->Height = h;
		params->Width = w;
		params->RootVisual = control;

		System::Windows::Interop::HwndSource^ source = gcnew System::Windows::Interop::HwndSource(*params);

		// Wire close event
		bool closed = false;
		auto aboutControlType = aboutType; // capture
		// Attempt to hook CloseRequested event if present
		try
		{
			auto ev = aboutType->GetEvent("CloseRequested");
			if (ev != nullptr)
			{
				System::Action^ action = gcnew System::Action(gcnew System::Action([&closed, parentHwnd]() {
					closed = true;
					::EnableWindow(parentHwnd, TRUE);
					::SetFocus(parentHwnd);
					::PostMessage(parentHwnd, WM_USER + 1, 0, 0);
				}));
				ev->AddEventHandler(control, action);
			}
		}
		catch (...) { }

		// Create a simple dialog window (#32770) as the modal host
		HWND dlgHwnd = ::CreateWindowExW(
			WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
			L"#32770",
			L"About",
			WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
			0, 0, w + 16, h + 38,
			parentHwnd,
			nullptr,
			::GetModuleHandle(nullptr),
			nullptr);

		// Re-parent the HwndSource child into the dialog
		::SetParent((HWND)source->Handle.ToPointer(), dlgHwnd);
		::SetWindowPos((HWND)source->Handle.ToPointer(), nullptr, 0, 0, w, h, SWP_NOZORDER | SWP_SHOWWINDOW);

		// Center dialog over parent
		RECT pr{};
		::GetWindowRect(parentHwnd, &pr);
		int cx = pr.left + (pr.right - pr.left) / 2;
		int cy = pr.top + (pr.bottom - pr.top) / 2;
		RECT dr{};
		::GetWindowRect(dlgHwnd, &dr);
		int dw = dr.right - dr.left;
		int dh = dr.bottom - dr.top;
		::SetWindowPos(dlgHwnd, nullptr, cx - dw/2, cy - dh/2, dw, dh, SWP_NOZORDER);

		::ShowWindow(dlgHwnd, SW_SHOW);
		::EnableWindow(parentHwnd, FALSE); // modal

		// Message pump until closed flag set or window destroyed
		MSG msg{};
		while (!closed && ::GetMessage(&msg, nullptr, 0, 0))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}

		::DestroyWindow(dlgHwnd);
		source->Dispose();
	}
	catch (System::Exception^)
	{
		// intentionally empty per request
	}
}
