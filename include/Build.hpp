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

#ifndef QUE_BUILD_HPP
#define QUE_BUILD_HPP

#include <cstring>

////////////////////////////////////////////////////////////////
// Define the platform
////////////////////////////////////////////////////////////////
#if defined( _WIN32 ) || defined( __WIN32__ ) || defined( _WIN64 ) || defined( __WIN64__ )
#define QUE_PLATFORM_WINDOWS
/*#elif defined( linux ) || defined( __linux )
#define BIT_PLATFORM_LINUX*/
#else
#error No platform is defined
#endif

////////////////////////////////////////////////////////////////
// Define the build type ( release / debug )
////////////////////////////////////////////////////////////////
#if defined( NDEBUG ) || !defined( _DEBUG )
#define QUE_BUILD_RELEASE
#else
#define QUE_BUILD_DEBUG
#endif

namespace Que
{

	typedef long long			int64;		///< Unsigned 64 bit integer type
	typedef unsigned long long	uint64;		///< Unsigned 64 bit integer type

}

#endif


