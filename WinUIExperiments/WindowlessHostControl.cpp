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

// Add an element to size the container and aid in hit testing.
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

    // Reparent and cloak. Uncomment to hide the original HWND while still using the visual.
    //if (m_xamlIslandWindow)
    //{         
    //    SetParent(m_hostedWindow, m_xamlIslandWindow);
    //    SetWindowLongPtr(m_hostedWindow, GWL_STYLE, WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
    //    BOOL cloak = TRUE;
    //    winrt::check_hresult(DwmSetWindowAttribute(m_hostedWindow, DWMWA_CLOAK, &cloak, sizeof(cloak)));
    //}
    
    winrt::com_ptr<IUnknown> retargetedWindowSurface;
    HRESULT hr = m_dcompDevice->CreateSurfaceFromHwnd(m_hostedWindow, retargetedWindowSurface.put());
    assert(SUCCEEDED(hr));

    hr = m_dcompDevice->CreateVisual(m_hostedVisual.put());
    assert(SUCCEEDED(hr));

    hr = m_hostedVisual->SetContent(retargetedWindowSurface.get());
    assert(SUCCEEDED(hr));
}

void WindowlessHostControl::SetHostedVisual()
{
    m_outputLink = winrt::Microsoft::UI::Content::ContentExternalOutputLink::Create(ElementCompositor());
    winrt::com_ptr<IDCompositionTarget> target = m_outputLink.as<IDCompositionTarget>();

    HRESULT hr = target->SetRoot(m_hostedVisual.get());
    assert(SUCCEEDED(hr));

    winrt::float2 size{400, 300};
    m_outputLink.PlacementVisual().Size(size);
    winrt::ElementCompositionPreview::SetElementChildVisual(*this, m_outputLink.PlacementVisual());

    // Hard-coding the size for now since during early init we don't seem to always
    // have an initial size yet in WinUI 3.
    SetHostedVisualSize(400, 301); // ISSUE: Why can't this be exactly 300 when window is cloaked?
}

void WindowlessHostControl::SetHostedVisualSize(float width, float height)
{
    float scaled_width = width * m_rasterizationScale;
    float scaled_height = height * m_rasterizationScale;

    if (m_hostedWindow && m_xamlIslandWindow)
    {
        winrt::GeneralTransform transform = TransformToVisual(nullptr);
        auto top_left = transform.TransformPoint(winrt::Windows::Foundation::Point(0, 0));
        SetWindowPos(m_hostedWindow, HWND_TOP, std::lroundf(top_left.X * m_rasterizationScale),
                     std::lroundf(top_left.Y * m_rasterizationScale), std::lroundf(scaled_width),
                     std::lroundf(scaled_height), SWP_SHOWWINDOW);
    }

    m_dcompDevice->Commit();
}

winrt::Compositor WindowlessHostControl::ElementCompositor()
{
    return winrt::ElementCompositionPreview::GetElementVisual(*this).Compositor();
}

} // namespace winrt::TestWinUIControls::implementation
