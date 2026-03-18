#include "pch.h"
#include "Internals.h"
#include "MainPage.h"

WV2Internals::WV2Internals(winrt::UWPWebView2::implementation::MainPage& mainPage_)
	: mainPage(mainPage_)
{
}

// Block navigations outside of the app scope
void WV2Internals::OnNavigationStarting(const WebView2&, const CoreWebView2NavigationStartingEventArgs& args)
{
	std::string uri{ winrt::to_string(args.Uri()) };
	const std::string& appBaseUrl = mainPage.GetAppBaseUrl();

	if (uri.substr(0, appBaseUrl.length()) != appBaseUrl)
	{
		args.Cancel(true);
	}
}

// Automatically allow all permission requests
void WV2Internals::OnPermissionRequested(const CoreWebView2&, const CoreWebView2PermissionRequestedEventArgs& args)
{
	args.State(CoreWebView2PermissionState::Allow);
}

// When opening a new window, launch the system browser instead
void WV2Internals::OnNewWindowRequested(const CoreWebView2&, const CoreWebView2NewWindowRequestedEventArgs& args)
{
	winrt::Windows::System::Launcher::LaunchUriAsync(Uri{ args.Uri() });
}

// Exit application via browser close method
void WV2Internals::OnWindowCloseRequested(const CoreWebView2&, const IInspectable&)
{
	Application::Current().Exit();
}

// Content serving
IAsyncAction WV2Internals::OnWebResourceRequested(CoreWebView2 coreWV2, CoreWebView2WebResourceRequestedEventArgs args)
{
	Deferral deferral = args.GetDeferral();

	auto request = args.Request();
	std::wstring uriW{ request.Uri() };

	const std::string& appBaseUrl = mainPage.GetAppBaseUrl();
	const std::string& webResourceFolder = mainPage.GetWebResourceFolder();
	std::string requestPath{ winrt::to_string(uriW.substr(appBaseUrl.length())) };

	int statusCode = 0;
	std::string statusPhrase;
	std::vector<std::string> responseHeaders;
	Streams::IRandomAccessStream responseStream = nullptr;
	std::string contentType = GetMimeTypeForFilename(requestPath);
	uint64_t contentLength = 0;
	Streams::DataWriter dataWriter = nullptr;

	// Skip caching on all responses as they are all local
	responseHeaders.push_back("Cache-Control: no-store");

	try {
		Uri fileUri{ winrt::to_hstring(std::string("ms-appx:///") + webResourceFolder + "/" + requestPath) };
		StorageFile localFile = co_await StorageFile::GetFileFromApplicationUriAsync(fileUri);
		responseStream = co_await localFile.OpenReadAsync();
		
		statusCode = 200;
		statusPhrase = "OK";
		contentLength = responseStream.Size();

		// Adding Accept-Ranges: bytes allows the Chromium engine to seek media elements, although it is inefficient
		// as it will still get full 200 OK responses instead of partial responses.
		responseHeaders.push_back("Accept-Ranges: bytes");
	}
	catch (const winrt::hresult_error& err)
	{
		responseStream = Streams::InMemoryRandomAccessStream();
		dataWriter = Streams::DataWriter(responseStream);

		if (err.code() == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			dataWriter.WriteString(winrt::to_hstring(std::string("<h1>404 Not Found</h1><p>Unable to locate a file at path: ") + requestPath));
			statusCode = 404;
			statusPhrase = "Not Found";
		}
		else
		{
			dataWriter.WriteString(L"<h1>500 Internal Server Error</h1><p>Oops, something went wrong.</p>");
			statusCode = 500;
			statusPhrase = "Internal Server Error";
		}
	}

	// Handle error response
	if (dataWriter != nullptr)
	{
		co_await dataWriter.StoreAsync();
		responseStream.Seek(0);

		contentLength = responseStream.Size();
		contentType = "text/html; charset=utf-8";
	}

	responseHeaders.push_back(std::string("Content-Type: ") + contentType);
	responseHeaders.push_back(std::string("Content-Length: ") + std::to_string(contentLength));

	CoreWebView2WebResourceResponse response = coreWV2.Environment().CreateWebResourceResponse(
		responseStream, statusCode, winrt::to_hstring(statusPhrase), winrt::to_hstring(JoinStrings(responseHeaders, "\n")));
	args.Response(response);

	deferral.Complete();
}