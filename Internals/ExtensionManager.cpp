
#include "pch.h"
#include "../MainPage.h"
#include "ExtensionManager.h"

void SendWebMessageFunc(LPCSTR jsonStr)
{
	if (g_extensionManager != nullptr)
		g_extensionManager->SendWebMessage(jsonStr);
}

ExtensionManager::ExtensionManager(const std::string& appFolder_, const std::string& webResourceFolder_, const std::string& userDataFolder_)
	: hModule(nullptr),
	  fInit(nullptr),
	  fRelease(nullptr),
	  fOnExtensionMessage(nullptr),
	  appFolder(appFolder_),
	  webResourceFolder(webResourceFolder_),
	  userDataFolder(userDataFolder_),
	  mainPage(nullptr)
{
}

bool ExtensionManager::Init()
{
	hModule = LoadPackagedLibrary(L"WV2ExtManager.uwp.dll", 0);

	if (hModule == NULL)
		return false;
	
	fInit = (DLL_Init_t)GetProcAddress(hModule, "Init");
	fRelease = (DLL_Release_t)GetProcAddress(hModule, "Release");
	fOnExtensionMessage = (DLL_OnExtensionMessage_t)GetProcAddress(hModule, "OnExtensionMessage");

	if (fInit == nullptr || fRelease == nullptr || fOnExtensionMessage == nullptr)
	{
		return false;
	}

	LPCSTR* componentIds = fInit(appFolder.c_str(), webResourceFolder.c_str(), userDataFolder.c_str(), SendWebMessageFunc);

	if (componentIds == nullptr)
		return false;

	LPCSTR* c = componentIds;
	while (*c != nullptr)
	{
		allComponentIds.push_back(*c);
		c++;
	}

	return true;
}

void ExtensionManager::SetMainPage(winrt::UWPWebView2::implementation::MainPage* mainPage_)
{
	mainPage = mainPage_;
}

void ExtensionManager::Release()
{
	if (fRelease != nullptr)
	{
		fRelease();
	}

	if (hModule != NULL)
	{
		FreeLibrary(hModule);
		hModule = NULL;
	}
}

const std::vector<std::string>& ExtensionManager:: GetAllRegisteredComponentIds()
{
	return allComponentIds;
}

void ExtensionManager::OnExtensionMessage(const std::string& jsonStr)
{
	if (fOnExtensionMessage != nullptr)
		fOnExtensionMessage(jsonStr.c_str());
}

void ExtensionManager::SendWebMessage(const std::string& jsonStr)
{
	if (mainPage != nullptr)
		mainPage->SendWebMessage(jsonStr);
}
