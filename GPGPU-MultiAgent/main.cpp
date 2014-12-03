////////////////////////////////////////////////////////////////////////////////
// Filename: main.cpp
////////////////////////////////////////////////////////////////////////////////
#include "systemclass.h"
#include "LoggerCpp/LoggerCpp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
#ifndef NDEBUG
	Log::Manager::setDefaultLevel(Log::Log::eDebug);
#else
	Log::Manager::setDefaultLevel(Log::Log::eNotice);
#endif

	Log::Config::Vector configList;
	//Log::Config::addOutput(configList, "OutputConsole");
	Log::Config::addOutput(configList, "OutputFile");
	Log::Config::setOption(configList, "filename", "log.txt");
	Log::Config::setOption(configList, "filename_old", "log.old.txt");
	Log::Config::setOption(configList, "max_startup_size", "0");
	Log::Config::setOption(configList, "max_size", "1000000000");
#ifdef WIN32
	Log::Config::addOutput(configList, "OutputDebug");
#endif

	Log::Manager::configure(configList);

	SystemClass* System;
	bool result;

	// Create the system object.
	System = new SystemClass;
	if(!System)
	{
		return 0;
	}

	// Initialize and run the system object.
	result = System->Initialize();
	if(result)
	{
		System->Run();
	}

	// Shutdown and release the system object.
	System->Shutdown();
	delete System;
	System = 0;

	return 0;
}