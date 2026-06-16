#pragma once

#pragma once

#include <windows.h>

#ifdef WPFBRIDGE_EXPORTS
#define WPFBRIDGE_API __declspec(dllexport)
#else
#define WPFBRIDGE_API __declspec(dllimport)
#endif

extern "C" WPFBRIDGE_API void ShowWpfAboutDialog(HWND parentHwnd);

using namespace System;

namespace WpfBridge {

	public ref class Bridge
	{
	public:
		static String^ GetMessage();
	};
}
