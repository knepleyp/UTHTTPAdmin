// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#include "HTTPAdmin.h"
#include "Base64.h"

UHTTPAdmin::UHTTPAdmin(const FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	HTTPServer = nullptr;
}

void UHTTPAdmin::Init()
{
	// Don't garbage collect me
	SetFlags(RF_RootSet);

	CombinedAuth = User + TEXT(":") + Password;

	HTTPServer = httpd_create(Port, StaticHTTPHandler, this);
}

void UHTTPAdmin::Tick(float DeltaTime)
{
	if (HTTPServer != nullptr)
	{
		httpd_process(HTTPServer, false);
	}
}

TStatId UHTTPAdmin::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UHTTPAdmin, STATGROUP_Tickables);
}

void UHTTPAdmin::StaticHTTPHandler(HttpResponse* Response, void* UserData)
{
	UHTTPAdmin* HTTPAdmin = static_cast<UHTTPAdmin*>(UserData);

	HTTPAdmin->HTTPHandler(Response);
}

void UHTTPAdmin::HTTPHandler(HttpResponse* Response)
{
	bool bAuthorized = false;

	FString Base64Authorization = (httpresponse_get_header(Response, "Authorization"));
	if (!Base64Authorization.IsEmpty())
	{
		FString Authorization;
		Base64Authorization.RemoveFromStart(TEXT("Basic "));
		FBase64::Decode(Base64Authorization, Authorization);
		if (Authorization == CombinedAuth)
		{
			bAuthorized = true;
		}
	}

	if (!bAuthorized && bRequireAuth)
	{
		httpresponse_response(Response, 401, 0, 0, "WWW-Authenticate: Basic realm=\"UT\"");
		return;
	}

	FString Location(httpresponse_location(Response));

	const char* ConsoleCommand = httpresponse_get_arg(Response, "consolecommand");
	if (ConsoleCommand != nullptr)
	{
		FString ConsoleCommandString(ConsoleCommand);
		GEngine->Exec(GWorld, *ConsoleCommandString);
	}
	
	const char* KickRequest = httpresponse_get_arg(Response, "kick");
	if (KickRequest != nullptr)
	{
		// kick here
	}

	const char* BanRequest = httpresponse_get_arg(Response, "ban");
	if (BanRequest != nullptr)
	{
		// ban here
	}

	if (Location == FString("/"))
	{
		Location = FString(TEXT("/admin.html"));
	}

	FString FilePath = FPaths::GamePluginsDir() / TEXT("HTTPAdmin") / TEXT("HTML") + *Location;
	const FString FileExtension = FPaths::GetExtension(Location);
	if (FileExtension == TEXT("json"))
	{
		PrepareAdminJSON(Response);
	}
	else if (FileExtension == TEXT("html") || FileExtension == TEXT("htm"))
	{
		FString FileData;
		FFileHelper::LoadFileToString(FileData, *FilePath);

		httpresponse_response(Response, 220, TCHAR_TO_ANSI(*FileData), 0, "Content-Type: text/html\r\n");
	}
	else if (FileExtension == TEXT("js"))
	{
		FString FileData;
		FFileHelper::LoadFileToString(FileData, *FilePath);

		httpresponse_response(Response, 220, TCHAR_TO_ANSI(*FileData), 0, "Content-Type: application/javascript\r\n");
	}
	else if (FileExtension == TEXT("jpg"))
	{
		TArray<uint8> FileData;
		FFileHelper::LoadFileToArray(FileData, *FilePath);

		httpresponse_response(Response, 220, (const char*)FileData.GetData(), FileData.Num(), "Content-Type: image/jpeg\r\n");
	}
	else if (FileExtension == TEXT("png"))
	{
		TArray<uint8> FileData;
		FFileHelper::LoadFileToArray(FileData, *FilePath);

		httpresponse_response(Response, 220, (const char*)FileData.GetData(), FileData.Num(), "Content-Type: image/png\r\n");
	}
	else
	{
		httpresponse_response(Response, 404, "Not Found", 0, nullptr);
	}
}

void UHTTPAdmin::PrepareAdminJSON(HttpResponse* Response)
{
	AUTGameMode* GameMode = Cast<AUTGameMode>(GWorld->GetAuthGameMode());

	httpresponse_begin(Response, 220, "Content-Type: application/json\r\n");

	FString JSON = TEXT("{");

	if (GameMode != nullptr && GameMode->GameState != nullptr)
	{
		JSON += TEXT("\"mapname\":\"") + GWorld->GetMapName() + TEXT("\",");

		JSON += TEXT("\"maprotation\":[");
		for (int i = 0; i < GameMode->MapRotation.Num(); i++)
		{
			if (i != 0)
			{
				JSON += TEXT(",");
			}
			JSON += TEXT("\"") + GameMode->MapRotation[i] + TEXT("\"");
		}
		JSON += TEXT("],");


		JSON += TEXT("\"timeremaining\":") + FString::FromInt(GameMode->UTGameState->RemainingTime) + TEXT(",");

		JSON += TEXT("\"goalscore\":") + FString::FromInt(GameMode->UTGameState->GoalScore) + TEXT(",");

		JSON += TEXT("\"Players\":[");
		for (int i = 0; i < GameMode->GameState->PlayerArray.Num(); i++)
		{
			if (i != 0)
			{
				JSON += TEXT(",");
			}
			AUTPlayerState *PS = Cast<AUTPlayerState>(GameMode->GameState->PlayerArray[i]);
			if (PS != nullptr)
			{
				JSON += TEXT("{\"name\":\"") + PS->PlayerName + TEXT("\",\"score\":") + FString::FromInt(PS->Score) + TEXT(",\"kills\":") + FString::FromInt(PS->Kills) + +TEXT(",\"deaths\":") + FString::FromInt(PS->Deaths) + TEXT("}");
			}
		}
		JSON += TEXT("]");
	}

	JSON += TEXT("}");

	httpresponse_writef(Response, TCHAR_TO_ANSI(*JSON));
	httpresponse_end(Response);
}
