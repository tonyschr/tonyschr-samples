#pragma once

// Simple small window that just paints with an updated number
// every 30 ms.

class HostedWindow
{
  public:
    HostedWindow() = default;

    void Create();
    HWND GetWindow()
    {
        return m_hwnd;
    }

  private:
    HWND m_hwnd{nullptr};

    static LRESULT CALLBACK StaticWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
