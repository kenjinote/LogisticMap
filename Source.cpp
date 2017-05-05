#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <windows.h>
#include <math.h>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define WM_EXITTHREAD (WM_APP+100)

TCHAR szClassName[] = TEXT("Window");
BOOL bAbort;
HDC hMemDC;
HWND hWnd;

double logistic(double a, double x)
{
	return a*x*(1 - x);
}

DWORD WINAPI ThreadFunc(LPVOID)
{
	int dispX;
	int dispY;
	int i, j;
	const double x0 = 0.1;
	double x;
	double a = 2.95;
	BYTE r;
	BYTE g;
	BYTE b;
	COLORREF color;
	for (j = 0; a <= 4.0 && !bAbort; j++)
	{
		a += 0.0001;
		x = x0;
		for (i = 0; i<1000; i++)
		{
			x = logistic(a, x);
		}
		for (i = 0; i<1000; i++)
		{
			x = logistic(a, x);
			dispX = (int)(WINDOW_WIDTH  * (a - 2.95)*3.81 / 4.0);
			dispY = (int)(WINDOW_HEIGHT - WINDOW_HEIGHT * x);
			color = GetPixel(hMemDC, dispX, dispY);
			r = GetRValue(color); if (r>0) r--;
			g = GetGValue(color); if (g>0) g--;
			b = GetBValue(color); if (b>0) b--;
			SetPixelV(hMemDC, dispX, dispY, RGB(r, g, b));
		}
		InvalidateRect(hWnd, 0, 0);
	}
	PostMessage(hWnd, WM_EXITTHREAD, 0, 0);
	ExitThread(0);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HANDLE hThread;
	static HBITMAP hBitmap, hOldBitmap;
	static DWORD dwParam;
	switch (msg)
	{
	case WM_CREATE:
	{
		HDC hdc = GetDC(hWnd);
		hMemDC = CreateCompatibleDC(hdc);
		hBitmap = CreateCompatibleBitmap(hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
		hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
		ReleaseDC(hWnd, hdc);
		PatBlt(hMemDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, WHITENESS);
		hThread = CreateThread(0, 0, ThreadFunc, 0, 0, &dwParam);
	}
	break;
	case WM_EXITTHREAD:
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
		hThread = 0;
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, hMemDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		if (hThread)
		{
			bAbort = TRUE;
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
			hThread = 0;
		}
		SelectObject(hMemDC, hOldBitmap);
		DeleteObject(hBitmap);
		DeleteDC(hMemDC);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass = {
		0,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0, IDC_ARROW),
		0,
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	RECT rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	AdjustWindowRect(&rect, WS_CAPTION | WS_SYSMENU, 0);
	hWnd = CreateWindow(
		szClassName,
		TEXT("logistic map"),
		WS_CAPTION | WS_SYSMENU,
		CW_USEDEFAULT,
		0,
		rect.right - rect.left,
		rect.bottom - rect.top,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
