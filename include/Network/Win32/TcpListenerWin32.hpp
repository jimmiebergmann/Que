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

#ifndef QUE_NETWORK_TCP_LISTENER_WIN32_HPP
#define QUE_NETWORK_TCP_LISTENER_WIN32_HPP

#include <Build.hpp>
#include <Network/TcpListenerBase.hpp>

namespace Que
{

	////////////////////////////////////////////////////////////////
	/// \ingroup Network
	/// \brief Win32 Tcp listener class.
	///
	////////////////////////////////////////////////////////////////
	class TcpListenerWin32 : private TcpListenerBase
	{

	public:

		////////////////////////////////////////////////////////////////
		/// \brief Default constructor
		///
		////////////////////////////////////////////////////////////////
		TcpListenerWin32();

		////////////////////////////////////////////////////////////////
		/// \brief Constructor
		///
		/// Start the listener.
		///
		/// \param p_Port The server port.
		///
		/// \see Host
		///
		////////////////////////////////////////////////////////////////
		TcpListenerWin32(const unsigned short p_Port, const Address & p_ListenAddress = Address(0));

		////////////////////////////////////////////////////////////////
		/// \brief Destructor
		///
		////////////////////////////////////////////////////////////////
		~TcpListenerWin32();

		////////////////////////////////////////////////////////////////
		/// \brief Start the listener.
		///
		/// \param p_Port The server port.
		///
		/// \throw std::exception If error occur during start.
		///
		////////////////////////////////////////////////////////////////
		virtual void Start(const unsigned short p_Port, const Address & p_ListenAddress = Address(0));

		////////////////////////////////////////////////////////////////
		/// \brief Stop the listener
		///
		////////////////////////////////////////////////////////////////
		virtual void Stop();

		////////////////////////////////////////////////////////////////
		/// \brief Listen for incoming connections
		///			This is a modal function.
		///
		/// \param p_TcpSocket The returned connected socket.
		///
		/// \return true if got connection else false
		///
		/// \throw std::exception If error occur during listen.
		///
		////////////////////////////////////////////////////////////////
		virtual bool Listen(TcpSocket & p_Connection, const Time & p_Timeout = Time::Infinite);

		////////////////////////////////////////////////////////////////
		/// \brief Checks if the listener is hosted.
		///
		/// \return True if hosted, else false.
		///
		////////////////////////////////////////////////////////////////
		virtual bool IsRunning() const;

	private:

		// Private varaibles
		bool m_Running;	    ///< Flag for checking if the listener is running.
		Socket m_Socket;    ///< Listener socket.

	};

}

#endif
