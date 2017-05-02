#include <Windows.h>
#include <Tchar.h>
#include <Shellapi.h>
#include "resource.h"

/*
	What is NCS ?
	N = Num lock
	C = Caps lock
	S = Scroll lock
*/

#define NCS_TRAY_MESSAGE (WM_APP + 1)

TCHAR MyClassName[] = _TEXT("MyWindowClass");

HMENU hMenu = NULL;

int CurrentLockState = 0;

void GetNCSState(int *state)
{
	SHORT NumToggled = 0;
	SHORT CapToggled = 0;
	SHORT ScrToggled = 0;
	*state = 0;
	if ((NumToggled = GetKeyState(VK_NUMLOCK) & 1) |
		(CapToggled = GetKeyState(VK_CAPITAL) & 1) |
		(ScrToggled = GetKeyState(VK_SCROLL) & 1))
	{
		*state = ((NumToggled << 2)|(CapToggled << 1)|ScrToggled) & 7;
	}
	return;
}

void NCSTrayIcon(int message,HWND hWnd,int LockState)
{
	NOTIFYICONDATA ico = {0};
	ico.cbSize = sizeof(ico);
	ico.hWnd = hWnd;
	ico.uID = 1;
	ico.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	ico.uCallbackMessage = NCS_TRAY_MESSAGE;
	ico.hIcon = LoadIcon(GetModuleHandle(NULL),2 + LockState);
	wsprintf(
	ico.szTip,
	_TEXT("Num lock is %s\nCaps lock is %s\nScroll lock is %s"),
	(LockState & 4) ? _TEXT("On") : _TEXT("Off"),
	(LockState & 2) ? _TEXT("On") : _TEXT("Off"),
	(LockState & 1) ? _TEXT("On") : _TEXT("Off"));
	Shell_NotifyIcon(message,&ico);
	return;
}

LRESULT CALLBACK WindowProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static UINT TaskbarRestart = RegisterWindowMessage(_TEXT("TaskbarCreated"));
	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDM_ABOUT:
			MessageBox(hWnd,_TEXT("A simple lock key state indication software."),_TEXT("About"),MB_OK);
			return 0;
		case IDM_EXIT:
			SendMessage(hWnd,WM_CLOSE,0,0);
			return 0;
		default:
			break;
		}
		break;
	case WM_CREATE:
		GetNCSState(&CurrentLockState);
		NCSTrayIcon(NIM_ADD,hWnd,CurrentLockState);
		hMenu = LoadMenu(((LPCREATESTRUCT)lParam)->hInstance,MAKEINTRESOURCE(IDR_MENU1));
		SetTimer(hWnd,1,50,NULL);
		return 0;
	case WM_DESTROY:
		if (hMenu)
			DestroyMenu(hMenu);
		NCSTrayIcon(NIM_DELETE,hWnd,0);
		KillTimer(hWnd,1);
		PostQuitMessage(0);
		return 0;
	case WM_TIMER:
		{
			int state = 0;
			GetNCSState(&state);
			if (state != CurrentLockState)
			{
				CurrentLockState = state;
				NCSTrayIcon(NIM_MODIFY,hWnd,state);
				MessageBeep(-1);
			}
		}
		return 0;
	case NCS_TRAY_MESSAGE:
		{
			POINT pt = {0};
			if (lParam == WM_RBUTTONDOWN)
			{
				GetCursorPos(&pt);
				if (hMenu)
				{
					SetForegroundWindow(hWnd);
					TrackPopupMenu(GetSubMenu(hMenu,0),TPM_LEFTALIGN | TPM_BOTTOMALIGN,pt.x,pt.y,0,hWnd,NULL);
					PostMessage(hWnd,WM_NULL,0,0);
				}
			}
		}
		return 0;
	default:
		if (TaskbarRestart == uMsg)
		{
			GetNCSState(&CurrentLockState);
			NCSTrayIcon(NIM_ADD,hWnd,CurrentLockState);
		}
		break;
	}
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

int CALLBACK _tWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR lpCmdLine,int nCmdShow)
{
	HWND window = NULL;
	WNDCLASS wc = {0};
	MSG msg = {0};
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = MyClassName;
	if (!RegisterClass(&wc))
		return -1;
	window = CreateWindow(MyClassName,NULL,WS_POPUPWINDOW,0,0,0,0,NULL,NULL,hInstance,NULL);;
	while(GetMessage(&msg,NULL,0,0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
