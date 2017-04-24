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

#ifndef QUE_SETTINGS_HPP
#define QUE_SETTINGS_HPP

#include <Build.hpp>
#include <Network/Address.hpp>
#include <string>

namespace Que
{

	////////////////////////////////////////////////////////////////
	/// \brief Settings for Que server.
	///
	/// \see Server
	///
	////////////////////////////////////////////////////////////////
	class Settings
	{

	public:

		////////////////////////////////////////////////////////////////
		/// \brief Constructor
		///
		////////////////////////////////////////////////////////////////
		Settings(	const Address		p_ListenAddress = Address(0),
					const unsigned int	p_ListenPort = 11400,
					const std::string	p_RootDir = "",
					const std::string	p_LogFile = "",
					const unsigned int	p_MaxMessageSize = 65535,
					const unsigned int	p_MaxConnections = 0);

		// Public variables.
		Address			ListenAddress;	///< Listening address.
		unsigned int	ListenPort;		///< Listening port.
		std::string		RootDir;		///< Server root path.
		std::string		LogFile;		///< Log file path.
		unsigned int	MaxMessageSize; ///< Maximum size of message.
		unsigned int	MaxConnections;	//< NOT IN USE YET.
	};
}

#endif