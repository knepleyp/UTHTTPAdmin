// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#define USE_MONGOOSE 0

#include "Core.h"
#include "UnrealTournament.h"

#if USE_MONGOOSE
#include "mongoose.h"
#else
#include "httpd.h"
#endif

#include "HTTPAdmin.generated.h"

UCLASS(Config=HTTPAdmin)
class UHTTPAdmin : public UObject, public FTickableGameObject
{
	GENERATED_UCLASS_BODY()

	void Init();

	// FTickableGameObject interface
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override
	{
		return true;
	}
	virtual bool IsTickableWhenPaused() const override
	{
		return true;
	}

#if USE_MONGOOSE
	static int StaticMGHandler(mg_connection* conn, enum mg_event ev);	
	int MGHandler(mg_connection* conn, enum mg_event ev);
#else
	static void StaticHTTPHandler(HttpResponse* Response, void* UserData);
	void HTTPHandler(HttpResponse* Response);
#endif

	FString PrepareAdminJSON();

	UPROPERTY(Config)
	bool bRequireAuth;

	UPROPERTY(Config)
	FString User;

	UPROPERTY(Config)
	FString Password;

	FString CombinedAuth;

	UPROPERTY(Config)
	int32 Port;

private:

#if USE_MONGOOSE
	mg_server* MGServer;
#else
	Httpd* HTTPServer;
#endif
};