// Copyright Type Dream Moon All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FDreamUMGModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
