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

#include <Connection.hpp>
#include <Server.hpp>
#include <Network/TcpSocket.hpp>
#include <Message.hpp>
#include <System/MemoryLeak.hpp>

namespace Que
{
	// Static variables
	static uint64 g_LastConnectionId = 0;
	
	// Connection class
	Connection::Connection(Server * p_pServer, TcpSocket * p_pTcpSocket) :
		m_pServer(p_pServer),
		m_pTcpSocket(p_pTcpSocket),
		m_State(Idle),
		m_pMessage(NULL)
	{
		// Error check pointer paramter.
		if (p_pServer == NULL)
		{
			throw std::invalid_argument("Connection::Connection(): Passed NULL Server.");
		}

		if (p_pTcpSocket == NULL)
		{
			throw std::invalid_argument("Connection::Connection(): Passed NULL TCP socket.");
		}

		m_Id = ++g_LastConnectionId;
	}

	Connection::~Connection()
	{
		delete m_pTcpSocket;
	}

	Server & Connection::GetServer() const
	{
		return *m_pServer;
	}

	TcpSocket & Connection::GetSocket()
	{
		return *m_pTcpSocket;
	}
	
	uint64 Connection::GetId() const
	{
		return m_Id;
	}

	Connection::eState Connection::GetState()
	{
		return m_State.Get();
	}

	void Connection::SetState(const eState p_State, Message * p_pMessage)
	{
		if (p_State == Producing && p_pMessage == NULL)
		{
			throw std::invalid_argument("Connection::SetState(): Message cannot be NULL.");
		}

		m_State.Set(p_State);
		m_pMessage = p_pMessage;
	}

	Message * Connection::GetMessage() const
	{
		return m_pMessage;
	}

}