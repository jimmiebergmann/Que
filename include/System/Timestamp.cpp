// ///////////////////////////////////////////////////////////////////////////
// Copyright (C) 2013 Jimmie Bergmann - jimmiebergmann@gmail.com
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

#include <System/Timestamp.hpp>
#include <ctime>
#include <chrono>

namespace Que
{

	// Static variables
	static const unsigned int g_MonthCount = 12;
	static const unsigned char g_DaysInMonth[g_MonthCount] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	// Timestamp class
	Timestamp::Timestamp() :
		m_Year(0),
		m_Month(0),
		m_Day(0),
		m_Hours(0),
		m_Minutes(0),
		m_Seconds(0),
		m_Microseconds(0)
	{

	}

	Timestamp::Timestamp(	const unsigned short	p_Year,
							const unsigned char		p_Month,
							const unsigned char		p_Day,
							const unsigned char		p_Hours,
							const unsigned char		p_Minutes,
							const unsigned char		p_Seconds,
							const unsigned int		p_Microseconds) :
		m_Year(p_Year),
		m_Month(p_Month),
		m_Day(p_Day),
		m_Hours(p_Hours),
		m_Minutes(p_Minutes),
		m_Seconds(p_Seconds),
		m_Microseconds(p_Microseconds)
	{

	}

	void Timestamp::SetYear(const unsigned short p_Year)
	{
		m_Year = p_Year;
	}

	void Timestamp::SetMonth(const unsigned char p_Month)
	{
		m_Month = p_Month;
	}

	void Timestamp::SetDay(const unsigned char p_Day)
	{
		m_Day = p_Day;
	}

	void Timestamp::SetHour(const unsigned char p_Hours)
	{
		m_Hours = p_Hours;
	}

	void Timestamp::SetMinute(const unsigned char p_Minutes)
	{
		m_Minutes = p_Minutes;
	}

	void Timestamp::SetSecond(const unsigned char p_Seconds)
	{
		m_Seconds = p_Seconds;
	}

	void Timestamp::SetMicrosecond(const unsigned int p_Microseconds)
	{
		m_Microseconds = p_Microseconds;
	}

	int Timestamp::GetYear() const
	{
		return static_cast<int>(m_Year);
	}

	int Timestamp::GetMonth() const
	{
		return static_cast<int>(m_Month);
	}

	int Timestamp::GetDay() const
	{
		return static_cast<int>(m_Day);
	}

	int Timestamp::GetHour() const
	{
		return static_cast<int>(m_Hours);
	}

	int Timestamp::GetMinute() const
	{
		return static_cast<int>(m_Minutes);
	}

	int Timestamp::GetSecond() const
	{
		return static_cast<int>(m_Seconds);
	}

	int Timestamp::GetMicrosecond() const
	{
		return static_cast<int>(m_Microseconds);
	}

	bool Timestamp::Validate() const
	{
		// Validate the clock and month
		bool valid = m_Hours < 24 && m_Minutes < 60 && m_Seconds < 60 &&
			m_Month <= 12 && m_Month > 0;

		if (valid == false)
		{
			return false;
		}

		// Validate the day.
		// Add leap year of needed.
		unsigned char daysInMonth = g_DaysInMonth[m_Month - 1];
		if (m_Month == 2 && IsLeapYear())
		{
			daysInMonth++;
		}

		return m_Day <= daysInMonth && m_Day > 0;
	}

	bool Timestamp::IsLeapYear() const
	{
		if ((m_Year % 400 == 0 || m_Year % 100 != 0) && (m_Year % 4 == 0))
		{
			return true;
		}

		return false;
	}

	Timestamp Timestamp::Now()
	{
		/*tm localtm;
		std::chrono::system_clock::time_point t = std::chrono::system_clock::now();
		time_t now = std::chrono::system_clock::to_time_t(t);
		//localtime_r(&now, &localTime);
		if (localtime_s(&localtm, &now) != 0)
		{
			return Timestamp();
		}

		const std::chrono::duration<double> tse = t.time_since_epoch();
		std::chrono::seconds::rep microseconds = std::chrono::duration_cast<std::chrono::microseconds>(tse).count() % 1000000;

		Timestamp timestamp(localtm.tm_year + 1900, localtm.tm_mon + 1, localtm.tm_mday,
			localtm.tm_hour, localtm.tm_min, localtm.tm_sec, microseconds);

		// Return timestamp.
		return timestamp;
		*/

		// Convert now to tm struct for local timezone
		time_t now = time(0);
		//tm * localtm = localtime(&now);
		tm localtm;
		if (localtime_s(&localtm, &now) != 0)
		{
			return Timestamp();
		}

		// Fill timestamp object.

		// Store data for timestamp.
		Timestamp timestamp(localtm.tm_year + 1900, localtm.tm_mon + 1, localtm.tm_mday,
		localtm.tm_hour, localtm.tm_min, localtm.tm_sec, 0);

		// Return timestamp.
		return timestamp;
	}

}