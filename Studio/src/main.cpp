#include <windows.h>
#include <stdio.h>

typedef void (__cdecl *ShowDialogFn)(const wchar_t*, HWND);

static constexpr UINT IDM_HELP  = 2;
static constexpr UINT IDM_ABOUT = 1;

static void LaunchDialog(HWND hWnd, const wchar_t* dialogId)
{
	HMODULE h = LoadLibraryW(L"WpfBridge.dll");
	if (h)
	{
		auto fn = (ShowDialogFn)GetProcAddress(h, "ShowDialog");
		if (fn) fn(dialogId, hWnd);
		FreeLibrary(h);
	}
}

// Launches HelpViewer.exe from the HelpViewer/ subdirectory next to this exe.
// Help is non-modal and runs in its own process, so it never touches this
// process's GDI/DWM state.
static void LaunchHelp(HWND owner)
{
	wchar_t exeDir[MAX_PATH]{};
	GetModuleFileNameW(nullptr, exeDir, MAX_PATH);
	if (auto* last = wcsrchr(exeDir, L'\\')) *last = L'\0';

	wchar_t cmd[MAX_PATH + 64];
	swprintf_s(cmd, L"\"%s\\HelpViewer\\HelpViewer.exe\" --parent-hwnd 0x%llX",
	           exeDir, (unsigned long long)(uintptr_t)owner);

	STARTUPINFOW si{ .cb = sizeof(si) };
	PROCESS_INFORMATION pi{};
	CreateProcessW(nullptr, cmd, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
	if (pi.hProcess) CloseHandle(pi.hProcess);
	if (pi.hThread)  CloseHandle(pi.hThread);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
		// Defer via PostMessage so the menu fully closes and repaints before
		// WpfBridge initialises.  Showing a WPF dialog while the menu system is
		// still in its "closing" state causes visual corruption of the menu bar.
		if (LOWORD(wParam) == IDM_ABOUT)
			::PostMessageW(hWnd, WM_APP, 0, 0);
		else if (LOWORD(wParam) == IDM_HELP)
			::PostMessageW(hWnd, WM_APP + 1, 0, 0);
		break;
	case WM_APP:
		LaunchDialog(hWnd, L"About");
		break;
	case WM_APP + 1:
		LaunchHelp(hWnd);
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

	// Create menu: Help > Help Topics / --- / About
	HMENU menu = CreateMenu();
	HMENU help = CreatePopupMenu();
	AppendMenuW(help, MF_STRING,    IDM_HELP,  L"Help Topics\tF1");
	AppendMenuW(help, MF_SEPARATOR, 0,          nullptr);
	AppendMenuW(help, MF_STRING,    IDM_ABOUT, L"About");
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
