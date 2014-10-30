// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Core.h"
#include "UnrealTournament.h"
#include "httpd.h"

class FHTTPAdmin : public FTickableGameObject
{
public:
	FHTTPAdmin()
	{
		HTTPServer = nullptr;
	}

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

private:
	Httpd* HTTPServer;
};