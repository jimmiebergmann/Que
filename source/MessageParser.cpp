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


	// Parameter class.
	MessageParser::Parameter::Parameter() :
		m_Type(None)
	{

	}

	MessageParser::Parameter::~Parameter()
	{
		if (m_Type == String)
		{
			delete m_Data._String;
		}
	}

	MessageParser::Parameter::Parameter(const std::string & p_Data)
	{
		// Empty paramter input
		if (p_Data.size() == 0 )
		{
			return;
		}

		// Negative value, could be a signed integer.
		unsigned int integer = 0;
		if (StringToInteger(p_Data, integer))
		{
			m_Type = Integer;
			m_Data._Integer = integer;
			return;
		}

		// This is not an integer, then it's a string
		m_Type = String;
		m_Data._String = new std::string;
		*m_Data._String = p_Data;
	}

	MessageParser::Parameter::eType MessageParser::Parameter::GetType() const
	{
		return m_Type;
	}

	bool MessageParser::Parameter::Empty() const
	{
		return m_Type == None;
	}

	unsigned int MessageParser::Parameter::AsInteger() const
	{
		switch (m_Type)
		{
		case Integer:
		{
			return m_Data._Integer;
		}
		break;
		case String:
		{
			return 0;
		}
		break;
		default:
			return 0;
			break;
		}
		return 0;
	}

	const std::string & MessageParser::Parameter::AsString() const
	{
		switch (m_Type)
		{
		case Integer:
		{
			return g_EmptyString;
		}
		break;
		case String:
		{
			return *m_Data._String;
		}
		break;
		default:
			return g_EmptyString;
			break;
		}

		return g_EmptyString;
	}


	// Message parser class.
	MessageParser::Parameter MessageParser::m_NoneParameter;

	MessageParser::MessageParser() :
		m_Command(""),
		m_NewLinePos(0)
	{

	}

	MessageParser::~MessageParser()
	{
		for (unsigned int i = 0; i < GetParameterCount(); i++)
		{
			delete m_Parameters[i];
		}
	}

	bool MessageParser::Parse(const char * p_pBuffer, const unsigned int p_BufferSize)
	{
		if (p_pBuffer == NULL || p_BufferSize == 0)
		{
			return false;
		}

		unsigned int currentPos = 0;
		unsigned currentWordLength = 0;
		bool lastWord = false;

		// Loop the buffer
		for (unsigned int i = 0; i < p_BufferSize; i++)
		{
			if (p_pBuffer[i] == '\n')
			{
				m_NewLinePos = i;
				lastWord = true;
			}
			else if (p_pBuffer[i] != ' ')
			{
				continue;
			}

			const char * pBuffStart = p_pBuffer + currentPos;
			const unsigned int wordLength = i - currentPos;

			// Multiple spaces? Ignore them
			if (wordLength == 0)
			{
				if (lastWord)
				{
					return true;
				}

				currentPos++;
				continue;
			}

			std::string word;
			word.assign(pBuffStart, 0, wordLength);

			if (m_Command.size() == 0)
			{
				m_Command = word;
				std::transform(m_Command.begin(), m_Command.end(), m_Command.begin(), ::toupper);
			}
			else
			{
				Parameter * pParameter = new Parameter(word);
				if (pParameter->GetType() == Parameter::None)
				{
					delete pParameter;
					return false;
				}

				m_Parameters.push_back(pParameter);
			}

			// Set new current pos
			currentPos = i + 1;

			// This is the last word, break!
			if (lastWord)
			{
				break;
			}
		}

		return lastWord;
	}

	const std::string & MessageParser::GetCommand() const
	{
		return m_Command;
	}

	const MessageParser::Parameter & MessageParser::GetParameter(const unsigned int p_Index) const
	{
		if (p_Index >= GetParameterCount())
		{
			return m_NoneParameter;
		}

		return *m_Parameters[p_Index];
	}

	unsigned int MessageParser::GetParameterCount() const
	{
		return static_cast<unsigned int>(m_Parameters.size());
	}

	unsigned int MessageParser::GetNewLinePosition() const
	{
		return m_NewLinePos;
	}











	/*
	static bool StringToUint64(const std::string & p_Input, uint64 & p_Out);

	MessageParser::MessageParser(char * p_pBuffer, const int p_BufferSize) :
		m_pBuffer(p_pBuffer),
		m_BufferSize(p_BufferSize),
		m_CurrentPos(0)
	{
	}

	MessageParser::eResult MessageParser::GetWord(std::string & p_Word, const int p_MaxLength)
	{
		if (m_CurrentPos >= m_BufferSize)
		{
			return EndOfBuffer;
		}

		bool reachedMax = false;
		bool reachedEnd = false;
		int iteratedCount = 0;
		int searchStart = m_CurrentPos;

		// Go through buffer, find space or end of buffer.
		int i = searchStart;
		for (i;  i < m_BufferSize; i++)
		{
			if (m_CurrentPos - searchStart >= p_MaxLength)
			{
				reachedMax = true;
				break;
			}

			m_CurrentPos++;

			if (m_pBuffer[i] == ' ')
			{
				break;
			}

			iteratedCount++;

			// Just allow readable ascii characters.
			if (m_pBuffer[i] < 33 || m_pBuffer[i] > 126)
			{
				return Invalid;
			}

			if (i + 1 == m_BufferSize)
			{
				reachedEnd = true;
				break;
			}

		}

		// Copy message
		p_Word.assign(m_pBuffer + searchStart, iteratedCount);

		if (reachedEnd)
		{
			return ReachedEnd;
		}

		if (reachedMax)
		{
			return ReachedMax;
		}

		return Ok;
	}

	MessageParser::eResult MessageParser::GetInteger(uint64 & p_Integer, const int p_MaxLength)
	{
		std::string word = "";
		eResult result = GetWord(word, p_MaxLength);

		if(!StringToUint64(word, p_Integer))
		{
			return Invalid;
		}

		return result;
	}

	char * MessageParser::GetBuffer() const
	{
		return m_pBuffer;
	}

	int MessageParser::GetBufferSize() const
	{
		return m_BufferSize;
	}
	int MessageParser::GetPosition() const
	{
		return m_CurrentPos;
	}

	bool MessageParser::HasReachedEnd() const
	{
		return m_CurrentPos >= m_BufferSize;
	}

	bool StringToUint64(const std::string & p_Input, uint64 & p_Out)
	{
		uint64 result = 0;

		int c = 0;
		for (int i = p_Input.size() - 1; i >= 0; i--)
		{
			if (p_Input[i] < '0' && p_Input[i] > '9')
			{
				return false;
			}

			uint64 curNum = static_cast<uint64>(p_Input[c]) - 48ULL;
			result += curNum * (std::pow<uint64>(10, i));
			c++;
		}

		p_Out = result;
		return true;
	}
	*/
}