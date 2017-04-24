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

#include <Network/Win32/TcpSocketWin32.hpp>
#ifdef QUE_PLATFORM_WINDOWS
#include <exception>
#include <System/MemoryLeak.hpp>

namespace Que
{

	static const uint64 g_MaxTimeout = 2147483647;

	TcpSocketWin32::TcpSocketWin32() :
		TcpSocketBase()
	{
	}

	TcpSocketWin32::~TcpSocketWin32()
	{
		Disconnect();
	}

	bool TcpSocketWin32::Connect(const Address & p_Address, const unsigned short p_Port,
		const Time & p_Timeout, const unsigned short p_EndpointPort)
	{
		// Create the socket
		if ((m_Handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
		{
			throw std::exception(std::string("Cannot create socket - " + std::to_string(static_cast<int>(GetLastError()))).c_str());
		}

		// Bind the socket to a port
		sockaddr_in service;

		if (p_EndpointPort != 0)
		{
			service.sin_family = AF_INET;
			service.sin_addr.s_addr = htonl(INADDR_ANY);
			service.sin_port = htons(static_cast<u_short>(p_EndpointPort));

			const int optVal = 1;
			const int optLen = sizeof(optVal);
			int rtn = setsockopt(m_Handle, SOL_SOCKET, SO_REUSEADDR, (const char*)&optVal, optLen);
			if (rtn != 0)
			{
				throw std::exception(std::string("Cannot set reusable socket - " + std::to_string(static_cast<int>(GetLastError()))).c_str());
			}

			// Bind
			if (bind(m_Handle, reinterpret_cast<const sockaddr *>(&service), sizeof(service)) == SOCKET_ERROR)
			{
				throw std::exception(std::string(	"Cannot bind socket to 0.0.0.0:" + std::to_string(p_EndpointPort) +
													" - " + std::to_string(static_cast<int>(GetLastError()))).c_str());
			}
		}

		// Create an object that's holding the host data
		service.sin_family = AF_INET;
		service.sin_addr.s_addr = htonl(static_cast<u_long>(p_Address.GetAddress()));
		service.sin_port = htons(static_cast<u_short>(p_Port));

		// We are using timeout
		// Get the blocking status and disable it.
		bool blocking = GetBlocking();
		SetBlocking(false);

		// Connect
		if (connect(m_Handle, (const sockaddr *)&service, sizeof(sockaddr_in)) != 0)
		{
			// Ignore the WSAEWOULDBLOCK error
			DWORD lastError = GetLastError();
			if (lastError != WSAEWOULDBLOCK)
			{
				Disconnect();
				return false;
			}
		}

		// We failed to connect, but we are waiting for the connection to establish
		struct timeval tv;
		if (p_Timeout.AsMicroseconds() / 1000000ULL > g_MaxTimeout)
		{
			tv.tv_sec = static_cast<long>(g_MaxTimeout);
			tv.tv_usec = static_cast<long>(0);
		}
		else
		{
			tv.tv_sec = static_cast<long>(p_Timeout.AsMicroseconds() / 1000000ULL);
			tv.tv_usec
				= static_cast<long>(p_Timeout.AsMicroseconds() % 1000000ULL);
		}

		// Create a FD_SET, and add the m_Handle to the set
		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(m_Handle, &fdset);

		// Select from the set
		if (select(static_cast<int>(m_Handle) + 1, NULL, &fdset, NULL, &tv) > 0)
		{
			// Check if the address is valid.
			Address address = GetPeerAddress();
			if (address == p_Address)
			{
				// The address is not 0, we successfully connected.
				SetBlocking(blocking);
				return true;
			}
		}

		DWORD lastError = GetLastError();

		// Failed to connect. Close the socket.
		Disconnect();

		// Failed.
		return false;
	}

	void TcpSocketWin32::Disconnect()
	{
		//shutdown(m_Handle, 2);

		// Close the socket handle.
		CloseHandle();
	}

	int TcpSocketWin32::Receive(void * p_pData, const unsigned int p_Size)
	{
		int size = recv(m_Handle, reinterpret_cast<char*>(p_pData), static_cast<int>(p_Size), 0);
		return size > 0 ? size : -1;
	}

	int TcpSocketWin32::Receive(void * p_pData, const unsigned int p_Size, const Time & p_Timeout)
	{
		// Create a socket address storage
		sockaddr_in address;
		int addressSize = sizeof(address);

		// Set blocking status
		bool blocking = GetBlocking();
		if (blocking)
		{
			SetBlocking(false);
		}

		// Put the socket handle in a fdset
		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(m_Handle, &fdset);

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
		if ((status = select(static_cast<int>(m_Handle) + 1, &fdset, NULL, NULL, &tv)) > 0)
		{
			// Receive the message
			int size = recv(m_Handle, reinterpret_cast<char*>(p_pData), static_cast<int>(p_Size), 0);

			// Restore the block status
			SetBlocking(blocking);

			if (size <= 0)
			{
				return -1;
			}

			// return the received message's size
			return size;
		}

		// Reset the blocking status and return false
		SetBlocking(blocking);
		return 0;
	}
	int TcpSocketWin32::Send(const void * p_pData, const unsigned int p_Size)
	{
		int size = send(m_Handle, reinterpret_cast<const char*>(p_pData), static_cast<int>(p_Size), 0);
		return static_cast<int>(size);
	}

	bool TcpSocketWin32::CheckConnection()
	{
		// Set blocking status
		bool blocking = GetBlocking();
		if (blocking)
		{
			SetBlocking(false);
		}

		// Receive with MSG_PEEK. This will force the message to be put back to next recv call.
		char temp;
		int size = recv(m_Handle, &temp, 1, MSG_PEEK);

		// Check if error.
		if (size < 0)
		{
			int error = GetLastError();
			// ignore "would block" errors.
			if (error = WSAEWOULDBLOCK)
			{
				SetBlocking(blocking);
				return true;
			}
		}
		SetBlocking(blocking);

		// Check if size is 0, then socket disconnected.
		return size != 0;
	}

}

#endif
