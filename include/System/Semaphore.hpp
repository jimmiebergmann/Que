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

#ifndef QUE_SYSTEM_SEMAPHORE_HPP
#define QUE_SYSTEM_SEMAPHORE_HPP

#ifndef QUE_PLATFORM_WINDOWS
#error Not supporting this platform.
#endif

#include <Win32/Windows.hpp>
#include <System/Time.hpp>

namespace Que
{

	class Semaphore
	{

	public:

		Semaphore::Semaphore()
		{
			m_Handle = CreateSemaphore(NULL, 0, 2147483647, NULL);
		}

		Semaphore::~Semaphore()
		{
			CloseHandle(m_Handle);
		}

		bool Semaphore::Wait(const Time & p_Timeout = Time::Infinite)
		{
			
			// Do not timeout
			if (p_Timeout == Time::Infinite)
			{
				WaitForSingleObject(m_Handle, INFINITE);
				return true;
			}
			
			const DWORD time = static_cast<DWORD>(p_Timeout.AsMilliseconds());

			// Wait for the sempahore to release, with timeout
			DWORD ret = WaitForSingleObject(m_Handle, time);

			if (ret == WAIT_TIMEOUT)
			{
				return false;
			}

			return true;
		}

		void Semaphore::Notify()
		{
			ReleaseSemaphore(m_Handle, 1, NULL);
		}

	private:

		HANDLE m_Handle;

	};

}

#endif


