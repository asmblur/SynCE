#include "snap.h"

#define MOUSE_PRESSED	1
#define MOUSE_RELEASED  2
#define MOUSE_MOVED		3

#define LEFT_BUTTON		1
#define RIGHT_BUTTON	2
#define MID_BUTTON		3

static SOCKET as;
static HANDLE sigEvent;

DWORD WINAPI snapProc(LPVOID lpParameter)
{
	Snap *snap = new Snap();
	Snap::SnapImage image = snap->createSnapImage();
	Snap::SnapImage image2 = snap->createSnapImage();
	bool ret = true;
	bool written = false;

	do {
		snap->snap(image2);
		snap->xorBits(image, image2);
		ret = snap->writeSocketRLE(as, image, &written);
		snap->exchangeImages(image, image2);
		if (!written) {
			WaitForSingleObject(sigEvent, 300);
		} else {
			Sleep(50);
		}
	} while (ret);

	return 0;
}


int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPTSTR    lpCmdLine,
					int       nCmdShow)
{
    struct sockaddr_in inet_address;
	inet_address.sin_family = AF_INET;
	inet_address.sin_port = htons(1234);
	unsigned int inaddr_any = INADDR_ANY;
	WORD wVersion;
	WSADATA wsaData;	
	SOCKET s;
	int lo, hi;

	sigEvent = CreateEvent (NULL, FALSE, FALSE, TEXT("WriteEvent"));


	int min = 1, maj = 1;

	wVersion = MAKEWORD(maj, min);
	
	int err = WSAStartup(wVersion, &wsaData);

	if (err != 0) {
		MessageBox(NULL, L"Invalid WSAStartupcall", L"Snap", MB_OK);
		return 1;
	}

	lo = LOBYTE(wsaData.wVersion);
	hi = HIBYTE(wsaData.wVersion);

	if ((lo != maj) || (hi != min)) {
		WSACleanup();
		MessageBox(NULL, L"Winsock-Version not supported", L"Snap", MB_OK);

		return 1;
	}

	s = socket(AF_INET, SOCK_STREAM, 0);

	if (s == INVALID_SOCKET) {
		MessageBox(NULL, L"Could not create ServerSocket", L"Snap", MB_OK);
		return 1;
	}

	memcpy(&inet_address.sin_addr, &inaddr_any, sizeof(INADDR_ANY));

	if (bind(s, (const sockaddr *) &inet_address, 
			sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
		MessageBox(NULL, L"Could not bind ServerSocket", L"Snap", 
				MB_OK | MB_ICONERROR);
		closesocket(s);
		return 1;
	}

	if (listen(s, 5) == SOCKET_ERROR) {
		MessageBox(NULL, L"Could not listen on ServerSocket", L"Snap", 
				MB_OK | MB_ICONERROR);
		closesocket(s);
		return 1;
	}

	as = accept(s, NULL, NULL);

	if (as == INVALID_SOCKET) {
		MessageBox(NULL, L"Could not accept on ServerSocket", L"Snap", 
				MB_OK | MB_ICONERROR);
		shutdown(as, 2);
		return 1;
	} else {
		fd_set readFd;
		fd_set writeFd;
		fd_set exceptFd;

		struct timeval tv = {0, 500000};
		int selret;
		bool ret = true;
		int r = 1;
		char buf[4 * sizeof(u_long)];
		u_long x, y, button, cmd;
		DWORD dwFlags, dx, dy, dwData = 0, dwExtraInfo = 0;
		DWORD threadId;
		HANDLE threadHandle;

		FD_ZERO(&readFd);
		FD_ZERO(&writeFd);
		FD_ZERO(&exceptFd);
		
		threadHandle = CreateThread(NULL, 0, snapProc, NULL, 0, &threadId);

		if (threadHandle == NULL) {
			MessageBox(NULL, L"Could not create Thread", L"Snap", 
					MB_OK | MB_ICONERROR);
			return 1;
		}

		if (SetThreadPriority(threadHandle, THREAD_PRIORITY_BELOW_NORMAL) == 0) {
			MessageBox(NULL, L"Could not set ThreadPriority", L"Snap", 
					MB_OK | MB_ICONWARNING);
		}

		do {
			tv.tv_sec = 0;
			tv.tv_usec = 100000;
	
			FD_SET(as, &readFd);
			selret = select(as + 1, &readFd, &writeFd, &exceptFd, &tv);

			if (selret > 0) {
				r = recv(as, (char *) buf, sizeof(u_long) * 4, 0);
					
				if (r == 0) {
					break;
				}
				if (r < 0) {
					MessageBox(NULL, L"Could not read on Socket", L"Snap", 
							MB_OK | MB_ICONERROR);
					break;
				}
				button = ntohl(*(u_long *) &buf[sizeof(UINT32) * 0]);
				cmd = ntohl(*(u_long *) &buf[sizeof(UINT32) * 1]);
				x = ntohl(*(u_long *) &buf[sizeof(UINT32) * 2]);
				y = ntohl(*(u_long *) &buf[sizeof(UINT32) * 3]);
					
				dx = x;
				dy = y;
				dwFlags = MOUSEEVENTF_ABSOLUTE;
				switch (cmd) {
				case MOUSE_PRESSED:
					switch(button) {
					case LEFT_BUTTON:
						dwFlags |= MOUSEEVENTF_LEFTDOWN;
						break;
					case RIGHT_BUTTON:
						dwFlags |= MOUSEEVENTF_RIGHTDOWN;
						break;
					case MID_BUTTON:
						dwFlags |= MOUSEEVENTF_MIDDLEDOWN;
						break;
					}
					break;
				case MOUSE_RELEASED:
					switch(button) {
					case LEFT_BUTTON:
						dwFlags |= MOUSEEVENTF_LEFTUP;
						break;
					case RIGHT_BUTTON:
						dwFlags |= MOUSEEVENTF_RIGHTUP;
						break;
					case MID_BUTTON:
						dwFlags |= MOUSEEVENTF_MIDDLEUP;
						break;
					}
					break;
				case MOUSE_MOVED:
					dwFlags |= MOUSEEVENTF_MOVE;
					switch(button) {
					case LEFT_BUTTON:
						break;
					case RIGHT_BUTTON:
						break;
					case MID_BUTTON:
						break;
					}
					break;
				}
				mouse_event(dwFlags, dx, dy, dwData, dwExtraInfo);
				if (PulseEvent(sigEvent) == 0){
					MessageBox(NULL, L"Could not Pulse Event", L"Snap", 
						MB_OK | MB_ICONERROR);
				}
			}
		} while (r > 0);

		if (shutdown(as, 2) == SOCKET_ERROR) {
			closesocket(as);
			MessageBox(NULL, L"Could not shut down acepted socket", L"Snap", 
					MB_OK | MB_ICONERROR);
		}
	}

	if (closesocket(s) == SOCKET_ERROR) {
		MessageBox(NULL, L"Could not shut down listen socket", L"Snap", 
				MB_OK | MB_ICONERROR);
	}

	CloseHandle(sigEvent);

	return 0;
}