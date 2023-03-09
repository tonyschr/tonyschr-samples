#include "pch.h"
#include "MainWindow.h"
#include "HostedWindow.h"
#include "WindowlessHostControl.h"

const int MENUITEM_TEST_EXTERNAL_OUTPUT_LINK = 5000;

void MainWindow::Create()
{
    WNDCLASSEXW wcex = {0};
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = MainWindow::StaticWindowProc;
    wcex.hInstance = g_instance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"MainWindowClass";
    wcex.hIconSm = NULL;
    RegisterClassExW(&wcex);

    m_hwnd =
        CreateWindowEx(0, L"MainWindowClass", L"WinUI MainWindow", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT,
                       CW_USEDEFAULT, 1200, 800, nullptr, CreateMainMenu(), g_instance, static_cast<void *>(this));
    ShowWindow(m_hwnd, SW_SHOW);

    // Initialize the base XAML Island, which we'll later dynamically add
    // content to.
    m_island.Initialize(this);
    m_island.LoadFromResource(IDR_XAML_MAINISLAND);

    RECT rc = {0};
    GetClientRect(m_hwnd, &rc);
    OnSize(rc.right - rc.left, rc.bottom - rc.top);
}

HMENU MainWindow::CreateMainMenu()
{
    HMENU menu = CreateMenu();
    HMENU testCases = CreatePopupMenu();
    AppendMenu(testCases, MF_STRING, MENUITEM_TEST_EXTERNAL_OUTPUT_LINK,
               L"WinUI3 / DComp Test: Test WinUI3 Exernal Output Link");
    AppendMenu(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(testCases), L"Test Cases");

    return menu;
}

LRESULT CALLBACK MainWindow::StaticWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    MainWindow *window = nullptr;

    if (msg == WM_NCCREATE)
    {
        window = reinterpret_cast<MainWindow *>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<INT_PTR>(window));
    }
    else
    {
        window = reinterpret_cast<MainWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (window)
    {
        return window->WindowProc(hwnd, msg, wParam, lParam);
    }
    else
    {
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    bool handled = false;

    switch (msg)
    {
    case WM_CLOSE:
        result = OnClose();
        handled = true;
        break;

    case WM_ERASEBKGND:
        result = 1;
        handled = true;
        break;

    case WM_PAINT:
        result = OnPaint();
        handled = true;
        break;

    case WM_SIZE:
        result = OnSize(LOWORD(lParam), HIWORD(lParam));
        handled = true;
        break;

    case WM_COMMAND: {
        switch (LOWORD(wParam))
        {
        case MENUITEM_TEST_EXTERNAL_OUTPUT_LINK:
            Test_ContentExternalOutputLink();
            handled = true;
            break;
        }
    }
    }

    if (!handled)
    {
        result = DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return result;
}

LRESULT MainWindow::OnClose()
{
    DestroyWindow(m_hwnd);
    m_hwnd = nullptr;

    PostQuitMessage(0);

    return 0;
}

LRESULT MainWindow::OnPaint()
{
    PAINTSTRUCT ps = {0};
    HDC hdc = BeginPaint(m_hwnd, &ps);
    RECT rc = {0};
    GetClientRect(m_hwnd, &rc);
    FillRect(hdc, &rc, reinterpret_cast<HBRUSH>(GetStockObject(DKGRAY_BRUSH)));
    EndPaint(m_hwnd, &ps);
    return 0;
}

LRESULT MainWindow::OnSize(int width, int height)
{
    m_island.SetSize({0, 0, width, height});
    return 0;
}

void MainWindow::Test_ContentExternalOutputLink()
{
    if (!m_hostedWindow.GetWindow())
    {
        m_hostedWindow.Create();
    }

    winrt::TestWinUIControls::WindowlessHostControl windowlessHost;
    m_island.AddTestElement(windowlessHost);

    windowlessHost.RetargetWindow(winrt::TestWinUIControls::TestMode::TestMode_ContentExternalOutputLink,
                                  reinterpret_cast<uint64_t>(m_island.GetIslandWindow()),
                                  reinterpret_cast<uint64_t>(m_hostedWindow.GetWindow()));
}
