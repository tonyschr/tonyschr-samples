#pragma once

#include "XamlIsland.h"
#include "HostedWindow.h"

class MainWindow
{
  public:
    MainWindow() = default;

    void Create();
    HWND GetWindow()
    {
        return m_hwnd;
    }

  private:
    static LRESULT CALLBACK StaticWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HMENU CreateMainMenu();
    LRESULT OnClose();
    LRESULT OnPaint();
    LRESULT OnSize(int width, int height);

    // Test cases
    void Test_ContentExternalOutputLink();

    HWND m_hwnd{nullptr};
    XamlIsland m_island;
    HostedWindow m_hostedWindow;
};
