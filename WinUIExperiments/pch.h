#pragma once

#include <windows.h>
#include <strsafe.h>

#include <d2d1_1.h>
#include <d3d11_1.h>
#include <dcomp.h>
#include <dwmapi.h>

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

#include <winrt/base.h>

#include <winrt/Microsoft.UI.Composition.h>
#include <winrt/Microsoft.UI.Input.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Hosting.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Microsoft.UI.Xaml.Markup.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>

#include <Microsoft.UI.Xaml.Hosting.DesktopWindowXamlSource.h>

#include <winrt/Windows.UI.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.System.h>

#include <algorithm>
#include <fstream>

#include "resource.h"

void DebugPrint(const PCWSTR message, ...);

extern HINSTANCE g_instance;