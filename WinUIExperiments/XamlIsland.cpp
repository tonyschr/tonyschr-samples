#include "pch.h"
#include "XamlIsland.h"
#include "MainWindow.h"

HRESULT ReadFile(const std::wstring &path, std::wstring &content)
{
    std::wifstream file(path);
    if (!file.is_open())
    {
        return E_FAIL;
    }

    content.assign((std::istreambuf_iterator<wchar_t>(file)), std::istreambuf_iterator<wchar_t>());
    return S_OK;
}

winrt::Microsoft::UI::Xaml::UIElement LoadXaml(const std::wstring &path)
{
    std::wstring xaml_content;
    winrt::check_hresult(ReadFile(path, xaml_content));

    auto content = winrt::Microsoft::UI::Xaml::Markup::XamlReader::Load(xaml_content.c_str());

    return content.as<winrt::Microsoft::UI::Xaml::UIElement>();
}

// Load file from resource. Assumes resource is UTF-8.
winrt::Microsoft::UI::Xaml::UIElement LoadXaml(uint32_t id)
{
    auto resource = ::FindResource(nullptr, MAKEINTRESOURCE(id), MAKEINTRESOURCE(XAMLRESOURCE));
    winrt::check_bool(resource != NULL);

    HGLOBAL resourceHandle = ::LoadResource(nullptr, resource);
    winrt::check_bool(resourceHandle != NULL);

    auto data = static_cast<char *>(::LockResource(resourceHandle));

    DWORD size = SizeofResource(nullptr, resource);
    WCHAR *wideData = new WCHAR[size + 1];
    MultiByteToWideChar(CP_UTF8, 0, data, size, wideData, size + 1);
    wideData[size] = L'\0';

    auto content = winrt::Microsoft::UI::Xaml::Markup::XamlReader::Load(wideData);

    delete[] wideData;

    return content.as<winrt::Microsoft::UI::Xaml::UIElement>();
}

void XamlIsland::Initialize(MainWindow *MainWindow)
{
    m_desktopWindowXamlSource = winrt::Microsoft::UI::Xaml::Hosting::DesktopWindowXamlSource();
    m_desktopWindowXamlSourceNative = m_desktopWindowXamlSource.as<IDesktopWindowXamlSourceNative>();
    winrt::check_hresult(m_desktopWindowXamlSourceNative->AttachToWindow(MainWindow->GetWindow()));
}

bool XamlIsland::LoadFromFile(std::wstring const &fileName)
{
    try
    {
        m_rootElement = LoadXaml(fileName.c_str()).as<winrt::Microsoft::UI::Xaml::Controls::StackPanel>();
    }
    catch (winrt::hresult_error const &exception)
    {
        // Non-critical failure to enable easy hot reload of different XAML content
        // for quick experimentation.
        DebugPrint(L"Can't load XAML file: %s; %0xp\r\n", fileName.c_str(), exception.code());
        return false;
    }

    m_desktopWindowXamlSource.Content(m_rootElement);
    return true;
}

bool XamlIsland::LoadFromResource(uint32_t id)
{
    try
    {
        m_rootElement = LoadXaml(id).as<winrt::Microsoft::UI::Xaml::Controls::StackPanel>();
    }
    catch (winrt::hresult_error const &)
    {
        return false;
    }

    m_desktopWindowXamlSource.Content(m_rootElement);
    return true;
}

void XamlIsland::SetSize(RECT rect)
{
    if (m_desktopWindowXamlSourceNative)
    {
        SetWindowPos(GetIslandWindow(), nullptr, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
                     SWP_SHOWWINDOW);

        if (m_rootElement)
        {
            float scale_factor = 96 / static_cast<float>(GetDpiForSystem());
            float width = std::fmaxf((rect.right - rect.left) * scale_factor, 0.0f);
            float height = std::fmaxf((rect.bottom - rect.top) * scale_factor, 0.0f);
            m_rootElement.Width(width);
            m_rootElement.Height(height);
        }
    }
}

HWND XamlIsland::GetIslandWindow()
{
    HWND xamlIslandWindow = nullptr;
    winrt::check_hresult(m_desktopWindowXamlSourceNative->get_WindowHandle(&xamlIslandWindow));
    return xamlIslandWindow;
}

void XamlIsland::AddTestElement(winrt::Microsoft::UI::Xaml::UIElement element)
{
    winrt::Windows::Foundation::IInspectable parent = m_rootElement.FindName(winrt::hstring(L"testParent"));
    winrt::StackPanel panel = parent.as<winrt::StackPanel>();
    panel.Children().Append(element);
}
