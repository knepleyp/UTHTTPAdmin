// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#include "HTTPAdmin.h"
#include "Base64.h"

UHTTPAdmin::UHTTPAdmin(const FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
#if USE_MONGOOSE
	MGServer = nullptr;
#else
	HTTPServer = nullptr;
#endif
}

void UHTTPAdmin::Init()
{
	// Don't garbage collect me
	SetFlags(RF_RootSet);

	CombinedAuth = User + TEXT(":") + Password;

	if (Port == 0)
	{
		Port = 8080;
	}

#if USE_MONGOOSE
	MGServer = mg_create_server(this, StaticMGHandler);
	
	FString PortStr = FString::FromInt(Port);
	mg_set_option(MGServer, "listening_port", TCHAR_TO_ANSI(*PortStr));

	FString DocumentRoot = FPaths::GamePluginsDir() / TEXT("HTTPAdmin") / TEXT("HTML");
	mg_set_option(MGServer, "document_root", TCHAR_TO_ANSI(*DocumentRoot));
#else
	HTTPServer = httpd_create(Port, StaticHTTPHandler, this);
#endif
}

void UHTTPAdmin::Tick(float DeltaTime)
{
#if USE_MONGOOSE
	if (MGServer != nullptr)
	{
		mg_poll_server(MGServer, 1);
	}
#else
	if (HTTPServer != nullptr)
	{
		httpd_process(HTTPServer, false);
	}
#endif
}

TStatId UHTTPAdmin::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UHTTPAdmin, STATGROUP_Tickables);
}

#if USE_MONGOOSE
int UHTTPAdmin::StaticMGHandler(mg_connection* conn, enum mg_event ev)
{
	return ((UHTTPAdmin*)conn->server_param)->MGHandler(conn, ev);
}

int UHTTPAdmin::MGHandler(mg_connection* conn, enum mg_event ev)
{
	if (ev == MG_AUTH)
	{
		return MG_TRUE;
	}
	else if (ev == MG_REQUEST)
	{
		char argumentbuffer[256];
		argumentbuffer[255] = 0;
		if (mg_get_var(conn, "consolecommand", argumentbuffer, 255) > 0)
		{
			FString ConsoleCommandString(argumentbuffer);
			GEngine->Exec(GWorld, *ConsoleCommandString);
		}

		FString URL(conn->uri);

		const FString FileExtension = FPaths::GetExtension(URL);
		if (FileExtension == FString(TEXT("json")))
		{
			FString JSON = PrepareAdminJSON();

			mg_send_header(conn, "Content-Type", "application/json");
			mg_send_data(conn, TCHAR_TO_ANSI(*JSON), JSON.Len());
			return MG_TRUE;
		}
	}

	return MG_FALSE;
}
#else
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
		Location = FString(TEXT("/index.html"));
	}

	FString FilePath = FPaths::GamePluginsDir() / TEXT("HTTPAdmin") / TEXT("HTML") + *Location;
	const FString FileExtension = FPaths::GetExtension(Location);
	if (FileExtension == TEXT("json"))
	{
		FString JSON = PrepareAdminJSON();

		// Want to use this method so no-cache is set
		httpresponse_begin(Response, 200, "Content-Type: application/json\r\n");
		httpresponse_writef(Response, TCHAR_TO_ANSI(*JSON));
		httpresponse_end(Response);
	}
	else if (FileExtension == TEXT("html") || FileExtension == TEXT("htm"))
	{
		FString FileData;
		FFileHelper::LoadFileToString(FileData, *FilePath);

		httpresponse_response(Response, 200, TCHAR_TO_ANSI(*FileData), 0, "Content-Type: text/html\r\n");
	}
	else if (FileExtension == TEXT("js"))
	{
		FString FileData;
		FFileHelper::LoadFileToString(FileData, *FilePath);

		httpresponse_response(Response, 200, TCHAR_TO_ANSI(*FileData), 0, "Content-Type: application/javascript\r\n");
	}
	else if (FileExtension == TEXT("jpg"))
	{
		TArray<uint8> FileData;
		FFileHelper::LoadFileToArray(FileData, *FilePath);

		httpresponse_response(Response, 200, (const char*)FileData.GetData(), FileData.Num(), "Content-Type: image/jpeg\r\n");
	}
	else if (FileExtension == TEXT("png"))
	{
		TArray<uint8> FileData;
		FFileHelper::LoadFileToArray(FileData, *FilePath);

		httpresponse_response(Response, 200, (const char*)FileData.GetData(), FileData.Num(), "Content-Type: image/png\r\n");
	}
	else
	{
		httpresponse_response(Response, 404, "Not Found", 0, nullptr);
	}
}

#endif

FString UHTTPAdmin::PrepareAdminJSON()
{
	AUTGameMode* GameMode = Cast<AUTGameMode>(GWorld->GetAuthGameMode());
	
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

	return JSON;
}
