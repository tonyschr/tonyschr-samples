#include "pch.h"
#include "HostedWindow.h"

void HostedWindow::Create()
{
    WNDCLASSEXW wcex = {0};
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = HostedWindow::StaticWindowProc;
    wcex.hInstance = g_instance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"HostedWindowClass";
    wcex.hIconSm = NULL;
    RegisterClassExW(&wcex);

    m_hwnd = CreateWindowEx(0, L"HostedWindowClass", L"HostedWindow", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 10, 10,
                            400, 300, nullptr, nullptr, g_instance, static_cast<void *>(this));
    ShowWindow(m_hwnd, SW_SHOW);

    SetTimer(m_hwnd, 42, 30, nullptr);
}

LRESULT CALLBACK HostedWindow::StaticWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static int number = 0;
    LRESULT result = 0;
    bool handled = false;

    switch (msg)
    {
    case WM_ERASEBKGND:
        result = 1;
        handled = true;
        break;

    case WM_PAINT: {
        RECT rc = {0};
        PAINTSTRUCT ps = {0};
        HDC hdc = BeginPaint(hwnd, &ps);
        GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));

        rc.top = 10;
        rc.left = 10;
        WCHAR buffer[64];
        StringCchPrintf(buffer, ARRAYSIZE(buffer), L"Test #%d", number);
        DrawTextW(hdc, buffer, -1, &rc, DT_WORDBREAK);

        EndPaint(hwnd, &ps);

        result = 0;
        handled = true;
    }
    break;

    case WM_TIMER:
        number++;
        InvalidateRect(hwnd, nullptr, TRUE);
        break;
    }

    if (!handled)
    {
        result = DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return result;
}