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
#include <mutex>
#include <iostream>

// Static variables
static std::mutex			g_Mutex;
static std::stringstream	g_Stream;

namespace Que
{
	namespace Private
	{

		void Log::Flush(const std::string & p_Type)
		{
			std::cout << "[" << p_Type << "] " << g_Stream.str() << std::endl;
			g_Stream.str("");

			g_Mutex.unlock();
		}

		std::stringstream & Log::GetStream()
		{
			g_Mutex.lock();
			return g_Stream;
		}

	}

}
