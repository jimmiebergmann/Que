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

#include <ConnectionSelector.hpp>
#include <System/MemoryLeak.hpp>

namespace Que
{

	static const uint64 g_MaxTimeout = 2147483647;

	ConnectionSelector::ConnectionSelector()
	{
	}

	ConnectionSelector::~ConnectionSelector()
	{
	}

	bool ConnectionSelector::Select(ConnectionList & p_ConnectionList, const Time & p_Timeout)
	{
		p_ConnectionList.clear();
		SocketHandle highestHandle = 0;
		fd_set socketSet;
		FD_ZERO(&socketSet);

		m_Mutex.lock();

		// Add child connection to set
		for (auto it = m_Connections.begin(); it != m_Connections.end(); it++)
		{
			TcpSocket & socket = it->second->GetSocket();
			SocketHandle handle = socket.GetHandle();

			//if valid socket descriptor then add to read list
			if (handle > 0)
				FD_SET(handle, &socketSet);

			//highest file descriptor number, need it for the select function
			if (handle > highestHandle)
			{
				highestHandle = handle;
			}
		}

		m_Mutex.unlock();


		// Create timeout structure.
		struct timeval tv;
		if (p_Timeout.AsMicroseconds() / 1000000ULL > g_MaxTimeout)
		{
			tv.tv_sec = static_cast<long>(g_MaxTimeout);
			tv.tv_usec = static_cast<long>(0);
		}
		else
		{
			tv.tv_sec = static_cast<long>(p_Timeout.AsMicroseconds() / 1000000ULL);
			tv.tv_usec = static_cast<long>(p_Timeout.AsMicroseconds() % 1000000ULL);
		}


		// Select from the set
		if (select(highestHandle + 1, &socketSet, NULL, NULL, &tv) >= 0)
		{
			std::lock_guard<std::mutex> lock(m_Mutex);

			for (auto it = m_Connections.begin(); it != m_Connections.end(); it++)
			{
				Socket & socket = it->second->GetSocket();
				SocketHandle handle = socket.GetHandle();

				if (FD_ISSET(handle, &socketSet))
				{
					p_ConnectionList.push_back(it->second);
				}
			}

			return p_ConnectionList.size() > 0;
		}

		return false;
	}

	bool ConnectionSelector::Add(Connection * p_pConnection)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		if (p_pConnection == NULL)
		{
			return false;
		}

		SocketHandle handle = p_pConnection->GetSocket().GetHandle();

		// Do not add the connection if it's already in the map
		auto it = m_Connections.find(handle);
		if (it != m_Connections.end())
		{
			return false;
		}

		// Add the socket to the map
		m_Connections[handle] = p_pConnection;

		return true;
	}

	bool ConnectionSelector::Remove(Connection * p_pConnection)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		if (p_pConnection == NULL)
		{
			return false;
		}

		SocketHandle handle = p_pConnection->GetSocket().GetHandle();

		// Find the socket
		auto it = m_Connections.find(handle);
		if (it == m_Connections.end())
		{
			return false;
		}

		m_Connections.erase(it);

		return true;
	}

}