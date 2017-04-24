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

#include <Network/Win32/TcpListenerWin32.hpp>
#ifdef QUE_PLATFORM_WINDOWS
#include <exception>
#include <System/MemoryLeak.hpp>

namespace Que
{

	static const uint64 g_MaxTimeout = 2147483647;

	TcpListenerWin32::TcpListenerWin32() :
		m_Running(false),
		m_Socket()
	{
	}

	TcpListenerWin32::TcpListenerWin32(const unsigned short p_Port, const Address & p_ListenAddress) :
		m_Running(false),
		m_Socket()
	{
		// Start the listener
		Start(p_Port);
	}

	TcpListenerWin32::~TcpListenerWin32()
	{
		Stop();
	}

	void TcpListenerWin32::Start(const unsigned short p_Port, const Address & p_ListenAddress)
	{
		// Create the socket
		SocketHandle socketHandle;
		if ((socketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) <= 0)
		{
			throw std::exception(std::string("Cannot create socket - " + std::to_string(static_cast<int>(GetLastError()))).c_str());
		}
		m_Socket.SetHandle(socketHandle);

		// Create an object that's holding the host data
		sockaddr_in service;
		service.sin_family = AF_INET;
		service.sin_addr.s_addr = htonl(p_ListenAddress.GetAddress());
		service.sin_port = htons(static_cast<u_short>(p_Port));

		// Bind
		if (bind(m_Socket.GetHandle(), reinterpret_cast<const sockaddr *>(&service), sizeof(service)) != 0)
		{
			Stop();
			throw std::exception(std::string(	"Cannot bind socket to " + p_ListenAddress.GetPretty() + ":" + std::to_string(p_Port) +
												" - " + std::to_string(static_cast<int>(GetLastError()))).c_str());
		}

		m_Socket.SetBlocking(false);

		// Succeeded
		m_Running = true;
	}

	void TcpListenerWin32::Stop()
	{
		// Close the socket handle
		m_Socket.CloseHandle();

		m_Running = false;
	}

	bool TcpListenerWin32::Listen(TcpSocket & p_TcpSocket, const Time & p_Timeout)
	{
		// Set blocking to false
		bool blocking = GetBlocking();
		if (blocking)
		{
			SetBlocking(false);
		}

		// Listen for incomming clients
		if (listen(m_Socket.GetHandle(), SOMAXCONN) != 0)
		{
			throw std::exception(std::string("Cannot listen - " + std::to_string(static_cast<int>(GetLastError()))).c_str());
		}

		// Put the socket handle in a fdset
		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(m_Socket.GetHandle(), &fdset);

		// Set the time
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

		// Select from the fdset
		int status = 0;
		if ((status = select(1, &fdset, NULL, NULL, &tv)) > 0)
		{
			// Accept the client
			SocketHandle acceptSocket = 0;
			if ((acceptSocket = accept(m_Socket.GetHandle(), NULL, NULL)) == -1)
			{
				throw std::exception(std::string(	"Cannot accept socket " + m_Socket.GetPeerAddress().GetPretty() + ":" + std::to_string(m_Socket.GetPeerPort()) +
													" - " + std::to_string(static_cast<int>(GetLastError()))).c_str());
			}

			// Set the handle for the TcpSocket.
			p_TcpSocket.SetHandle(acceptSocket);

			// Reset the blocking status
			SetBlocking(blocking);

			return true;
		}

		// Reset the blocking status
		SetBlocking(blocking);

		// Failed
		return false;
	}

	bool TcpListenerWin32::IsRunning() const
	{
		return m_Running;
	}

}

#endif
