#include "pch.h"
#include "MainWindow.h"

int __stdcall wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int)
{
    g_instance = instance;
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    MainWindow window;
    window.Create();

    MSG msg = {0};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (msg.message == WM_QUIT)
        {
            break;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
