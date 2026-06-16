#include <windows.h>

typedef void (__cdecl *ShowDialogFn)(const wchar_t*, HWND);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
		// Defer via PostMessage so the menu fully closes and repaints before
		// WpfBridge initialises.  Showing a WPF dialog while the menu system is
		// still in its "closing" state causes visual corruption of the menu bar.
		if (LOWORD(wParam) == 1) // About menu id
			::PostMessageW(hWnd, WM_APP, 0, 0);
		break;
	case WM_APP:
		{
			HMODULE h = LoadLibraryW(L"WpfBridge.dll");
			if (h)
			{
				auto fn = (ShowDialogFn)GetProcAddress(h, "ShowDialog");
				if (fn) fn(L"About", hWnd);
				FreeLibrary(h);
			}
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	const wchar_t CLASS_NAME[] = L"StudioWindowClass";

	WNDCLASS wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"Studio", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, nullptr, hInstance, nullptr);

	// Create a simple menu with Help->About
	HMENU menu = CreateMenu();
	HMENU help = CreatePopupMenu();
	AppendMenuW(help, MF_STRING, 1, L"About");
	AppendMenuW(menu, MF_POPUP, (UINT_PTR)help, L"Help");
	SetMenu(hwnd, menu);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}
