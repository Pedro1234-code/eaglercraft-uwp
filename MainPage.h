#pragma once

#include "MainPage.g.h"
#include "Internals/Internals.h"

namespace winrt::UWPWebView2::implementation
{
	struct MainPage : MainPageT<MainPage>
	{
		MainPage();

		const std::string& GetAppBaseUrl();
		const std::string& GetWebResourceFolder();
		WebView2& GetWebView2();

		void SendWebMessage(const std::string& jsonStr);

	private:
		WebView2 webView = nullptr;
		CoreWebView2 coreWV2 = nullptr;
		WV2Internals internals;

		void OnLoaded(const Windows::Foundation::IInspectable&, const Windows::UI::Xaml::RoutedEventArgs&);
		fire_and_forget InitializeWebView();

		void OnNavigationCompleted(const WebView2&, const CoreWebView2NavigationCompletedEventArgs&);
		void OnWebMessageReceived(const WebView2&, const CoreWebView2WebMessageReceivedEventArgs&);

		void HandleRuntimeInitMessage();

		std::string appBaseUrl = "https://app.localhost/";
		std::string webResourceFolder = "www";
	};
}

namespace winrt::UWPWebView2::factory_implementation
{
	struct MainPage : MainPageT<MainPage, implementation::MainPage>
	{
	};
}
