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

#ifndef QUE_CONNECTION_HPP
#define QUE_CONNECTION_HPP

#include <Build.hpp>
#include <System\ThreadValue.hpp>
#include <Network\TcpSocket.hpp>

namespace Que
{

	// Forward declarations
	class Server;
	class Message;

	////////////////////////////////////////////////////////////////
	/// \brief Connection class.
	///
	////////////////////////////////////////////////////////////////
	class Connection
	{

	public:

		enum eState
		{
			Idle,
			Producing,
			Consuming
		};

		////////////////////////////////////////////////////////////////
		/// \brief Constructor
		///
		/// \param p_pTcpSocket Pointer to connection socket.
		///
		/// \throw std::invalid_argument If p_pTcpSocket is NULL.
		///
		////////////////////////////////////////////////////////////////
		Connection(Server * p_pServer, TcpSocket * p_pTcpSocket);

		////////////////////////////////////////////////////////////////
		/// \brief Destructor
		///
		////////////////////////////////////////////////////////////////
		~Connection();

		////////////////////////////////////////////////////////////////
		/// \brief Get parent server.
		///
		////////////////////////////////////////////////////////////////
		Server & GetServer() const;

		////////////////////////////////////////////////////////////////
		/// \brief Get TCP socket.
		///
		////////////////////////////////////////////////////////////////
		TcpSocket & GetSocket();

		////////////////////////////////////////////////////////////////
		/// \brief Get id of connection
		///
		////////////////////////////////////////////////////////////////
		uint64 GetId() const;

		////////////////////////////////////////////////////////////////
		/// \brief Get current connection state.
		///
		////////////////////////////////////////////////////////////////
		eState GetState();

		////////////////////////////////////////////////////////////////
		/// \brief Set connection type. Also sets new message.
		///
		////////////////////////////////////////////////////////////////
		void SetState(const eState p_State, Message * p_pMessage = NULL);

		////////////////////////////////////////////////////////////////
		/// \brief Get current message of connection. Null if type == None.
		///
		////////////////////////////////////////////////////////////////
		Message * GetMessage() const;

	private:

		// Private variables.
		Server *				m_pServer;		///< Pointer to server.
		uint64					m_Id;			///< Id of current user.
		TcpSocket *				m_pTcpSocket;	///< Pointer to communication socket.
		ThreadValue<eState>		m_State;			///< Producer/consumer, Idle if no command is executed.
		Message *				m_pMessage;		///< Current pushed or pulled message.

	};

}

#endif