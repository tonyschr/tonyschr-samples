# WinUIExperiments

This repro contains a test/demo application for asking questions and sharing bug repros.

### Prerequsites
* 1.2.220727.1-experimental1 Runtime, as this is an unpackaged app. Available from https://learn.microsoft.com/en-us/windows/apps/windows-app-sdk/downloads
* The other dependencies should get resolved by NuGet from the packages.config

Once built, use the menu to launch the test scenarios.

## Application Details

This is a Win32 desktop application using WinUI 3 to host a XAML Island, where the current main
scenario is creation of a custom XAML control to host other "legacy" Win32 content. Unlike this
trivial example, that code can't be transitioned to XAML / WinUI. But it's important to be able
to seamlessly compose it, apply transitions, masks, etc. like any other XAML element.

The intent is to use CreateSurfaceFromHwnd along with IDComposition* APIs to host the Win32 
content within the XAML tree. _Currently_ only the output (display) side is considered.
