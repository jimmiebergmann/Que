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

#ifndef QUE_MESSAGE_PARSER_HPP
#define QUE_MESSAGE_PARSER_HPP

#include <Build.hpp>
#include <string>
#include <vector>

namespace Que
{

	////////////////////////////////////////////////////////////////
	/// \brief  Message parser class.
	///			Used for parsing all incoming messages to server.
	///			All messages should start with a command word,
	///			and all parameters should be separated by spaces(' ').
	///
	/// \see Connection
	///
	////////////////////////////////////////////////////////////////
	class MessageParser
	{

	public:

		////////////////////////////////////////////////////////////////
		/// \brief Constructor.
		///
		////////////////////////////////////////////////////////////////
		MessageParser(char * p_pBuffer, const unsigned int p_BufferSize);

		////////////////////////////////////////////////////////////////
		/// \brief Desstructor.
		///
		////////////////////////////////////////////////////////////////
		~MessageParser();

		////////////////////////////////////////////////////////////////
		/// \brief Parse the message buffer.
		///
		/// \return false if protocol errors are found, else true.
		///
		////////////////////////////////////////////////////////////////
		bool Parse();

		////////////////////////////////////////////////////////////////
		/// \brief Get command name.
		///
		////////////////////////////////////////////////////////////////
		const std::string & GetCommand() const;

		////////////////////////////////////////////////////////////////
		/// \brief Get message data.
		///
		/// \return NULL of no data is available, else pointer to data.
		///
		////////////////////////////////////////////////////////////////
		char * GetMessage(unsigned int & p_MessageSize);

		////////////////////////////////////////////////////////////////
		/// \brief Get message data.
		///
		/// \return NULL of no data is available, else pointer to data.
		///
		////////////////////////////////////////////////////////////////
		unsigned int GetNextCommandPosition() const;

	private:

		// Private functions

		////////////////////////////////////////////////////////////////
		/// \brief Private function for parsing PUSH or ACK message.
		///
		////////////////////////////////////////////////////////////////
		bool ParseMessage(const unsigned int p_CurrentPosition);

		// Private variables
		std::string			m_Command;
		char *				m_pBuffer;			///< Parsing buffer.
		unsigned int		m_BufferSize;		///< Size of buffer.
		unsigned int		m_MessageStart;		///< Start of data message(PUSH/ACK).
		unsigned int		m_MessageSize;		///< Size of message;
		unsigned int		m_NextCommandPosition;

	};

}

#endif