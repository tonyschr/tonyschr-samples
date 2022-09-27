#include "pch.h"
#include "WindowlessHostControl.h"
#include "TestWinUIControls.WindowlessHostControl.g.cpp"

namespace winrt::TestWinUIControls::implementation
{

WindowlessHostControl::WindowlessHostControl()
{
    InitializeDirectComposition();
    AddChildPanel();
}

WindowlessHostControl::~WindowlessHostControl()
{
    DestroyDCompositionDevice();
    DestroyD2D1Device();
    DestroyD2D1Factory();
    DestroyD3D11Device();
}

void WindowlessHostControl::RetargetWindow(TestMode testMode, UINT64 rootWindow, UINT64 hostedWindow)
{
    m_testMode = testMode;
    m_xamlIslandWindow = reinterpret_cast<HWND>(rootWindow);
    m_hostedWindow = reinterpret_cast<HWND>(hostedWindow);

    CreateHostedVisual();
    SetHostedVisual();
}

void WindowlessHostControl::InitializeDirectComposition()
{
    CreateD3D11Device();
    CreateD2D1Factory();
    CreateD2D1Device();
    CreateDCompositionDevice();
}

void WindowlessHostControl::CreateD3D11Device()
{
    HRESULT hr = E_FAIL;

    D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
    };

    for (auto &driverType : driverTypes)
    {
        D3D_FEATURE_LEVEL featureLevelSupported;
        winrt::com_ptr<ID3D11Device> d3d11Device;
        winrt::com_ptr<ID3D11DeviceContext> d3d11DeviceContext;
        hr = D3D11CreateDevice(nullptr, driverType, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0,
                               D3D11_SDK_VERSION, d3d11Device.put(), &featureLevelSupported, d3d11DeviceContext.put());
        if (SUCCEEDED(hr))
        {
            m_d3d11Device = d3d11Device;
            m_d3d11DeviceContext = d3d11DeviceContext;
            break;
        }
    }

    winrt::check_hresult(hr);
}

void WindowlessHostControl::DestroyD3D11Device()
{
    m_d3d11DeviceContext = nullptr;
    m_d3d11Device = nullptr;
}

void WindowlessHostControl::CreateD2D1Factory()
{
    winrt::check_hresult(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_d2d1Factory.put()));
}

void WindowlessHostControl::DestroyD2D1Factory()
{
    m_d2d1Factory = nullptr;
}

void WindowlessHostControl::CreateD2D1Device()
{
    winrt::com_ptr<IDXGIDevice> dxgiDevice;
    winrt::check_hresult(m_d3d11Device->QueryInterface(dxgiDevice.put()));
    winrt::check_hresult(m_d2d1Factory->CreateDevice(dxgiDevice.get(), m_d2d1Device.put()));
    winrt::check_hresult(
        m_d2d1Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, m_d2d1DeviceContext.put()));
}

void WindowlessHostControl::DestroyD2D1Device()
{
    m_d2d1DeviceContext = nullptr;
    m_d2d1Device = nullptr;
}

void WindowlessHostControl::CreateDCompositionDevice()
{
    winrt::com_ptr<IDXGIDevice> dxgiDevice;
    winrt::check_hresult(m_d3d11Device->QueryInterface(dxgiDevice.put()));

    winrt::check_hresult(DCompositionCreateDevice3(dxgiDevice.get(), __uuidof(IDCompositionDesktopDevice),
                                                   reinterpret_cast<void **>(m_dcompDevice.put())));
}

void WindowlessHostControl::DestroyDCompositionDevice()
{
    m_dcompDevice = nullptr;
}

winrt::Compositor WindowlessHostControl::Compositor()
{
    return winrt::ElementCompositionPreview::GetElementVisual(*this).Compositor();
}

void WindowlessHostControl::AddChildPanel()
{
    m_mainContent = winrt::Grid();
    m_mainContent.Background(winrt::SolidColorBrush(winrt::Colors::Blue()));
    m_mainContent.Width(400);
    m_mainContent.Height(300);
    Content(m_mainContent);
}

// Use CreateSurfaceFromHwnd to create a classic IDCompositionVisual, with
// the content coming from CreateSurfaceFromHwnd.
void WindowlessHostControl::CreateHostedVisual()
{
    SetWindowLongPtr(m_hostedWindow, GWL_EXSTYLE, WS_EX_LAYERED);
    SetLayeredWindowAttributes(m_hostedWindow, RGB(0, 0, 255), 255, LWA_ALPHA);

    // Reparent and cloak. For debugging purposes this is optional.
    if (m_xamlIslandWindow)
    {
        SetParent(m_hostedWindow, m_xamlIslandWindow);
        SetWindowLongPtr(m_hostedWindow, GWL_STYLE, WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
        BOOL cloak = TRUE;
        winrt::check_hresult(DwmSetWindowAttribute(m_hostedWindow, DWMWA_CLOAK, &cloak, sizeof(cloak)));
    }

    winrt::com_ptr<IUnknown> retargetedWindowSurface;
    winrt::check_hresult(m_dcompDevice->CreateSurfaceFromHwnd(m_hostedWindow, retargetedWindowSurface.put()));
    winrt::check_hresult(m_dcompDevice->CreateVisual(m_hostedVisual.put()));
    winrt::check_hresult(m_hostedVisual->SetContent(retargetedWindowSurface.get()));
}

// From: https://github.com/microsoft/microsoft-ui-xaml/blob/main/dev/WebView2/WebView2.cpp
//
//  void WebView2::CreateAndSetVisual()
//  {
//    if (!m_visual)
//    {
//      m_visual = winrt::Window::Current().Compositor().CreateSpriteVisual();
//    }
//    UpdateDefaultVisualBackgroundColor();
//
//    SetCoreWebViewAndVisualSize(static_cast<float>(ActualWidth()), static_cast<float>(ActualHeight()));
//
//    winrt::ElementCompositionPreview::SetElementChildVisual(*this, m_visual);
//
//    auto coreWebView2CompositionControllerInterop =
//    m_coreWebViewCompositionController.as<ICoreWebView2CompositionControllerInterop>();
//    winrt::check_hresult(coreWebView2CompositionControllerInterop->put_RootVisualTarget(m_visual.as<::IUnknown>().get()));
//  }

// This code does what the WebView2 team mentioned put_RootVisualTarget does internally.
void WindowlessHostControl::Test_TryInteropWebView2Style()
{
    assert(m_spriteVisual); // Microsoft.UI.Composition::SpriteVisual
    assert(m_hostedVisual); // IDCompositionVisual2, created using DCompositionCreateDevice3

    auto target = m_spriteVisual.try_as<IDCompositionTarget>();
    assert(target); // FAILS. QI'ing a WinUI3 visual for IDCompositionTarget returns E_NOINTERFACE

    target->SetRoot(m_hostedVisual.get());

    // Assuming this is needed. Also, do we need a commit on the underlying WinUI 3 device?
    m_dcompDevice->Commit();
}

// Try to use the WinUI3 compositor as IDCompositionDesktopDevice, which is internally
// is known as dcompi!Microsoft::UI::Composition::InteropCompositor based on the vtable.
//
// This is based on a conceptual idea that we could use an the container visual created
// here as a bridge between WinUI visuals and IDCompositionVisual visuals.
void WindowlessHostControl::Test_TryInteropCompositor()
{
    assert(m_spriteVisual); // Microsoft.UI.Composition::SpriteVisual
    assert(m_hostedVisual); // IDCompositionVisual2, created using DCompositionCreateDevice3

    auto interopDevice = Compositor().try_as<IDCompositionDesktopDevice>();
    assert(interopDevice);

    winrt::com_ptr<IDCompositionVisual2> interopContainerVisual;
    winrt::check_hresult(interopDevice->CreateVisual(interopContainerVisual.put()));

    HRESULT hrAddVisual = interopContainerVisual->AddVisual(m_hostedVisual.get(), TRUE, nullptr);
    assert(SUCCEEDED(hrAddVisual)); // FAILS. E_NOINTERFACE.

    auto winui3Visual = interopContainerVisual.try_as<winrt::Visual>();
    assert(winui3Visual);

    m_spriteVisual.Children().InsertAtTop(winui3Visual);

    // Assuming this is needed. Also, do we need a commit on the underlying WinUI 3 device?
    m_dcompDevice->Commit();
}

void WindowlessHostControl::SetHostedVisual()
{
    // Modeled after WebView2, we use a SpriteVisual as the contaimer visual.
    // For debugging, we give it a color and initial size.
    m_spriteVisual = Compositor().CreateSpriteVisual();
    m_spriteVisual.Brush(Compositor().CreateColorBrush({255, 0, 255, 0}));
    m_spriteVisual.Size({300, 300});

    switch (m_testMode)
    {
    case TestMode::TestMode_WebView2:
        Test_TryInteropWebView2Style();
        break;

    case TestMode::TestMode_InteropCompositor:
        Test_TryInteropCompositor();
        break;
    }

    winrt::ElementCompositionPreview::SetElementChildVisual(*this, m_spriteVisual);

    // ISSUE: Note that depending on initialization order and other vagaries, width
    // and height might not be known yet.
    //
    // WORKAROUND: Manually resize the main window.
    SetHostedVisualSize(static_cast<float>(ActualWidth()), static_cast<float>(ActualHeight()));
}

void WindowlessHostControl::SetHostedVisualSize(float width, float height)
{
    float scaled_width = width * m_rasterizationScale;
    float scaled_height = height * m_rasterizationScale;

    if (m_hostedWindow && m_xamlIslandWindow)
    {
        // For input, accessibility, and IMEs we need the hidden input window to
        // match the position and dimensions of the visual in the XAML tree.
        winrt::GeneralTransform transform = TransformToVisual(nullptr);
        auto top_left = transform.TransformPoint(winrt::Windows::Foundation::Point(0, 0));
        SetWindowPos(m_hostedWindow, HWND_TOP, std::lroundf(top_left.X * m_rasterizationScale),
                     std::lroundf(top_left.Y * m_rasterizationScale), std::lroundf(scaled_width),
                     std::lroundf(scaled_height), SWP_SHOWWINDOW);

        // WORKAROUND: Toggle cloak to update the bounds of the content in m_hostedVisual.
        BOOL cloak = FALSE;
        DwmSetWindowAttribute(m_hostedWindow, DWMWA_CLOAK, &cloak, sizeof(cloak));
        cloak = TRUE;
        DwmSetWindowAttribute(m_hostedWindow, DWMWA_CLOAK, &cloak, sizeof(cloak));
    }

    D2D_RECT_F clip{0.0f, 0.0f, scaled_width, scaled_height};
    m_hostedVisual->SetClip(clip);

    m_dcompDevice->Commit();
}

} // namespace winrt::TestWinUIControls::implementation
