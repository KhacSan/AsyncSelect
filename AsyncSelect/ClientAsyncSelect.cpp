// AsyncSelect.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "winsock2.h"

#define WM_SOCKET WM_USER + 1

BOOL CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);

int main()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(8888);

	int res = connect(client, (SOCKADDR *)&addr, sizeof(addr));
	if (res < 0) {
		return 0;
	}
	WNDCLASS wndclass;
	CHAR *providerClass = "AsyncSelect";
	HWND window;

	wndclass.style = 0;
	wndclass.lpfnWndProc = (WNDPROC)WinProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = NULL;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = (LPCWSTR)providerClass;

	if (RegisterClass(&wndclass) == 0)
		return NULL;

	// Create a window
	if ((window = CreateWindow((LPCWSTR)providerClass, L"", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, NULL, NULL)) == NULL)
		return NULL;

	WSAAsyncSelect(client, window, WM_SOCKET, FD_READ | FD_CLOSE);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

BOOL CALLBACK WinProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	if (wMsg == WM_SOCKET)
	{
		if (WSAGETSELECTERROR(lParam))
		{
			closesocket((SOCKET)wParam);
			return TRUE;
		}

		if (WSAGETSELECTEVENT(lParam) == FD_READ)
		{
			char buf[256];
			int res = recv((SOCKET)wParam, buf, sizeof(buf), 0);
			buf[res] = 0;
			printf("%s", buf);
		}
		else if (WSAGETSELECTEVENT(lParam) == FD_CLOSE) {
			closesocket((SOCKET)wParam);
			return TRUE;
		}
	}
}



