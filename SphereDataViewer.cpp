// SphereDataViewer.cpp : Defines the entry point for the application.

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
#define _USE_MATH_DEFINES

#include <math.h>
#include <windows.h>

#include "resource.h"
#include "Timer.h"
#include "Test/SphereData.h"
#include "Test/FrameBuffer.h"


CSphereData g_Data("sphere_sample_points.txt");
//CSphereData g_Data("sphere_sample_points_min.txt");
CFrameBuffer g_Framebuffer(1024, 1024);

// Initial orientation
static const float INITIAL_ANGLE = M_PI / 3;
static const float ANGLE_DELTA = 0.05f;
static const float ANGLE_AUTO_ROTATION = 0.05f;
static const bool AUTO_ROTATION = true;

static const int NUM_TIME_HISTORY = 16;


//////////////////////////////////////////////////////////////////////////////////
class CViewer
{
public:
	CViewer()
	{
		m_wi = m_wi_last = INITIAL_ANGLE;
		m_fAnimateAngleRatio = ANGLE_AUTO_ROTATION;
		m_autoRotation = AUTO_ROTATION;

		m_accumulatedRenderTime = 0;
		m_lastFullRotationTime = 0;
		m_averageFrameTime = 0;
		hbmMem = 0;
		hdcMem = 0;
		m_nFrame = 0;
		m_curTimeHistory = 0;
	}

	void RenderFrame(HDC hdc)
	{
		double t0 = Timer::GetMillisFloat();
		if (m_wi != m_wi_last) {
			g_Framebuffer.Clear();
			g_Data.Render(g_Framebuffer, m_wi);
		}
		PaintFrameBuffer(hdc);

		double t1 = Timer::GetMillisFloat();
		SetRenderTime(t1 - t0);

		m_wi_last = m_wi;

		if (m_autoRotation)
		{
			m_wi += m_fAnimateAngleRatio;
			if (m_wi >= 2 * M_PI)
			{
				m_lastFullRotationTime = m_accumulatedRenderTime;
				m_accumulatedRenderTime = 0;
				m_wi = 0;
			}
		} // if m_autoRotation
	}

	void IncreaseAngle()
	{
		m_wi += ANGLE_DELTA;
	}

	void DecreaseAngle()
	{
		m_wi -= ANGLE_DELTA;
	}

	void SetAutoRotation(bool v)
	{
		m_autoRotation = v;
	}


private:
	void PaintFrameBuffer(HDC hdc)
	{
		m_nFrame++;

		const int iWidth = g_Framebuffer.GetWidth();
		const int iHeight = g_Framebuffer.GetHeight();

		// Create an off-screen DC for double-buffering
		if (!hbmMem)
		{
			hdcMem = CreateCompatibleDC(hdc);
			hbmMem = CreateCompatibleBitmap(hdc, iWidth, iHeight);
		}

		const CFrameBuffer::color_t* p = g_Framebuffer.GetFrameBuffer();

		// Draw back buffer
		HANDLE hOld = SelectObject(hdcMem, hbmMem);

		SetBitmapBits(
			hbmMem,
			sizeof(CFrameBuffer::frameBuffer_t::value_type) * iWidth * iHeight, p);

		//////////////////////////////////////////////////////////////////////////////////
		// Display FPS
		//////////////////////////////////////////////////////////////////////////////////
		float fps = (float)(1000 / m_averageFrameTime);
		char str[1024];
		sprintf_s(str, "Frame: %d:  FPS: %.1f ms, %.2f ms",
			m_nFrame, fps, m_averageFrameTime);
		TextOut(hdcMem, 0, 0, str, (int)strlen(str));

		sprintf_s(str, "Turn time: %.2f s",
			(float)(m_lastFullRotationTime / 1000));
		TextOut(hdcMem, 0, 16, str, (int)strlen(str));

		const char* s = "> Press arrows for manual rotation.";
		TextOut(hdcMem, 0, 32, s, (int)strlen(s));
		//////////////////////////////////////////////////////////////////////////////////

		// Transfer the off-screen DC to the screen
		BitBlt(hdc, 0, 0, iWidth, iHeight, hdcMem, 0, 0, SRCCOPY);

		// Free-up the off-screen DC
		SelectObject(hdcMem, hOld);
	}

	//////////////////////////////////////////////////////////////////////////
	void SetRenderTime(double t)
	{
		m_accumulatedRenderTime += t;
		m_timeHistory[m_curTimeHistory] = t;
		if (++m_curTimeHistory >= NUM_TIME_HISTORY)
			m_curTimeHistory = 0;

		double mint = 100000;
		double avgt = 0;
		for (int i = 0; i < NUM_TIME_HISTORY; i++)
		{
			avgt += m_timeHistory[i];
			if (m_timeHistory[i] < mint)
				mint = m_timeHistory[i];
		}
		avgt /= NUM_TIME_HISTORY;

		// Smooth out fps (disabled)
		float ratio = 1.0f;
		m_averageFrameTime =
			m_averageFrameTime * (1.0f - ratio) + avgt * ratio;
	}


private:
	int m_nFrame;
	HDC hdcMem;
	HBITMAP hbmMem;

	double m_accumulatedRenderTime;
	double m_lastFullRotationTime;

	double m_timeHistory[NUM_TIME_HISTORY];
	double m_averageFrameTime;
	int m_curTimeHistory;

	float m_wi;
	float m_wi_last;
	float m_fAnimateAngleRatio;
	bool m_autoRotation;
};


CViewer g_viewer;


//////////////////////////////////////////////////////////////////////////////////
static const int MAX_LOADSTRING = 100;

// Global Variables:
HWND g_hWnd = 0;
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
//////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SPHEREDATAVIEWER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(
		hInstance, MAKEINTRESOURCE(IDC_SPHEREDATAVIEWER));

	HDC hdc = GetDC(g_hWnd);

	while (true)
	{
		// Main message loop:
		while (PeekMessage(&msg, NULL, 0, 0, TRUE))
		{
			if (msg.message == WM_QUIT)
				return 0;
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		InvalidateRect(g_hWnd, 0, FALSE);
	}

	ReleaseDC(g_hWnd, hdc);

	return (int)msg.wParam;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SPHEREDATAVIEWER));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_SPHEREDATAVIEWER);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}


//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle
//        in a global variable and create and display
//        the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	Timer::Init();

	int width = g_Framebuffer.GetWidth();
	int height = g_Framebuffer.GetHeight();
	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	g_hWnd = hWnd;
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_KEYDOWN  - pressed keys on a keyboard
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_KEYDOWN:
		// any key stops auto rotation
		g_viewer.SetAutoRotation(false);
		switch (wParam)
		{
		case VK_LEFT:
			g_viewer.IncreaseAngle();
			break;

		case VK_RIGHT:
			g_viewer.DecreaseAngle();
			break;
		}
		break;

	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;

		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);
		g_viewer.RenderFrame(hdc);
		EndPaint(hWnd, &ps);
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


// Message handler for about box.
INT_PTR CALLBACK About(
	HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
