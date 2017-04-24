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

		class Parameter
		{

		public:
			
			enum eType
			{
				None,
				String,
				Integer
			};

			Parameter();

			~Parameter();

			Parameter(const std::string & p_Data);

			eType GetType() const;

			bool Empty() const;
			
			unsigned int AsInteger() const;
			
			const std::string & AsString() const;

		private:

			union Data
			{
				unsigned int _Integer;
				std::string * _String;
			};

			eType m_Type;
			Data m_Data;

		};


		////////////////////////////////////////////////////////////////
		/// \brief Parsing result enum
		///
		////////////////////////////////////////////////////////////////
		enum eResult
		{
			Ok,				///< Parsing were ok.
			ReachedMax,		///< Reached max length.
			ReachedEnd,		///< Parsing were ok, but reached end.
			EndOfBuffer,	///< Failed to parse, position is on end.
			Invalid			///< Failed to parse.
		};

		////////////////////////////////////////////////////////////////
		/// \brief Constructor.
		///
		////////////////////////////////////////////////////////////////
		MessageParser();

		////////////////////////////////////////////////////////////////
		/// \brief Desstructor.
		///
		////////////////////////////////////////////////////////////////
		~MessageParser();

		////////////////////////////////////////////////////////////////
		/// \brief Constructor.
		///
		////////////////////////////////////////////////////////////////
		bool Parse(const char * p_pBuffer, const unsigned int p_BufferSize);

		////////////////////////////////////////////////////////////////
		/// \brief Get command name.
		///
		////////////////////////////////////////////////////////////////
		const std::string & GetCommand() const;

		////////////////////////////////////////////////////////////////
		/// \brief Get parameter.
		///
		////////////////////////////////////////////////////////////////
		const Parameter & GetParameter(const unsigned int p_Index) const;

		////////////////////////////////////////////////////////////////
		/// \brief Get parameter count.
		///
		////////////////////////////////////////////////////////////////
		unsigned int GetParameterCount() const;

		////////////////////////////////////////////////////////////////
		/// \brief Get position of new line
		///
		////////////////////////////////////////////////////////////////
		unsigned int GetNewLinePosition() const;



	private:

		// Private typedefs
		typedef std::vector<Parameter *> Parameters;

		// Private variables
		std::string			m_Command;
		Parameters			m_Parameters;
		static Parameter	m_NoneParameter;	///< Default empty parameter.
		unsigned int		m_NewLinePos;		///< Position of new line

	};

}

#endif