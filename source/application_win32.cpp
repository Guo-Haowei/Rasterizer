#include <cstdio>
#include "application.h"
#include "common/core_assert.h"
#include "example_base.h"

#ifdef UNICODE
#undef UNICODE
#endif
#include <Windows.h>

static WNDCLASSEX g_wc;
static HWND g_hWnd;
static HDC g_hDC;
static BITMAP bitmap;
static MSG g_msg;
static bool g_shouldQuit;

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void initialize() {
    g_pExample->initialize();
    g_wc.cbSize = sizeof(WNDCLASSEX);
    g_wc.style = CS_CLASSDC;
    g_wc.lpfnWndProc = WndProc;
    g_wc.cbClsExtra = 0;
    g_wc.cbWndExtra = 0;
    g_wc.hInstance = GetModuleHandle(NULL);
    g_wc.hIcon = NULL;
    g_wc.hCursor = NULL;
    g_wc.hbrBackground = NULL;
    g_wc.lpszMenuName = NULL;
    g_wc.lpszClassName = "MainWindow";
    g_wc.hIconSm = NULL;

    ::RegisterClassEx(&g_wc);

    g_hWnd = ::CreateWindow(g_wc.lpszClassName,              // lpClassName
                            g_pExample->getTitle().c_str(),  // lpWindowName
                            WS_OVERLAPPED | WS_SYSMENU,      // dwStyle
                            CW_USEDEFAULT,                   // x
                            CW_USEDEFAULT,                   // y
                            g_pExample->getWidth(),          // nWidth
                            g_pExample->getHeight(),         // nHeight
                            NULL,                            // hWndParent
                            NULL,                            // hMenu
                            g_wc.hInstance,                  // hInstance
                            NULL                             // lpParam
    );

    if (!g_hWnd) {
        PANIC("CreateWindow() failed");
    }

    ::ShowWindow(g_hWnd, SW_SHOWDEFAULT);
    ::UpdateWindow(g_hWnd);

    g_hDC = GetDC(g_hWnd);
    // TODO: not sure why is there here..., figure it out!!
    HGDIOBJ hBitmap = GetCurrentObject(g_hDC, OBJ_BITMAP);
    GetObject(hBitmap, sizeof(BITMAP), &bitmap);
}

void finalize() {
    ::ReleaseDC(g_hWnd, g_hDC);
    ::DestroyWindow(g_hWnd);
    ::UnregisterClass(g_wc.lpszClassName, g_wc.hInstance);
}

void drawPixels(int width, int height, const void* data) {
    HBITMAP map = CreateBitmap(width,   // width
                               height,  // height
                               1,       // Color Planes
                               32,      // 4 * 8 bits
                               data);   // pointer to array

    // Temp HDC to copy picture
    HDC src = CreateCompatibleDC(g_hDC);  // hdc - Device context for window, I've got earlier with GetDC(hWnd) or GetDC(NULL);
    SelectObject(src, map);               // Inserting picture into our temp HDC
    // Copy image from temp HDC to window
    BitBlt(g_hDC,            // Destination
           0,                // x and
           0,                // y - upper-left corner of place, where we'd like to copy
           bitmap.bmWidth,   // width of the region
           bitmap.bmHeight,  // height
           src,              // source
           0,                // x and
           0,                // y of upper left corner  of part of the source, from where we'd like to copy
           SRCCOPY);         // Defined DWORD to juct copy pixels. Watch more on msdn;

    DeleteObject(map);
    DeleteDC(src);  // Deleting temp HDC
}

bool shouldQuit() {
    return g_shouldQuit;
}

void pollEvents() {
    for (;;) {
        if (::PeekMessage(&g_msg, NULL, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&g_msg);
            ::DispatchMessage(&g_msg);
            continue;
        }

        break;
    }
}

void setWindowTitle(const char* title) {
    ::SetWindowTextA(g_hWnd, title);
}

static bool g_keyStatus[1024];

bool getKey(enum KeyCode keycode) {
    return g_keyStatus[keycode];
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_DESTROY:
            ::PostQuitMessage(0);
            g_shouldQuit = true;
            break;
        case WM_KEYDOWN:
            ASSERT(wParam >= 0 && wParam <= sizeof(g_keyStatus));
            g_keyStatus[wParam] = true;
            // printf("%d\n", wParam);
            break;
        case WM_KEYUP:
            ASSERT(wParam >= 0 && wParam <= sizeof(g_keyStatus));
            g_keyStatus[wParam] = false;
            break;
        default:
            break;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
