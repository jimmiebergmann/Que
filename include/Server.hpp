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

/*

'*' = Available
'-' = Not Implemented.

Command:

- PUSH <ttl> <tta> <message size>\n<message>\n		- Push job.
- PULL\n											- Pull job.
- ACK <message_size>\n<message>\n 					- Acknowledge pulled message.
- ABORT\n											- Abort PUSH or PULL.

*/


#ifndef QUE_SERVER_HPP
#define QUE_SERVER_HPP

#include <Build.hpp>
#include <System/Semaphore.hpp>
#include <System/ThreadValue.hpp>
#include <Network/TcpListener.hpp>
#include <ConnectionSelector.hpp>
#include <Settings.hpp>
#include <Message.hpp>
#include <string>
#include <thread>
#include <map>
#include <list>
#include <queue>

namespace Que
{

	// Forward declarations
	class Connection;

	////////////////////////////////////////////////////////////////
	/// \brief Message and job distribution queue in one, 
	///			using simple commands over TCP. 
	///
	/// All commands are handled by the Connection class.
	///
	/// \see Connection
	///
	////////////////////////////////////////////////////////////////
	class Server
	{

	public:

		////////////////////////////////////////////////////////////////
		/// \brief Default constructor.
		///
		////////////////////////////////////////////////////////////////
		Server();

		////////////////////////////////////////////////////////////////
		/// \brief Destructor.
		///
		////////////////////////////////////////////////////////////////
		~Server();

		////////////////////////////////////////////////////////////////
		/// \brief Start the queue server.
		///
		/// \param p_Settings Server settings container.
		///
		////////////////////////////////////////////////////////////////
		bool Run(const Settings & p_Settings);

		////////////////////////////////////////////////////////////////
		/// \brief Stop the queue server.
		///
		////////////////////////////////////////////////////////////////
		void Stop();

		////////////////////////////////////////////////////////////////
		/// \brief Check if server is running.
		///
		////////////////////////////////////////////////////////////////
		bool IsRunning();

		////////////////////////////////////////////////////////////////
		/// \brief Get maximum receive buffer size in one call.
		///
		////////////////////////////////////////////////////////////////
		unsigned int GetMaxMessageSize() const;

	private:

		// Private functions

		////////////////////////////////////////////////////////////////
		/// \brief Disconnect client from server
		///			Will turn the input pointer invalid.
		///
		/// \throw std::exception if connection cannot be found.
		///
		////////////////////////////////////////////////////////////////
		void Disconnect(Connection * p_pConnection);

		////////////////////////////////////////////////////////////////
		/// \brief Send response to client.
		///			Will internally disconnect clinet if needed.
		///
		////////////////////////////////////////////////////////////////
		bool Responde(Connection * p_pConnection, const std::string & p_Message);

		////////////////////////////////////////////////////////////////
		/// \brief Send response to client.
		///			Will internally disconnect clinet if needed.
		///
		////////////////////////////////////////////////////////////////
		bool Responde(Connection * p_pConnection, const char * p_pVoid, const unsigned int p_DataSize);
		
		////////////////////////////////////////////////////////////////
		/// \brief Send unknown command respoonse.
		///			Internally calls Responde.
		///
		/// \see Responde
		///
		////////////////////////////////////////////////////////////////
		bool RespondeUnkownCommand(Connection * p_pConnection);

		////////////////////////////////////////////////////////////////
		/// \brief Send bad format respoonse.
		///			Internally calls Responde.
		///
		/// \see Responde
		///
		////////////////////////////////////////////////////////////////
		bool RespondeBadFormat(Connection * p_pConnection);

		////////////////////////////////////////////////////////////////
		/// \brief Send message too big respoonse.
		///			Internally calls Responde.
		///
		/// \see Responde
		///
		////////////////////////////////////////////////////////////////
		bool RespondeMessageTooBig(Connection * p_pConnection);

		////////////////////////////////////////////////////////////////
		/// \brief Send busy respoonse.
		///			Internally calls Responde.
		///
		/// \see Responde
		///
		////////////////////////////////////////////////////////////////
		bool RespondeBusy(Connection * p_pConnection);

		////////////////////////////////////////////////////////////////
		/// \brief Send idle response.
		///			Internally calls Responde.
		///
		/// \see Responde
		///
		////////////////////////////////////////////////////////////////
		bool RespondeIdle(Connection * p_pConnection);

		////////////////////////////////////////////////////////////////
		/// \brief Open log.
		///
		/// \see Settings
		///
		////////////////////////////////////////////////////////////////
		bool OpenLog(const std::string & p_Path);


		// Private typedefs
		typedef std::map<uint64, Connection * > ConnectionMap;
		typedef std::list<Message * >			MessageList;
		typedef std::queue<uint64>				PullQueue;

		// Private variables
		ThreadValue<bool>				m_Running;				///< Indicates if the server is running.
		Settings						m_Settings;				///< Strucutre full of server settings.	
		std::thread						m_ListeningThread;		///< Thread for listenting for connections.
		std::thread						m_ConnectionThread;		///< Mail thread of incoming messages.
		std::thread						m_MessageThread;		///< Thread to handle messages.
		std::thread						m_TimeToLiveThread;		///< Thread to handle job's time to live.
		std::thread						m_TimeToAckThread;		///< Thread to handle job's time to ack.
		TcpListener						m_Listener;				///< Listener of incoming connections.
		ConnectionSelector				m_Selector;				///< Selector class for incoming messages/disconnections.
		ThreadValue<ConnectionMap>		m_Connections;			///< Map of all connections.
		ThreadValue<MessageList>		m_Messages;				///< List of messages waiting for being pulled.
		Semaphore						m_MessageSemaphore;		///< Semaphore to notify if a new message is available.
		ThreadValue<PullQueue>			m_PullQueue;			///< Queue of connections pulling.
		Semaphore						m_PullSemaphore;		///< Semaphore to notify if a consumer pull.














/*
		////////////////////////////////////////////////////////////////
		/// \brief Default constructor.
		///
		////////////////////////////////////////////////////////////////
		Server();

		////////////////////////////////////////////////////////////////
		/// \brief Destructor.
		///
		////////////////////////////////////////////////////////////////
		~Server();

		////////////////////////////////////////////////////////////////
		/// \brief Start the queue server.
		///
		/// \param p_Settings Server settings container.
		///
		////////////////////////////////////////////////////////////////
		bool Run(const Settings & p_Settings);

		////////////////////////////////////////////////////////////////
		/// \brief Stop the queue server.
		///
		////////////////////////////////////////////////////////////////
		void Stop();

		////////////////////////////////////////////////////////////////
		/// \brief Check if server is running.
		///
		////////////////////////////////////////////////////////////////
		bool IsRunning();

		////////////////////////////////////////////////////////////////
		/// \brief Close connection by given id.
		///
		////////////////////////////////////////////////////////////////
		void CloseConnection(const uint64 & p_Id);

		////////////////////////////////////////////////////////////////
		/// \brief Push job to given tube.
		///
		/// \return Pointer to job if succeeded, else NULL.
		///
		////////////////////////////////////////////////////////////////
		Job * PushJob(	const uint64 p_ProcuerId,
						const std::string & p_Tube,
						const void * p_pMessage,
						const unsigned int p_MessageSize,
						const unsigned int p_Priority,
						const Time & p_Delay,
						const Time & p_TimeToRun,
						const Time & p_TimeToLive);

		////////////////////////////////////////////////////////////////
		/// \brief Push job to original tube owner.
		///
		/// \param p_pJob Pointer to job.
		///
		/// \throw std::invalid_argument If p_pJob is NULL.
		/// \return true of succeeded, else false.
		///
		////////////////////////////////////////////////////////////////
		void PushJobAgain(Job * p_pJob);

		////////////////////////////////////////////////////////////////
		/// \brief Deliver job to consumer(connection).
		///
		/// \param p_pJob Pointer to job.
		/// \param p_Id Connection id.
		/// \param p_pTube Pointer to tube, who deliver the work.
		///
		/// \throw std::invalid_argument If p_pJob is NULL.
		/// \return true of succeeded, else false.
		///
		////////////////////////////////////////////////////////////////
		bool DeliverJob(Job * p_pJob, const uint64 p_Id, Tube * p_pTube);

		////////////////////////////////////////////////////////////////
		/// \brief Get or create new tube by name.
		///
		////////////////////////////////////////////////////////////////
		Tube * GetTube(const std::string & p_Name);

	private:

		// Private functions

		////////////////////////////////////////////////////////////////
		/// \brief Open log. Path to log file is provided by settings class.
		///
		/// \see Settings
		///
		////////////////////////////////////////////////////////////////
		bool OpenLog();

		////////////////////////////////////////////////////////////////
		/// \brief Clean up function run by Run function after terminating.
		///
		/// \see Run
		///
		////////////////////////////////////////////////////////////////
		void CleanUp();

		// Private typedefs
		typedef std::map<uint64, Connection *> ConnectionMap;
		typedef std::queue<uint64> DisconnectQueue;
		typedef std::map<std::string, Tube *> TubeMap;
		typedef std::queue<Job *> JobQueue;

		// Private variables
		Settings						m_Settings;					///< Container of server settings.
		ThreadValue<bool>				m_Running;					///< Flag for checking if server is running.
		ThreadValue<bool>				m_Termianted;				///< Flag for checking if Run should terminate.
		TcpListener						m_Listener;					///< Tcp listener object.
		std::thread						m_ConnectThread;			///< Thread for listening for connections
		std::thread						m_DisconnectThread;			///< Thread for cleaning up connections
		Semaphore						m_DisconnectSemaphore;		///< Semaphore for firing disconnect cleanup in disconnect thread.
		ThreadValue<ConnectionMap>		m_Connections;				///< Map of all connections.
		ThreadValue<DisconnectQueue>	m_DisconnectQueue;			///< Queue of connections to disconnect.
		ThreadValue<TubeMap>			m_Tubes;					///< Map of tubes.
		uint64							m_LastConnnectionId;		///< Last connection id in use.
		ThreadValue<uint64>				m_LastJobNo;				///< Last job id in use.

		std::thread						m_PushJobAgainThread;		///< Thread for pushing jobs again.
		ThreadValue<JobQueue>			m_PushJobAgainQueue;		///< List of jobs to get pushed again by push thread.
		Semaphore						m_PushJobAgainSemaphore;	///< Semaphore to tell thread a new job is ready to be pushed to tube.

		*/
	};

}

#endif