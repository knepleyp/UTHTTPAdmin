// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Core.h"
#include "UnrealTournament.h"
#include "httpd.h"

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

	static void StaticHTTPHandler(HttpResponse* Response, void* UserData);

	void HTTPHandler(HttpResponse* Response);

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
	Httpd* HTTPServer;
};