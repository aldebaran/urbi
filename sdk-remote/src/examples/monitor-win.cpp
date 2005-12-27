#include "monitor-win.h"
static LRESULT CALLBACK windowProc(HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
	) {
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
Monitor::Monitor(int w, int h, const char * title) {
	this->w = w;
	this->h = h;
WNDCLASS wc;
wc.style = CS_OWNDC | CS_NOCLOSE;
wc.lpfnWndProc = &windowProc;
wc.cbClsExtra = 0;
wc.cbWndExtra = 0;
wc.hInstance = NULL;//GetCurrentProcess();
wc.hIcon = NULL;
wc.hCursor = NULL;
wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
wc.lpszMenuName = NULL;
wc.lpszClassName = "Monitor";
RegisterClass(&wc);
window = CreateWindow("Monitor",title, WS_OVERLAPPED | WS_VISIBLE, 
					CW_USEDEFAULT, CW_USEDEFAULT, w, h+40, NULL, NULL, NULL, NULL);
//unsigned char * data = (unsigned char *)malloc(w*h*4);
//memDC = CreateCompatibleDC ( hDC );
//memBM = CreateCompatibleBitmap ( hDC , w, h);
//SelectObject ( memDC, memBM );
}

int Monitor::setImage(unsigned char * tdata, int size) {
//handle messages
MSG msg;
while (PeekMessage(&msg, window, 0, 0, PM_REMOVE))
	DispatchMessage(&msg);
BITMAPINFO bmi;
bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
bmi.bmiHeader.biWidth = w;
bmi.bmiHeader.biHeight = -h;
bmi.bmiHeader.biPlanes = 1;
bmi.bmiHeader.biBitCount = 24;
bmi.bmiHeader.biCompression = BI_RGB;
bmi.bmiHeader.biSizeImage = 0;
SetDIBitsToDevice(GetDC(window), 0, 0, w, h, 0, 0, 0, h, tdata, &bmi, DIB_RGB_COLORS);
return 0;
}