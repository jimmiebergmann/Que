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

#include <Message.hpp>
#include <sstream>
#include <System/MemoryLeak.hpp>

namespace Que
{

	unsigned int Message::GetSize() const
	{
		return m_Size;
	}


	Message::Message(const void * p_pMessage,
		const unsigned int p_Size,
		const uint64 p_Producer) :
			m_pMessage(NULL),
			m_Size(p_Size),
			m_Producer(p_Producer),
			m_Consumer(0)
	{
		if (p_pMessage == NULL)
		{
			throw std::invalid_argument("Message is NULL.");
		}

		if (p_Size == 0)
		{
			throw std::invalid_argument("Message size can't be 0.");
		}

		m_PushedTime = Time::GetSystemTime();

		// Create header
		std::stringstream ss;
		ss << "PULLING "  << p_Size << " ";
		const unsigned int headerSize = ss.str().size();

		// Copy message
		const unsigned int fullSize = p_Size + headerSize + 1;
		m_pMessage = new char[fullSize];
		
		memcpy(m_pMessage, ss.str().c_str(), headerSize);
		memcpy(m_pMessage + headerSize, p_pMessage, p_Size);

		m_pMessage[headerSize - 1] = '\n';
		m_pMessage[fullSize - 1] = '\n';

		m_Size = fullSize;
	}

	Message::~Message()
	{
		if (m_pMessage)
		{
			delete[] m_pMessage;
		}
	}

	void Message::SetConsumer(const uint64 p_Consumer)
	{
		m_Consumer = p_Consumer;
		m_PulledTime = Time::GetSystemTime();
	}

	uint64 Message::GetProdcer() const
	{
		return m_Producer;
	}

	uint64 Message::GetConsumer() const
	{
		return m_Consumer;
	}

	const Time & Message::GetPushTime() const
	{
		return m_PushedTime;
	}

	const Time & Message::GetPullTime() const
	{
		return m_PulledTime;
	}


	// Private operators.
	Message::Message(const Message & p_Message)
	{
	}

	Message & Message::operator = (const Message & p_Message)
	{
		return *this;
	}


}