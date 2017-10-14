// ServerAsyncSelect.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "winsock2.h"

#define WM_SOCKET WM_USER + 1

SOCKET registeredClients[64];
int numRegisteredClients = 0;

char cmd[16], id[64], tmp[64];
char * ids[64];
char sendbuf[1024];

BOOL CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);

int main()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(8000);

	bind(listener, (SOCKADDR *)&addr, sizeof(addr));
	listen(listener, 5);

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

	WSAAsyncSelect(listener, window, WM_SOCKET, FD_ACCEPT);

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

		if (WSAGETSELECTEVENT(lParam) == FD_ACCEPT)
		{
			SOCKET client = accept((SOCKET)wParam, NULL, NULL);
			char *msg = "Hello client.\n";
			send(client, msg, strlen(msg), 0);
			WSAAsyncSelect(client, hDlg, WM_SOCKET, FD_READ | FD_CLOSE);
		}
		else if (WSAGETSELECTEVENT(lParam) == FD_READ)
		{
			char buf[256];
			int res = recv((SOCKET)wParam, buf, sizeof(buf), 0);
			buf[res] = 0;
			printf("%s", buf);

			int j = 0;
			for (; j < numRegisteredClients; j++)
				if (registeredClients[j] == (SOCKET)wParam)
					break;
			if (j >= numRegisteredClients)
			{
				// chua dang nhap
				res = sscanf(buf, "%s %s %s", cmd, id, tmp);
				if (res != 2)
				{
					char * msg = "Wrong format. Please send again.\n";
					send((SOCKET)wParam, msg, strlen(msg), 0);
				}
				else
				{
					if (strcmp(cmd, "client_id:") != 0)
					{
						char * msg = "Wrong format. Please send again.\n";
						send((SOCKET)wParam, msg, strlen(msg), 0);
					}
					else
					{
						// Correct format
						char * msg = "OK. You can send message now.\n";
						send((SOCKET)wParam, msg, strlen(msg), 0);

						registeredClients[numRegisteredClients] = (SOCKET)wParam;
						ids[numRegisteredClients] = (char *)malloc(64);
						memcpy(ids[numRegisteredClients], id, strlen(id) + 1);
						numRegisteredClients++;
					}
				}
			}
			else
			{
				// da dang nhap

				sprintf(sendbuf, "%s: %s", ids[j], buf);

				for (int j = 0; j < numRegisteredClients; j++)
					if (registeredClients[j] != (SOCKET)wParam)
						send(registeredClients[j], sendbuf, strlen(sendbuf), 0);
			}
		}
		else if (WSAGETSELECTEVENT(lParam) == FD_CLOSE)
		{
			closesocket((SOCKET)wParam);
			return TRUE;
		}
	}
}

