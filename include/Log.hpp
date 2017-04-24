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

#include <Build.hpp>

#ifndef QUE_LOG_HPP
#define QUE_LOG_HPP

#include <sstream>

#define QueLogInfo(message) Que::Private::Log::GetStream()<<message;Que::Private::Log::Flush("Info");
#define QueLogError(message) Que::Private::Log::GetStream()<<message;Que::Private::Log::Flush("Error");
#define QueLogWarning(message) Que::Private::Log::GetStream()<<message;Que::Private::Log::Flush("Warning");

#ifdef QUE_BUILD_DEBUG
#define QueLogDebug(message) Que::Private::Log::GetStream()<<message;Que::Private::Log::Flush("Debug");
#else
#define QueLogDebug sizeof
#endif


namespace Que
{

	namespace Private
	{

		class Log
		{

		public:

			static bool Open(const std::string & p_LogFile, const bool p_AppendOld);

			static void Close();

			static std::stringstream & GetStream();

			static void Flush(const char * p_pType );

		};

	}

}

#endif