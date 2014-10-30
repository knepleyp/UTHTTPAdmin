// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "HTTPAdmin.h"

#include "Core.h"
#include "Engine.h"
#include "ModuleManager.h"
#include "ModuleInterface.h"

class FHTTPAdminPlugin : public IModuleInterface
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FHTTPAdminPlugin, HTTPAdmin)

void FHTTPAdminPlugin::StartupModule()
{
	FHTTPAdmin* HTTPAdmin = new FHTTPAdmin;
	HTTPAdmin->Init();
}


void FHTTPAdminPlugin::ShutdownModule()
{

}



