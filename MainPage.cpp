#include "pch.h"
#include "MainPage.h"
#include "MainPage.g.cpp"
#include "Internals/ExtensionManager.h"

// Includes necessários para Gamepad e Teclado Virtual
#include <winrt/Windows.Gaming.Input.h>
#include <winrt/Windows.UI.ViewManagement.Core.h>

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Media;
using namespace winrt::Windows::UI::ViewManagement;
using namespace winrt::Windows::UI::ViewManagement::Core;
using namespace winrt::Windows::Gaming::Input;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Data::Json;

ExtensionManager* g_extensionManager = nullptr;

namespace winrt::UWPWebView2::implementation
{
    MainPage::MainPage()
        : internals(*this)
    {
        Loaded({ this, &MainPage::OnLoaded });

        ApplicationView::GetForCurrentView().SetDesiredBoundsMode(ApplicationViewBoundsMode::UseCoreWindow);

        if (!ApplicationViewScaling::TrySetDisableLayoutScaling(true))
        {
            OutputDebugString(L"Error: Failed to disable layout scaling.");
        }
    }

    const std::string& MainPage::GetAppBaseUrl() { return appBaseUrl; }
    const std::string& MainPage::GetWebResourceFolder() { return webResourceFolder; }
    WebView2& MainPage::GetWebView2() { return webView; }

    fire_and_forget MainPage::InitializeWebView()
    {
        webView.Background(SolidColorBrush(Windows::UI::ColorHelper::FromArgb(255, 0, 0, 0)));

        co_await webView.EnsureCoreWebView2Async();
        if (coreWV2 = webView.CoreWebView2())
        {
            auto settings = coreWV2.Settings();
            settings.IsZoomControlEnabled(false);
            settings.IsPinchZoomEnabled(false);

            webView.NavigationCompleted({ this, &MainPage::OnNavigationCompleted });
            webView.WebMessageReceived({ this, &MainPage::OnWebMessageReceived });

            webView.NavigationStarting({ &internals, &WV2Internals::OnNavigationStarting });
            coreWV2.PermissionRequested({ &internals, &WV2Internals::OnPermissionRequested });
            coreWV2.NewWindowRequested({ &internals, &WV2Internals::OnNewWindowRequested });
            coreWV2.WindowCloseRequested({ &internals, &WV2Internals::OnWindowCloseRequested });

            std::string requestFilter = appBaseUrl + "*";
            coreWV2.AddWebResourceRequestedFilter(winrt::to_hstring(requestFilter), CoreWebView2WebResourceContext::All);
            coreWV2.WebResourceRequested({ &internals, &WV2Internals::OnWebResourceRequested });

            webView.Source(Uri{ L"https://app.localhost/index.html" });
        }
    }

    void MainPage::OnLoaded(const IInspectable&, const RoutedEventArgs&)
    {
        std::string appFolder = winrt::to_string(Windows::ApplicationModel::Package::Current().InstalledLocation().Path());
        std::string userDataFolder = winrt::to_string(Windows::Storage::ApplicationData::Current().LocalFolder().Path());

        g_extensionManager = new ExtensionManager(appFolder, webResourceFolder, userDataFolder);
        g_extensionManager->SetMainPage(this);
        g_extensionManager->Init();

        webView = WebView2();
        InitializeWebView();

        // --- MONITORAMENTO DO GAMEPAD ---
        DispatcherTimer timer;
        timer.Interval(std::chrono::milliseconds(32)); // ~30fps é suficiente para detecção de botões
        timer.Tick([this](auto&&, auto&&) {
            auto gamepads = Gamepad::Gamepads();
            if (gamepads.Size() > 0)
            {
                auto reading = gamepads.GetAt(0).GetCurrentReading();

                // Bitwise check para Menu (Start) e View (Select/Back)
                bool menuPressed = (reading.Buttons & GamepadButtons::Menu) == GamepadButtons::Menu;
                bool viewPressed = (reading.Buttons & GamepadButtons::View) == GamepadButtons::View;

                static bool wasPressed = false;
                if (menuPressed && viewPressed)
                {
                    if (!wasPressed)
                    {
                        // Abre o teclado do Xbox
                        CoreInputView::GetForCurrentView().TryShow(CoreInputViewKind::Keyboard);
                        wasPressed = true;
                    }
                }
                else
                {
                    wasPressed = false;
                }
            }
        });
        timer.Start();
    }

    void MainPage::OnNavigationCompleted(const WebView2&, const CoreWebView2NavigationCompletedEventArgs& args)
    {
        if (!webView.Parent())
        {
            if (args.IsSuccess())
            {
                Content(webView);
                webView.Focus(FocusState::Programmatic);
            }
        }
    }

    void MainPage::OnWebMessageReceived(const WebView2&, const CoreWebView2WebMessageReceivedEventArgs& args)
    {
        hstring jsonStr{ args.TryGetWebMessageAsString() };
        JsonObject json{};
        if (!JsonObject::TryParse(jsonStr, json)) return;

        hstring type{ json.GetNamedString(L"type") };
        if (type == L"wrapper-init")
        {
            HandleRuntimeInitMessage();
        }
        else if (type == L"extension-message")
        {
            g_extensionManager->OnExtensionMessage(winrt::to_string(jsonStr));
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
        if (coreWV2) coreWV2.PostWebMessageAsJson(winrt::to_hstring(jsonStr));
    }
}