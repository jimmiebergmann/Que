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

#include <Log.hpp>
#include <System/Timestamp.hpp>
#include <mutex>
#include <fstream>
#include <iostream>
#include <System/MemoryLeak.hpp>

// Static variables
static std::stringstream g_StringStream;
static std::ofstream g_FileStream;
static bool g_FileOpen = false;
static std::mutex g_Mutex;

// Static functions
static std::string GetNumberMin2(const unsigned int number);

namespace Que
{
	namespace Private
	{

		bool Log::Open(const std::string & p_LogFile, const bool p_AppendOld)
		{
			int mode = 0;
			if (p_AppendOld)
			{
				mode = std::fstream::app;
			}

			g_FileStream.open(p_LogFile.c_str(), mode);
			if (g_FileStream.is_open() == false)
			{
				return false;
			}

			g_FileOpen = true;
			return true;
		}

		void Log::Close()
		{
			if (g_FileStream.is_open())
			{
				g_FileStream.close();
			}
		}

		std::stringstream & Log::GetStream()
		{
			g_Mutex.lock();
			return g_StringStream;
		}

		void Log::Flush(const char * p_pType)
		{
			if (g_FileOpen)
			{
				Timestamp time = Timestamp::Now();
				g_FileStream << time.GetYear() << "-" << GetNumberMin2(time.GetMonth()) << "-" << GetNumberMin2(time.GetDay()) << " ";
				g_FileStream << GetNumberMin2(time.GetHour()) << ":" << GetNumberMin2(time.GetMinute()) << ":" << GetNumberMin2(time.GetSecond()) << " ";
				g_FileStream << "[" << p_pType << "] - " << g_StringStream.str() << std::endl;
				g_FileStream.flush();
			}

#ifdef QUE_BUILD_DEBUG
			std::cout << "[" << p_pType << "] - " << g_StringStream.str() << std::endl;
#endif

			g_StringStream.str("");
			g_Mutex.unlock();
		}

	}

}

std::string GetNumberMin2(const unsigned int number)
{
	if (number == 0)
	{
		return "00";
	}
	
	std::stringstream ss;
	if (number < 10)
	{
		ss << "0";
	}
	ss << number;

	return ss.str();;
}