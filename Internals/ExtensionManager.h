#pragma once

class ConstructWebView;

extern class ExtensionManager* g_extensionManager;

typedef void (*SendWebMessage_t)(LPCSTR jsonStr);

typedef LPCSTR* (*DLL_Init_t)(LPCSTR appFolder, LPCSTR webResourceFolder, LPCSTR userDataFolder, SendWebMessage_t sendWebMessageFunc);
typedef void (*DLL_Release_t)();
typedef void (*DLL_OnExtensionMessage_t)(LPCSTR jsonStr);

class ExtensionManager {
public:
	ExtensionManager(const std::string& appFolder_, const std::string& webResourceFolder_, const std::string& userDataFolder_);

	bool Init();
	void SetMainPage(winrt::UWPWebView2::implementation::MainPage* mainPage_);
	void Release();

	void OnExtensionMessage(const std::string& jsonStr);

	void SendWebMessage(const std::string& jsonStr);

	const std::vector<std::string>& GetAllRegisteredComponentIds();

protected:
	HMODULE hModule;
	
	DLL_Init_t fInit;
	DLL_Release_t fRelease;
	DLL_OnExtensionMessage_t fOnExtensionMessage;

	std::string appFolder;
	std::string webResourceFolder;
	std::string userDataFolder;
	winrt::UWPWebView2::implementation::MainPage* mainPage;

	std::vector<std::string> allComponentIds;
};
