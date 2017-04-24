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

#ifndef QUE_NETWORK_CONNECTION_SELECTOR_HPP
#define QUE_NETWORK_CONNECTION_SELECTOR_HPP

#include <Build.hpp>
#include <Network/Socket.hpp>
#include <System/Time.hpp>
#include <mutex>
#include <map>
#include <list>
#include <Connection.hpp>

namespace Que
{

	////////////////////////////////////////////////////////////////
	/// \ingroup Network
	/// \brief Connection selector class.
	/// 
	////////////////////////////////////////////////////////////////
	class ConnectionSelector
	{

	public:

		typedef std::list<Connection *> ConnectionList;

		////////////////////////////////////////////////////////////////
		/// \brief Default constructor
		/// 
		////////////////////////////////////////////////////////////////
		ConnectionSelector();

		////////////////////////////////////////////////////////////////
		/// \brief Destructor.
		/// 
		////////////////////////////////////////////////////////////////
		~ConnectionSelector();

		////////////////////////////////////////////////////////////////
		/// \brief Select an active socket.
		///
		/// \return True if paramter m_Socket is set to an active socket,
		///			else false.
		/// 
		////////////////////////////////////////////////////////////////
		bool Select(ConnectionList & p_SocketList, const Time & p_Timeout = Time::Infinite);

		////////////////////////////////////////////////////////////////
		/// \brief Add socket to the selector.
		/// 
		////////////////////////////////////////////////////////////////
		bool Add(Connection * p_pConnection);

		////////////////////////////////////////////////////////////////
		/// \brief Remove socket from the selector.
		/// 
		////////////////////////////////////////////////////////////////
		bool Remove(Connection * p_pConnection);

	private:

		// Private typedefs
		typedef std::map<SocketHandle, Connection*> ConnectiontMap;

		// Private variables
		SocketHandle	m_LatestHandle;
		ConnectiontMap	m_Connections;
		std::mutex		m_Mutex;

	};

}

#endif