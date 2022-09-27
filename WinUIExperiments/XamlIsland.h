#pragma once

#include "WinRTHelpers.h"

class MainWindow;

class XamlIsland
{
  public:
    XamlIsland() = default;

    void Initialize(MainWindow *MainWindow);
    bool LoadFromFile(std::wstring const &fileName);
    bool LoadFromResource(uint32_t id);
    void SetSize(RECT rect);
    HWND GetIslandWindow();

    void AddTestElement(winrt::Microsoft::UI::Xaml::UIElement element);

  private:
    winrt::Hosting::DesktopWindowXamlSource m_desktopWindowXamlSource{nullptr};
    winrt::com_ptr<IDesktopWindowXamlSourceNative> m_desktopWindowXamlSourceNative;
    winrt::StackPanel m_rootElement{nullptr};
};
