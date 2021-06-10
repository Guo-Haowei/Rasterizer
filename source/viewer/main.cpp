#include <Windows.h>
#undef min
#undef max
#include <stdio.h>
#include "BaseApp.h"

constexpr int WIDTH = 1200;
constexpr int HEIGHT = 800;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
        default:
            return ::DefWindowProc(hWnd, msg, wParam, lParam);
    }
}

using namespace hguo;

int main(int, char**) {
    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX),     // cbSize
        CS_CLASSDC,             // sylte
        WndProc,                // plfnWndProc
        0,                      // cbClsExtra
        0,                      // cbWndExxtra
        GetModuleHandle(NULL),  // hInstance
        NULL,                   // hIcon
        NULL,                   // hCursor
        NULL,                   // hbrBackground
        NULL,                   // lpszMenuName
        "MainWindow",           // lpszClassName
        NULL,                   // hIconSm
    };

    ::RegisterClassEx(&wc);

    HWND hWnd = ::CreateWindow(wc.lpszClassName,            // lpClassName
                               g_pApp->getTitle().c_str(),  // lpWindowName
                               WS_OVERLAPPED | WS_SYSMENU,  // dwStyle
                               CW_USEDEFAULT,               // x
                               CW_USEDEFAULT,               // y
                               g_pApp->getWidth(),          // nWidth
                               g_pApp->getHeight(),         // nHeight
                               NULL,                        // hWndParent
                               NULL,                        // hMenu
                               wc.hInstance,                // hInstance
                               NULL                         // lpParam
    );

    if (!hWnd) {
        printf("Failed to create window");
        return 1;
    }

    ::ShowWindow(hWnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hWnd);

    g_pApp->initialize();

    // TODO: allow resizing
    HDC hDC = GetDC(hWnd);
    BITMAP bitmap {};
    HGDIOBJ hBitmap = GetCurrentObject(hDC, OBJ_BITMAP);
    GetObject(hBitmap, sizeof(BITMAP), &bitmap);

    MSG msg {};
    while (msg.message != WM_QUIT) {
        if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            continue;
        }

        // render
        // FIXME: dummy elapsed time
        g_pApp->update(.003);
        const Texture& buffer = g_pApp->getTexture();

        // Creating temp bitmap
        HBITMAP map = CreateBitmap(bitmap.bmWidth,     // width
                                   bitmap.bmHeight,    // height
                                   1,                  // Color Planes
                                   32,                 // 4 * 8 bits
                                   buffer.getData());  // pointer to array

        // Temp HDC to copy picture
        HDC src = CreateCompatibleDC(hDC);  // hdc - Device context for window, I've got earlier with GetDC(hWnd) or GetDC(NULL);
        SelectObject(src, map);             // Inserting picture into our temp HDC
        // Copy image from temp HDC to window
        BitBlt(hDC,              // Destination
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

    ::ReleaseDC(hWnd, hDC);
    ::DestroyWindow(hWnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);
    return 0;
}
