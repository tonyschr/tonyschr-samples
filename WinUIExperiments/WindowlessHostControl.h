#pragma once
#include "TestWinUIControls.WindowlessHostControl.g.h"

#include "WinRTHelpers.h"

namespace winrt::TestWinUIControls::implementation
{
struct WindowlessHostControl : WindowlessHostControlT<WindowlessHostControl>
{
    WindowlessHostControl();
    ~WindowlessHostControl();

    void RetargetWindow(TestMode testMode, UINT64 rootWindow, UINT64 hostedWindow);

  private:
    TestMode m_testMode;
    HWND m_xamlIslandWindow{nullptr};
    HWND m_hostedWindow{nullptr};

    // NOTE: Demo app does not yet have high DPI support.
    float m_rasterizationScale{1.0f};

    // For sizing/testing use a grid where we can set the bounds.
    winrt::Grid m_mainContent = winrt::Grid{nullptr};

    // "Classic" DComp visual created using DCompositionCreateDevice3, with
    // the content set from CreateSurfaceFromHwnd.
    winrt::com_ptr<IDCompositionVisual2> m_hostedVisual;

    // Used by boilerplate initialization of m_dcompDevice.
    winrt::com_ptr<ID3D11Device> m_d3d11Device;
    winrt::com_ptr<ID3D11DeviceContext> m_d3d11DeviceContext;
    winrt::com_ptr<ID2D1Factory1> m_d2d1Factory;
    winrt::com_ptr<ID2D1Device> m_d2d1Device;
    winrt::com_ptr<ID2D1DeviceContext> m_d2d1DeviceContext;

    winrt::com_ptr<IDCompositionDesktopDevice> m_dcompDevice;

    winrt::Microsoft::UI::Content::IContentExternalOutputLink m_outputLink;

    void AddChildPanel();
    void CreateHostedVisual();
    void SetHostedVisual();
    void SetHostedVisualSize(float width, float height);

    void InitializeDirectComposition();
    void CreateD3D11Device();
    void DestroyD3D11Device();
    void CreateD2D1Factory();
    void DestroyD2D1Factory();
    void CreateD2D1Device();
    void DestroyD2D1Device();
    void CreateDCompositionDevice();
    void DestroyDCompositionDevice();

    winrt::Compositor ElementCompositor();
};

} // namespace winrt::TestWinUIControls::implementation
namespace winrt::TestWinUIControls::factory_implementation
{
struct WindowlessHostControl : WindowlessHostControlT<WindowlessHostControl, implementation::WindowlessHostControl>
{
};
} // namespace winrt::TestWinUIControls::factory_implementation
