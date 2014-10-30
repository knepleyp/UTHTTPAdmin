// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#include "HTTPAdmin.h"

void FHTTPAdmin::Init()
{
	HTTPServer = httpd_create(8080, StaticHTTPHandler, this);
}

void FHTTPAdmin::Tick(float DeltaTime)
{
	if (HTTPServer != nullptr)
	{
		httpd_process(HTTPServer, false);
	}
}

TStatId FHTTPAdmin::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FHTTPAdmin, STATGROUP_Tickables);
}

void FHTTPAdmin::StaticHTTPHandler(HttpResponse* Response, void* UserData)
{
	FHTTPAdmin* HTTPAdmin = static_cast<FHTTPAdmin*>(UserData);

	HTTPAdmin->HTTPHandler(Response);
}

void FHTTPAdmin::HTTPHandler(HttpResponse* Response)
{
	FString Location(httpresponse_location(Response));

	FString Output = Location + "<br />Hello, world!";
	httpresponse_response(Response, 220, TCHAR_TO_ANSI(*Output), 0, 0);
}