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

#include <Network/Win32/SocketWin32.hpp>


#ifdef QUE_PLATFORM_WINDOWS
#include <iostream>
#include <Log.hpp>
#include <System/MemoryLeak.hpp>

namespace Que
{

	SocketWin32::SocketWin32() :
		SocketBase()
	{
		m_Handle = 0;
	}

	SocketWin32::SocketWin32(const SocketHandle & p_SocketHandle) :
		SocketBase(p_SocketHandle)
	{
		m_Handle = p_SocketHandle;
	}

	SocketWin32::~SocketWin32()
	{
		CloseHandle();
	}

	bool SocketWin32::SetBlocking(const bool p_Blocking)
	{
		// The blocking status is already set.
		if (m_Blocking == p_Blocking)
		{
			return true;
		}

		if (!m_Handle)
		{
			return false;
		}
		// If blocking = 0, blocking is enabled.
		// If blocking != 0, non-blocking mode is enabled.
		u_long blocking = p_Blocking ? 0 : 1;

		int result = ioctlsocket(m_Handle, FIONBIO, &blocking);
		if (result != NO_ERROR)
		{
			return false;
		}

		m_Blocking = !static_cast<bool>(blocking);
		return true;
	}

	bool SocketWin32::GetBlocking() const
	{
		return m_Blocking;
	}

	void SocketWin32::CloseHandle()
	{
		if (m_Handle)
		{
			closesocket(m_Handle);
			m_Handle = 0;
		}
	}

	void SocketWin32::SetHandle(const SocketHandle & m_SocketHandle)
	{
		m_Handle = m_SocketHandle;
	}

	Address SocketWin32::GetPeerAddress() const
	{
		// Get the address
		sockaddr_in address;
		int size = sizeof(address);
		getpeername(m_Handle, reinterpret_cast<sockaddr *>(&address), &size);

		// Return the address
		return Address(static_cast<unsigned int>(ntohl((u_long)address.sin_addr.S_un.S_addr)));
	}

	unsigned short SocketWin32::GetPeerPort() const
	{
		// Get the address
		SOCKADDR_IN address;
		int size = sizeof(address);
		getpeername(m_Handle, reinterpret_cast<sockaddr *>(&address), &size);

		// Return the port
		return ntohs(address.sin_port);
	}

	const SocketHandle SocketWin32::GetHandle() const
	{
		return m_Handle;
	}


	// Intiialize winsock( A little hack from sfml )
	struct WinsockInitializer
	{
		WinsockInitializer()
		{
			WSADATA wsaData;
			if (WSAStartup(MAKEWORD(2, 2), &wsaData))
			{
				QueLogError("Socket Error: Failed to initialize winsock.");
			}
		}
	};

	// Create an instance of the intiialize
	static WinsockInitializer g_WinsockInitializer;

}

#endif
