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

#include <Settings.hpp>

namespace Que
{

	Settings::Settings(	const Address		p_ListenAddress,
						const unsigned int	p_ListenPort,
						const std::string	p_RootDir,
						const std::string	p_LogFile,
						const unsigned int	p_MaxMessageSize,
						const unsigned int	p_MaxConnections) :
		ListenAddress(p_ListenAddress),
		ListenPort(p_ListenPort),
		RootDir(p_RootDir),
		LogFile(p_LogFile),
		MaxMessageSize(p_MaxMessageSize),
		MaxConnections(p_MaxConnections)
	{
		if (MaxMessageSize == 0)
		{
			throw std::exception("MaxMessageSize can't be 0.");
		}
	}

}