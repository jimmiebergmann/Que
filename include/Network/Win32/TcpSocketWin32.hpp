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

#ifndef QUE_NETWORK_TCP_SOCKET_WIN32_HPP
#define QUE_NETWORK_TCP_SOCKET_WIN32_HPP

#include <Build.hpp>
#include <Network/TcpSocketBase.hpp>

namespace Que
{

	////////////////////////////////////////////////////////////////
	/// \ingroup Network
	/// \brief Win32 Tcp socket class.
	///
	////////////////////////////////////////////////////////////////
	class TcpSocketWin32 : public TcpSocketBase
	{

	public:

		////////////////////////////////////////////////////////////////
		/// \brief Default constructor
		///
		////////////////////////////////////////////////////////////////
		TcpSocketWin32();

		////////////////////////////////////////////////////////////////
		/// \brief Destructor
		///
		////////////////////////////////////////////////////////////////
		~TcpSocketWin32();

		////////////////////////////////////////////////////////////////
		/// \brief Connect the socket to a server.
		///
		/// \param p_Address Server address.
		/// \param p_Port Server port.
		/// \param m_Timeout Time in milliseconds until
		///		the connection attemp timeouts.
		///
		/// \return true if connection succeeded, else false.
		///
		/// \throw std::exception If error occur during listen.
		///
		////////////////////////////////////////////////////////////////
		virtual bool Connect(const Address & p_Address,
			const unsigned short p_Port,
			const Time & p_Timeout = Time::Infinite,
			const unsigned short p_EndpointPort = 0);

		////////////////////////////////////////////////////////////////
		/// \brief Disconnect the socket from the server.
		///
		////////////////////////////////////////////////////////////////
		virtual void Disconnect();

		////////////////////////////////////////////////////////////////
		/// \brief Receive data from server.
		///
		/// \param p_pData Data from server.
		/// \param p_Size The size of the data.
		///
		/// \return > 0 if receied data.
		///			== -1 if socket disconnected.
		///			
		///
		////////////////////////////////////////////////////////////////
		virtual int Receive(void * p_pData, const unsigned int p_Size);

		////////////////////////////////////////////////////////////////
		/// \brief Receive data from server.
		///
		/// \param p_pData Data from server.
		/// \param p_Size The size of the data.
		/// \param m_Timeout Time until the attemp in receiving a message timeouts.
		///
		/// \return > 0 if receied data.
		///			== 0 if function timed out.
		///			== -1 if socket disconnected.
		///
		////////////////////////////////////////////////////////////////
		virtual int Receive(void * p_pData, const unsigned int p_Size, const Time & p_Timeout);

		////////////////////////////////////////////////////////////////
		/// \brief Send data to server.
		///
		/// \param p_pData The data to be sent to server.
		/// \param p_Size The size of the data.
		///
		////////////////////////////////////////////////////////////////
		virtual int Send(const void * p_pData, const unsigned int p_Size);

		////////////////////////////////////////////////////////////////
		/// \brief Check the connection.
		///
		/// \return True if connected, else false.
		///
		////////////////////////////////////////////////////////////////
		virtual bool CheckConnection();

	};

}


#endif
