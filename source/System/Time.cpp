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

#include <System/Time.hpp>
#include <ctime>
#ifdef QUE_PLATFORM_WINDOWS
#include <Win32/Windows.hpp>
#endif
#include <System/MemoryLeak.hpp>

namespace Que
{

	const Time Time::Infinite = Microseconds(0xFFFFFFFFFFFFFFFFULL);
	const Time Time::Zero = Microseconds(0);


	// Functions for initializing time classes
	Time Time::Seconds(const double & p_Seconds)
	{
		return Time(static_cast<uint64>(p_Seconds * 1000000.0f));
	}

	Time Time::Milliseconds(const uint64 & p_Milliseconds)
	{
		return Time(p_Milliseconds * 1000ULL);
	}

	Time Time::Microseconds(const uint64 & p_Microseconds)
	{
		return Time(p_Microseconds);
	}
	Time Time::GetSystemTime()
	{
		// Windows implementation.
	#ifdef QUE_PLATFORM_WINDOWS

		int64 counter = 0;
		int64 frequency = 0;

		QueryPerformanceCounter((LARGE_INTEGER*)&counter);
		QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);

		double timeFloat = (static_cast<double>(counter) * 1000000.0f) / static_cast<double>(frequency);
	
		return Microseconds(static_cast<uint64>(timeFloat));

		// Linux implementation.
	#elif defined( QUE_PLATFORM_LINUX )
	#error Invalid platform.

		timeval Time;
		gettimeofday(&Time, 0);

		return	static_cast< Int64 >(Time.tv_sec) * 1000000ULL +
			static_cast< Int64 >(Time.tv_usec);

	#endif
	}




	Time::Time() :
		m_Microseconds(0)
	{
	}

	double Time::AsSeconds() const
	{
		return static_cast<double>(m_Microseconds) / 1000000.0f;
	}

	uint64 Time::AsMilliseconds() const
	{
		return m_Microseconds / 1000ULL;
	}

	uint64 Time::AsMicroseconds() const
	{
		return m_Microseconds;
	}

	bool Time::operator == (const Time & p_Time) const
	{
		return m_Microseconds == p_Time.m_Microseconds;
	}

	bool Time::operator != (const Time & p_Time) const
	{
		return m_Microseconds != p_Time.m_Microseconds;
	}

	bool Time::operator > (const Time & p_Time) const
	{
		return m_Microseconds > p_Time.m_Microseconds;
	}

	bool Time::operator < (const Time & p_Time) const
	{
		return m_Microseconds < p_Time.m_Microseconds;
	}

	bool Time::operator >= (const Time & p_Time) const
	{
		return m_Microseconds >= p_Time.m_Microseconds;
	}

	bool Time::operator <= (const Time & p_Time) const
	{
		return m_Microseconds <= p_Time.m_Microseconds;
	}

	Time Time::operator + (const Time & p_Time) const
	{
		return Time(m_Microseconds + p_Time.m_Microseconds);
	}

	Time & Time::operator += (const Time & p_Time)
	{
		m_Microseconds += p_Time.m_Microseconds;
		return *this;
	}

	Time Time::operator - (const Time & p_Time) const
	{
		if (p_Time.m_Microseconds > m_Microseconds)
		{
			return Time(0ULL);
		}

		return Time(m_Microseconds - p_Time.m_Microseconds);
	}

	Time Time::operator * (const uint64 & p_Value) const
	{
		return Time(m_Microseconds * p_Value);
	}

	Time Time::operator / (const uint64 & p_Value) const
	{
		return Time(m_Microseconds / p_Value);
	}

	Time Time::operator % (const Time & p_Time) const
	{
		return Time(m_Microseconds % p_Time.m_Microseconds);
	}

	Time::operator bool () const
	{
		return m_Microseconds != 0ULL;
	}

	// Private functions
	Time::Time(const uint64 & p_Microseconds) :
		m_Microseconds(p_Microseconds)
	{
	}

}