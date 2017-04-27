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

#include <MessageParser.hpp>
#include <algorithm>
#include <System/MemoryLeak.hpp>

namespace Que
{

	static const std::string g_EmptyString = "";

	static bool StringToInteger(const std::string & p_Input, unsigned int & p_Value)
	{
		if (p_Input.size() == 0 || p_Input.size() > 20)
		{
			throw false;
		}

		uint64 value = 0;

		int64 j = 0;
		for (int64 i = static_cast<int64>(p_Input.size()) - 1; i >= 0 ; i--)
		{
			char currChar = p_Input[i];

			if (currChar < '0' || currChar > '9')
			{
				return false;
			}

			int64 currNum = currChar - static_cast<int64>('0');
			value += currNum * pow(10, j);

			// Too big number.
			if (value > 4294967295ULL)
			{
				return false;
			}

			j++;
		}

		p_Value = static_cast<unsigned int>(value);
		return true;
	}

	static int FindNewlineOrSpace(const char * p_pBuffer, const unsigned int p_BufferSize)
	{
		int pos = -1;

		for (unsigned int i = 0; i < p_BufferSize; i++)
		{
			if (p_pBuffer[i] == '\n' || p_pBuffer[i] == ' ')
			{
				pos = static_cast<int>(i);
				break;
			}
		}

		return pos;
	}





	// Message parser class.
	MessageParser::MessageParser(char * p_pBuffer, const unsigned int p_BufferSize) :
		m_pBuffer(p_pBuffer),
		m_BufferSize(p_BufferSize),
		m_MessageStart(0),
		m_MessageSize(0),
		m_NextCommandPosition(0),
		m_Command("")
	{
	}

	MessageParser::~MessageParser()
	{
	}

	bool MessageParser::Parse()
	{
		if (m_pBuffer == NULL || m_BufferSize == 0)
		{
			throw std::exception("MessageParser::Parse: NULL Buffer or m_BufferSize");
		}
		
		int newLinePos = -1;
		m_Command = "";

		// Find first new line
		if ((newLinePos = FindNewlineOrSpace(m_pBuffer, m_BufferSize)) == -1)
		{
			m_NextCommandPosition = m_BufferSize;
			return false;
		}
		else if (newLinePos == 0)
		{
			m_NextCommandPosition++;
			return false;
		}

		m_NextCommandPosition = newLinePos + 1;

		// Get command
		m_Command.assign(m_pBuffer, 0, newLinePos);
		std::transform(m_Command.begin(), m_Command.end(), m_Command.begin(), ::toupper);

		if (m_Command == "PUSH")
		{
			if (m_pBuffer[newLinePos] != ' ')
			{
				m_Command = "";
				return false;
			}

			return ParseMessage(m_NextCommandPosition);
		}
		else if (m_Command == "PULL")
		{
			if (m_pBuffer[newLinePos] != '\n')
			{
				m_Command = "";
				return false;
			}

			return true;
		}
		else if (m_Command == "ABORT")
		{
			if (m_pBuffer[newLinePos] != '\n')
			{
				m_Command = "";
				return false;
			}

			return true;
		}
		else if (m_Command == "ACK")
		{
			if (m_pBuffer[newLinePos] != ' ')
			{
				m_Command = "";
				return false;
			}

			return ParseMessage(m_NextCommandPosition);
		}
		
		m_Command = "";
		return true;
	}


	bool MessageParser::ParseMessage(const unsigned int p_CurrentPosition)
	{
		// End of buffer.
		const unsigned int currPos = p_CurrentPosition;
		if (p_CurrentPosition >= m_BufferSize)
		{
			return false;
		}

		// Get next space
		int endOfMessPos = FindNewlineOrSpace(m_pBuffer + currPos, m_BufferSize + currPos);
		if (endOfMessPos == -1)
		{
			m_NextCommandPosition = m_BufferSize;
			m_Command = "";
			return false;
		}

		endOfMessPos += currPos;
		if (m_pBuffer[endOfMessPos] != '\n')
		{
			m_NextCommandPosition = endOfMessPos + 1;
			m_Command = "";
			return false;
		}
		
		m_NextCommandPosition = endOfMessPos + 1;
		std::string messageSizeString;
		messageSizeString.assign(m_pBuffer + currPos, endOfMessPos - currPos);
		unsigned int messageSize = 0;
		if (StringToInteger(messageSizeString, messageSize) == false)
		{
			m_Command = "";
			return false;
		}

		// Error check message size
		if (messageSize + m_NextCommandPosition > m_BufferSize)
		{
			m_Command = "";
			m_NextCommandPosition = m_BufferSize;
			return false;
		}

		unsigned int endCharacterPos = messageSize + m_NextCommandPosition;

		if (m_pBuffer[endCharacterPos] != '\n')
		{
			m_Command = "";
			m_NextCommandPosition = m_BufferSize;
			return false;
		}

		m_MessageStart = endOfMessPos + 1;
		m_MessageSize = messageSize;
		m_NextCommandPosition = endCharacterPos + 1;

		return true;
	}

	const std::string & MessageParser::GetCommand() const
	{
		return m_Command;
	}

	char * MessageParser::GetMessage(unsigned int & p_MessageSize)
	{
		p_MessageSize = m_MessageSize;
		return m_pBuffer + m_MessageStart;
	}

	unsigned int MessageParser::GetNextCommandPosition() const
	{
		return m_NextCommandPosition;
	}

}