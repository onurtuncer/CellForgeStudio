#include "pch.h"
#include "Bridge.h"

using namespace System;
using namespace WpfBridge;

String^ Bridge::GetMessage()
{
	return "Hello from WpfBridge";
}

// Implementation of exported function that loads WpfControls and shows the About window
extern "C" WPFBRIDGE_API void ShowWpfAboutDialog(HWND parentHwnd)
{
	try
	{
		// Determine path to WpfControls.dll (next to this DLL or next to executable)
		String^ asmPath = System::IO::Path::Combine(System::IO::Path::GetDirectoryName(System::Reflection::Assembly::GetExecutingAssembly()->Location), "WpfControls.dll");
		if (!System::IO::File::Exists(asmPath))
		{
			// Try executable folder
			String^ exePath = System::Reflection::Assembly::GetEntryAssembly() ? System::IO::Path::GetDirectoryName(System::Reflection::Assembly::GetEntryAssembly()->Location) : System::Environment::CurrentDirectory;
			asmPath = System::IO::Path::Combine(exePath, "WpfControls.dll");
		}

		if (!System::IO::File::Exists(asmPath))
		{
			// Fail gracefully
			return;
		}

		auto asm = System::Reflection::Assembly::LoadFrom(asmPath);
		auto aboutType = asm->GetType("WpfControls.AboutWindow");
		if (aboutType == nullptr)
			return;

		System::Windows::Window^ wnd = safe_cast<System::Windows::Window^>(System::Activator::CreateInstance(aboutType));

		// Set owner via WindowInteropHelper
		System::Windows::Interop::WindowInteropHelper^ helper = gcnew System::Windows::Interop::WindowInteropHelper(wnd);
		helper->Owner = System::IntPtr(parentHwnd);

		wnd->ShowDialog();
	}
	catch (System::Exception^)
	{
		// swallow for safety; real code should log
	}
}
