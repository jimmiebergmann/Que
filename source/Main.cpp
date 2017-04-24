// ///////////////////////////////////////////////////////////////////////////
// Copyright (C) 2017 Jimmie Bergmann - jimmiebergmann@gmail.com
//
// This software is provided 'as-is', without any express or
// implied warranty. In no event will the authors be held
// liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute
// it freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but
//    is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any
//    source distribution.
// ///////////////////////////////////////////////////////////////////////////

#include <Server.hpp>
#include <System/Sleep.hpp>
#include <iostream>
#include <System/MemoryLeak.hpp>

using namespace Que;

// Static variables
static int g_ArgCount = 0;
static Que::Address g_Address;
static unsigned short g_Port = 11400;
static std::string g_ServerRoot = "";
static std::string g_ServerLog = "";
static unsigned int g_MaxMessageSize = 65535;

// Static functions
static bool HandleArguments(int argc, char ** argv);
static bool EnoughArguments(int p_CurrentIndex, int p_Require);
static void PrintHelp();

// Argument list
// -l ADDR		- Listening address
// -p PORT		- Port
// -d DIR		- Root directory of message queue
// -o LOG		- Log file.
// -m SIZE		- Max size of message.
int main(int argc, char ** argv)
{
	// initialize the memory leak detector.
	QueInitMemoryLeak(NULL);

	// Handle argument.
	if (HandleArguments(argc, argv) == false)
	{
		return 0;
	}

	// Run Que
	Server que;
	Settings settings(g_Address, g_Port, g_ServerRoot, g_ServerLog, g_MaxMessageSize);
	que.Run(settings);

#ifdef QUE_BUILD_DEBUG
	// Run until termination flag is set.
	while (que.IsRunning())
	{
		//Sleep(Time::Seconds(0.1f));
		int input = std::cin.get();
		if (input == 'q')
		{
			que.Stop();
		}

	}
#elif
	while (que.IsRunning())
	{
#error No DEBUG main loop.
		//Sleep();
	}
#endif

	return 0;
}


bool HandleArguments(int argc, char ** argv)
{
	// Set global arg counter.
	g_ArgCount = argc;

	for (int i = 1; i < argc; i++)
	{

		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			PrintHelp();
			return false;
		}
		if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "ADDR") == 0)
		{
			if (!EnoughArguments(i, 1))
			{
				std::cout << "Missing argument value: -l ADDR";
				return false;
			}

			std::string address;
			address.assign(argv[++i]);

			g_Address = Que::Address(0);
			if (address.size() != 0 && address != "0" && address != "0.0.0.0")
			{
				if (g_Address.SetAddressFromString(address) == false)
				{
					std::cout << "Invalid argument: Listen address: " << address << std::endl;
					return false;
				}
			}
			continue;
		}
		else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "PORT") == 0)
		{
			if (!EnoughArguments(i, 1))
			{
				std::cout << "Missing argument value: -p PORT";
				return false;
			}

			std::string port;
			port.assign(argv[++i]);
	
			int portNum = atoi(port.c_str());
			if (portNum < 0 || portNum > 65535)
			{
				std::cout << "Invalid argument: Listen port: " << port << std::endl;
				return 0;
			}
			g_Port = static_cast<unsigned short>(portNum);

			continue;
		}
		else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "DIR") == 0)
		{
			if (!EnoughArguments(i, 1))
			{
				std::cout << "Missing argument value: -d DIR";
				return false;
			}

			g_ServerRoot.assign(argv[++i]);
			continue;
		}

		else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "LOG") == 0)
		{
			if (!EnoughArguments(i, 1))
			{
				std::cout << "Missing argument value: -o LOG";
				return false;
			}

			g_ServerLog.assign(argv[++i]);
			continue;
		}
		else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "SIZE") == 0)
		{
			if (!EnoughArguments(i, 1))
			{
				std::cout << "Missing argument value: -m SIZE";
				return false;
			}

			std::string size;
			size.assign(argv[++i]);

			int sizeNum = atoi(size.c_str());
			if (sizeNum < 0 || sizeNum >= 1000000000)
			{
				std::cout << "Invalid argument: Maximum message size: " << size << std::endl;
				return 0;
			}
			g_MaxMessageSize = static_cast<unsigned int>(sizeNum);

			continue;
		}
	}
	
	return true;
}

bool EnoughArguments(int p_CurrentIndex, int p_Require)
{
	if (p_CurrentIndex + p_Require >= g_ArgCount)
	{
		return false;
	}

	return true;
}

void PrintHelp()
{
	std::cout << "Use: que [OPTIONS]" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << " -l ADDR    Listen on address(default is 0.0.0.0)" << std::endl;
	std::cout << " -p PORT    Listen on port(default is 11400)" << std::endl;
	std::cout << " -d DIR     Direcotry of queue folder" << std::endl;
	std::cout << " -o LOG     Log path(Default is program location)" << std::endl;
	std::cout << " -m SIZE    Maximum size of message.(Default is 65545)" << std::endl;
	std::cout << " -h --help  Show this help" << std::endl;
}