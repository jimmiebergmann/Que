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

#ifndef QUE_SYSTEM_FILE_WIN32_HPP
#define QUE_SYSTEM_FILE_WIN32_HPP

#include <Build.hpp>
#ifdef QUE_PLATFORM_WINDOWS
#include <string>

namespace Que
{

	////////////////////////////////////////////////////////////////
	/// \ingroup Network
	/// \brief Win32 socket class.
	///
	////////////////////////////////////////////////////////////////
	class FileWin32
	{

	public:

		////////////////////////////////////////////////////////////////
		/// \brief Default constructor
		///
		////////////////////////////////////////////////////////////////
		/*FileWin32();

		////////////////////////////////////////////////////////////////
		/// \brief Virtual destructor
		///
		////////////////////////////////////////////////////////////////
		~FileWin32();*/

		////////////////////////////////////////////////////////////////
		/// \brief Check if path is directory and exists.
		///
		////////////////////////////////////////////////////////////////
		static bool IsDir(const std::string & p_Dir);

		static bool CreateDir(const std::string & p_Dir);


	protected:


	};

}

#endif

#endif
