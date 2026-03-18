#include "pch.h"
#include "MainPage.h"
#include "MainPage.g.cpp"
#include "Internals/ExtensionManager.h"

ExtensionManager* g_extensionManager = nullptr;

namespace winrt::UWPWebView2::implementation
{
	MainPage::MainPage()
		: internals(*this)
	{
		Loaded({ this, &MainPage::OnLoaded });

		// By default, Xbox gives you a border around your content to help you keep it inside a "TV-safe"
		// area. This helps protect you from drawing too close to the edges of the screen where content may
		// not be visible due to physical variations in televisions.
		// This line disables that behavior. If you want, you can restore the automatic TV-safe area by
		// commenting this line out. Otherwise, be careful not to draw vital content too close to the edge
		// of the screen. Details can be found here:
		// https://docs.microsoft.com/en-us/windows/apps/design/devices/designing-for-tv#tv-safe-area
		ApplicationView::GetForCurrentView().SetDesiredBoundsMode(ApplicationViewBoundsMode::UseCoreWindow);

		// By default, XAML apps are scaled up 2x on Xbox. This line disables that behavior, allowing the
		// app to use the actual resolution of the device (1920 x 1080 pixels).
		if (!ApplicationViewScaling::TrySetDisableLayoutScaling(true))
		{
			OutputDebugString(L"Error: Failed to disable layout scaling.");
		}
	}

	const std::string& MainPage::GetAppBaseUrl()
	{
		return appBaseUrl;
	}

	const std::string& MainPage::GetWebResourceFolder()
	{
		return webResourceFolder;
	}

	WebView2& MainPage::GetWebView2()
	{
		return webView;
	}

	/// <summary>
	/// Sets up the web view, hooking up all of the event handlers that the app cares about.
	/// </summary>
	fire_and_forget MainPage::InitializeWebView()
	{
		// The WebView's background color can sometimes draw while a page is loading. Set it to
		// something that matches the app's color scheme so it does not produce a jarring flash.
		webView.Background(SolidColorBrush(Windows::UI::ColorHelper::FromArgb(255, 0, 0, 0)));

		co_await webView.EnsureCoreWebView2Async();
		if (coreWV2 = webView.CoreWebView2())
		{
			auto settings = coreWV2.Settings();
			settings.IsZoomControlEnabled(false);
			settings.IsPinchZoomEnabled(false);
			
			// Set up event handlers
			webView.NavigationCompleted({ this, &MainPage::OnNavigationCompleted });
			webView.WebMessageReceived({ this, &MainPage::OnWebMessageReceived });

			webView.NavigationStarting({ &internals, &WV2Internals::OnNavigationStarting });
			coreWV2.PermissionRequested({ &internals, &WV2Internals::OnPermissionRequested });
			coreWV2.NewWindowRequested({ &internals, &WV2Internals::OnNewWindowRequested });
			coreWV2.WindowCloseRequested({ &internals, &WV2Internals::OnWindowCloseRequested });

			// Handling content requests: ideally this could be handled entirely with SetVirtualHostNameToFolderMapping().
			// However this has a bug that prevents audio/video seeks working (see https://github.com/MicrosoftEdge/WebView2Feedback/issues/1386).
			// Therefore instead handle the WebResourceRequested event manually so requests can be customized to work with seeking.
			
			//coreWV2.SetVirtualHostNameToFolderMapping(L"app.localhost", L"www", CoreWebView2HostResourceAccessKind::Allow);

			std::string requestFilter = appBaseUrl + "*";
			coreWV2.AddWebResourceRequestedFilter(winrt::to_hstring(requestFilter), CoreWebView2WebResourceContext::All);
			coreWV2.WebResourceRequested({ &internals, &WV2Internals::OnWebResourceRequested });

			// Navigate to our initial page
			webView.Source(Uri{ L"https://app.localhost/index.html" });
		}
		else
		{
			OutputDebugString(L"Unable to retrieve CoreWebView2");
		}
	}

	/// <summary>
	/// Called when the XAML page has finished loading.
	/// We wait to initialize the WebView until this point because it can take a little time to load.
	/// </summary>
	void MainPage::OnLoaded(const IInspectable&, const RoutedEventArgs&)
	{
		std::string appFolder = winrt::to_string(Windows::ApplicationModel::Package::Current().InstalledLocation().Path());
		std::string userDataFolder = winrt::to_string(ApplicationData::Current().LocalFolder().Path());
		EnsureTrailingSlash(appFolder);
		EnsureTrailingSlash(userDataFolder);
		g_extensionManager = new ExtensionManager(appFolder, webResourceFolder, userDataFolder);
		g_extensionManager->SetMainPage(this);
		g_extensionManager->Init();

		// Create the WebView and point it at the initial URL to begin loading the app's UI.
		// The WebView is not added to the XAML page until the NavigationCompleted event fires.
		webView = WebView2();
		InitializeWebView();
	}

	/// <summary>
	/// Called whenever a new page is fully loaded (or fails to load) in the WebView.
	/// </summary>
	/// <param name="args">Details about the page which was loaded.</param>
	void MainPage::OnNavigationCompleted(const WebView2&, const CoreWebView2NavigationCompletedEventArgs& args)
	{
		// If we haven't done so yet, add the WebView to the page, making it visible to the user.
		// We create it dynamically and wait for the first navigation to complete before doing so.
		// This ensures that focus gets set on the WebView only after it is ready to receive it.
		// Failing to do this can occasionally result in Gamepad input failing to route to your
		// JavaScript code.
		if (!webView.Parent())
		{
			if (args.IsSuccess())
			{
				// Replace the current contents of this page with the WebView
				Content(webView);
				webView.Focus(FocusState::Programmatic);
			}
			else
			{
				// WebView navigation failed.
				hstring errStr = L"Initial WebView navigation failed with error status: " + to_hstring(static_cast<int>(args.WebErrorStatus()));
				OutputDebugString(errStr.c_str());
			}
		}
	}

	
	void MainPage::OnWebMessageReceived(const WebView2&, const CoreWebView2WebMessageReceivedEventArgs& args)
	{
		hstring jsonStr{ args.TryGetWebMessageAsString() };
		JsonObject json{};
		if (!JsonObject::TryParse(jsonStr, json))
			return;
		
		hstring type{ json.GetNamedString(L"type") };
		if (type == L"wrapper-init")
		{
			HandleRuntimeInitMessage();
		}
		else if (type == L"extension-message")
		{
			g_extensionManager->OnExtensionMessage(winrt::to_string(jsonStr));
		}
		else
		{
			// invalid message type
		}
	}

	void MainPage::HandleRuntimeInitMessage()
	{
		const std::vector<std::string>& allRegisteredComponentIds = g_extensionManager->GetAllRegisteredComponentIds();
		
		JsonArray idArr{};
		for (const std::string& id : allRegisteredComponentIds)
		{
			idArr.Append(JsonValue::CreateStringValue(winrt::to_hstring(id)));
		}

		JsonObject sendJson{};
		sendJson.SetNamedValue(L"type", JsonValue::CreateStringValue(L"wrapper-init-response"));
		sendJson.SetNamedValue(L"registeredComponentIds", idArr);

		SendWebMessage(winrt::to_string(sendJson.Stringify()));
	}

	void MainPage::SendWebMessage(const std::string& jsonStr)
	{
		coreWV2.PostWebMessageAsJson(winrt::to_hstring(jsonStr));
	}
}
