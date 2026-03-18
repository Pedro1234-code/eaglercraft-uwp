
#pragma once

#include <winrt/Microsoft.Web.WebView2.Core.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.FileProperties.h>
#include <winrt/Windows.Storage.Streams.h>

using namespace winrt;
using namespace winrt::Windows::Data::Json;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Media;
using namespace winrt::Windows::UI::ViewManagement;
using namespace winrt::Windows::Storage;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::Web::WebView2::Core;

namespace winrt::UWPWebView2::implementation {
	struct MainPage;
}

class WV2Internals {
public:
	WV2Internals(winrt::UWPWebView2::implementation::MainPage&);

	void OnNavigationStarting(const WebView2&, const CoreWebView2NavigationStartingEventArgs&);
	void OnPermissionRequested(const CoreWebView2&, const CoreWebView2PermissionRequestedEventArgs&);
	void OnNewWindowRequested(const CoreWebView2&, const CoreWebView2NewWindowRequestedEventArgs&);
	void OnWindowCloseRequested(const CoreWebView2&, const IInspectable&);
	winrt::Windows::Foundation::IAsyncAction OnWebResourceRequested(CoreWebView2, CoreWebView2WebResourceRequestedEventArgs);

private:
	winrt::UWPWebView2::implementation::MainPage& mainPage;
};