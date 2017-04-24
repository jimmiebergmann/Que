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

#include <Network/Address.hpp>
#include <Network/Socket.hpp>
#include <sstream>
#include <System/MemoryLeak.hpp>

namespace Que
{

	const Address Address::NoAddress = Address(0);
	const Address Address::Localhost = Address(2130706433);

	Address::Address() :
		m_Address(0)
	{
	}

	Address::Address(const unsigned char p_A, const unsigned char p_B,
		const unsigned char p_C, const unsigned char p_D)
	{
		/// Bit swift the address
		m_Address = (p_A << 24) | (p_B << 16) | (p_C << 8) | p_D;
	}

	Address::Address(const unsigned int p_Address) :
		m_Address(p_Address)
	{
	}

	Address::Address(const std::string & p_String)
	{
		if (SetAddressFromString(p_String) == false)
		{
			m_Address = 0;
		}
	}

	bool Address::SetAddressFromString(const std::string & p_String)
	{
		// Example input:
		// 127.0.0.1
		// 192.168.0.1
		// www.google.com
		// http://www.sunet.se
		// 0.europe.pool.ntp.org

		// Get the address
		Address address = Socket::GetHostByName(p_String);
		if (address == NoAddress)
		{
			return false;
		}

		// Set the internal address varaible.
		m_Address = address.GetAddress();

		// Succeeded
		return true;
	}

	unsigned int Address::GetAddress() const
	{
		return m_Address;
	}

	unsigned char Address::GetA() const
	{
		return static_cast<unsigned char>(m_Address >> 24);
	}

	unsigned char Address::GetB() const
	{
		return static_cast<unsigned char>(m_Address >> 16);
	}

	unsigned char Address::GetC() const
	{
		return static_cast<unsigned char>(m_Address >> 8);
	}

	unsigned char Address::GetD() const
	{
		return static_cast<unsigned char>(m_Address);
	}

	std::string Address::GetPretty() const
	{
		std::stringstream ss;
		ss << static_cast<unsigned int>(GetA()) << ".";
		ss << static_cast<unsigned int>(GetB()) << ".";
		ss << static_cast<unsigned int>(GetC()) << ".";
		ss << static_cast<unsigned int>(GetD());
		return ss.str();
	}

	void Address::SetA(const unsigned char p_A)
	{
		m_Address &= ~(0xFF000000);
		m_Address |= p_A << 24;
	}

	void Address::SetB(const unsigned char p_B)
	{
		m_Address &= ~(0xFF0000);
		m_Address |= p_B << 16;
	}

	void Address::SetC(const unsigned char p_C)
	{
		m_Address &= ~(0xFF00);
		m_Address |= p_C << 8;
	}

	void Address::SetD(const unsigned char p_D)
	{
		m_Address &= ~(0xFF);
		m_Address |= p_D;
	}

	bool Address::operator == (const Address & p_Address) const
	{
		return m_Address == p_Address.GetAddress();
	}

	bool Address::operator != (const Address & p_Address) const
	{
		return m_Address != p_Address.GetAddress();
	}

	bool Address::operator < (const Address & p_Address) const
	{
		return m_Address < p_Address.m_Address;
	}

	bool Address::operator > (const Address & p_Address) const
	{
		return m_Address > p_Address.m_Address;
	}

}