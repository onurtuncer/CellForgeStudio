#include <windows.h>

typedef void (__cdecl *ShowDialogFn)(const wchar_t*, HWND);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	const wchar_t CLASS_NAME[] = L"AboutTestWindowClass";

	WNDCLASS wc = {};
	wc.lpfnWndProc = DefWindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"About Test Host", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, nullptr, nullptr, hInstance, nullptr);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	HMODULE h = LoadLibraryW(L"WpfBridge.dll");
	if (h)
	{
		auto fn = (ShowDialogFn)GetProcAddress(h, "ShowDialog");
		if (fn) fn(L"About", hwnd);
		FreeLibrary(h);
	}

	// Exit immediately after dialog closes
	return 0;
}
