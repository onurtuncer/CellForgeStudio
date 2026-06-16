#include <windows.h>
#include <stdio.h>

static constexpr UINT IDM_HELP  = 2;
static constexpr UINT IDM_ABOUT = 1;

#define STUDIO_VERSION   L"0.1.0"
#define STUDIO_COPYRIGHT L"Copyright © 2026 CellForge Studio"

// Launches AboutViewer.exe as a modal child process.
// MsgWaitForMultipleObjects keeps the message pump alive so the parent
// repaints, handles WM_PAINT etc., while blocking new About invocations.
static bool s_aboutOpen = false;

static void LaunchAbout(HWND owner)
{
	if (s_aboutOpen) return;
	s_aboutOpen = true;

	wchar_t exeDir[MAX_PATH]{};
	GetModuleFileNameW(nullptr, exeDir, MAX_PATH);
	if (auto* last = wcsrchr(exeDir, L'\\')) *last = L'\0';

	wchar_t cmd[MAX_PATH + 256];
	swprintf_s(cmd,
	    L"\"%s\\AboutViewer\\AboutViewer.exe\""
	    L" --parent-hwnd 0x%llX"
	    L" --version \"" STUDIO_VERSION L"\""
	    L" --copyright \"" STUDIO_COPYRIGHT L"\"",
	    exeDir, (unsigned long long)(uintptr_t)owner);

	STARTUPINFOW si{ .cb = sizeof(si) };
	PROCESS_INFORMATION pi{};
	if (CreateProcessW(nullptr, cmd, nullptr, nullptr, FALSE, 0,
	                   nullptr, nullptr, &si, &pi))
	{
		DWORD res;
		do {
			res = MsgWaitForMultipleObjects(1, &pi.hProcess, FALSE, INFINITE, QS_ALLINPUT);
			if (res == WAIT_OBJECT_0 + 1)
			{
				MSG msg;
				while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
				{
					if (msg.message == WM_QUIT)
					{
						PostQuitMessage((int)msg.wParam);
						goto done;
					}
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		} while (res != WAIT_OBJECT_0);
done:
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	s_aboutOpen = false;
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
		LaunchAbout(hWnd);
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
