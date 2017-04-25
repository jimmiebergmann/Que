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

#include <Server.hpp>
#include <Log.hpp>
#include <Connection.hpp>
#include <Message.hpp>
#include <MessageParser.hpp>
#include <System/Sleep.hpp>
#include <vector>
#include <algorithm>
#include <iostream>
#include <System/MemoryLeak.hpp>
#include <Network/TcpSocket.hpp>

namespace Que
{
	  
	Server::Server() :
		m_Running(false)
	{

	}

	Server::~Server()
	{
		Stop();
	}

	bool Server::Run(const Settings & p_Settings)
	{
		// Copy the settings
		m_Settings = p_Settings;

		// Error check the settings.
		if (m_Settings.MaxMessageSize == 0)
		{
			QueLogError("Max message cannot be 0.");
			return false;
		}
		if (m_Settings.MaxMessageSize > 1000000000)
		{
			QueLogError("Max message size cannot be larger than 1 GB.");
		}

		// Set running to true.
		m_Running.Set(true);

		QueLogInfo("Started Que server.");

		// Start listening thread.
		m_ListeningThread = std::thread([this]()
		{
			// Start listener.
			try
			{
				m_Listener.Start(m_Settings.ListenPort, m_Settings.ListenAddress);
			}
			catch(std::exception & e)
			{
				QueLogError(e.what());
				return;
			}

			TcpSocket * pSocket = NULL;

			// Main loop of listening thread.
			while (m_Running.Get())
			{
				if (pSocket == NULL)
				{
					pSocket = new TcpSocket;
				}

				bool gotNewConnection = false;

				// Listen for connections.
				try
				{
					gotNewConnection = m_Listener.Listen(*pSocket, Time::Milliseconds(10));
				}
				catch (std::exception & e)
				{
					QueLogInfo(e.what());
					return;
				}
				
				// Listen for incoming connections.
				if(gotNewConnection)
				{
					QueLogInfo("Client connected: " << pSocket->GetPeerAddress().GetPretty() << ":" << pSocket->GetPeerPort());

					// Create new connection
					Connection * pConnection = new Connection(this, pSocket);
					const uint64 connectionId = pConnection->GetId();
					///< Lock connections mutex
					{
						std::lock_guard<std::mutex> lock(m_Connections.Mutex);
						m_Connections.Value.insert(std::pair<uint64, Connection*>(connectionId, pConnection));
						m_Selector.Add(pConnection);
					}
					///< Unlock connections mutex

					// Clear socket, let's allocate a new one one next loop
					pSocket = NULL;
				}
			}

			// Clear left over socket if any.
			if (pSocket)
			{
				delete pSocket;
			}
		});


		// Start connection thread.
		m_ConnectionThread = std::thread([this]()
		{
			const unsigned int bufferSize = GetMaxMessageSize();
			char * pBuffer = new char[bufferSize];

			while (m_Running.Get())
			{
				// Get incoming data.
				ConnectionSelector::ConnectionList selections;
				if (m_Selector.Select(selections, Time::Milliseconds(10)) == false)
				{
					Sleep(Time::Milliseconds(2));
					continue;
				}

				// Go through all selections.
				for (auto sIt = selections.begin(); sIt != selections.end(); sIt++)
				{
					// Get connection
					Connection * pConnection = *sIt;

					// Read from socket.
					TcpSocket & tcpSocket = pConnection->GetSocket();
					int recvSize = tcpSocket.Receive(pBuffer, bufferSize);

					// Close thread if not running anymore.
					if (m_Running.Get() == false)
					{
						break;
					}

					// Check incoming data.

					// Client disconnected.
					if (recvSize == -1)
					{
						Disconnect(pConnection);
						continue;
					}
					else if (recvSize == 0)
					{
						throw std::exception("ConnectionThread: Received 0 data.");
					}

					QueLogInfo("Received from " << pConnection->GetSocket().GetPeerAddress().GetPretty() << ":"
						<< pConnection->GetSocket().GetPeerPort() << ": " << recvSize);


					// Parse message
					MessageParser messageParser;
					if (messageParser.Parse(pBuffer, recvSize) == false)
					{
						RespondeBadFormat(pConnection);
						continue;
					}

					const std::string & command = messageParser.GetCommand();
					const unsigned int paramCount = messageParser.GetParameterCount();
					const unsigned int newLinePos = messageParser.GetNewLinePosition();

					if (command == "PUSH")
					{
						if (pConnection->GetState() != Connection::Idle)
						{
							RespondeBusy(pConnection);
							continue;
						}

						// Expect 3 paramters and integers
						if (paramCount != 1 ||
							messageParser.GetParameter(0).GetType() != MessageParser::Parameter::Integer)
						{
							RespondeBadFormat(pConnection);
							continue;
						}
							
						// Error check message size
						const unsigned int messageSize = messageParser.GetParameter(0).AsInteger();
						const unsigned int actualMessageSize = recvSize - newLinePos - 2;
						if (actualMessageSize != messageSize)
						{
							RespondeBadFormat(pConnection);
							continue;
						}

						// Let's create the message and store it for pulling.
						const char * pMessageData = pBuffer + newLinePos + 1;
						Message * pMessage = new Message(pMessageData, messageSize, pConnection->GetId());

						///< Lock message mutex
						{
							std::lock_guard<std::mutex> lock(m_Messages.Mutex);
							m_Messages.Value.push_back(pMessage);
						}
						///< Unlock connections mutex

						m_MessageSemaphore.Notify();

						pConnection->SetState(Connection::Producing, pMessage);

						// Send no response until consumer ack or, tta is reached.
						continue;
					}
					else if (command == "PULL")
					{
						if (pConnection->GetState() != Connection::Idle)
						{
							RespondeBusy(pConnection);
							continue;
						}

						// Expect no paramters
						if (paramCount != 0)
						{
							RespondeBadFormat(pConnection);
							continue;
						}

						///< Lock message mutex
						{
							std::lock_guard<std::mutex> lock(m_PullQueue.Mutex);
							m_PullQueue.Value.push(pConnection->GetId());
						}
						///< Unlock connections mutex

						pConnection->SetState(Connection::Consuming);
						m_PullSemaphore.Notify();

						// Send no response until a message is available.
						continue;
					}
					else if (command == "ACK")
					{
						Message * pMessage = pConnection->GetMessage();

						if (pConnection->GetState() != Connection::Consuming ||
							pMessage == NULL)
						{
							RespondeIdle(pConnection);
							continue;
						}

						uint64 producerId = pMessage->GetProdcer();

						// Expect 1 paramter, and integer
						if (paramCount != 1 ||
							messageParser.GetParameter(0).GetType() != MessageParser::Parameter::Integer)
						{
							RespondeBadFormat(pConnection);
							continue;
						}

						const unsigned int messageSize = messageParser.GetParameter(0).AsInteger();

						// Error check message size
						const unsigned int actualMessageSize = recvSize - newLinePos - 2;
						if (actualMessageSize != messageSize)
						{
							RespondeBadFormat(pConnection);
							continue;
						}

						// Get ack message
						const char * pMessageData = pBuffer + newLinePos + 1;

						// Get producer
						Connection * pProducer = NULL;

						///< Lock connections mutex
						{
							std::lock_guard<std::mutex> lock3(m_Connections.Mutex);
							ConnectionMap::iterator cIt = m_Connections.Value.find(producerId);
							if (cIt == m_Connections.Value.end())
							{
								// Producer disconnected.
								// Delete message.
								pConnection->SetState(Connection::Idle, NULL);
								delete pMessage;

								continue;
							}

							// Try to send the message to the pulling client.
							pProducer = cIt->second;
						}
						///< Unlock connections mutex
						
						// Send ack message to producer.
						Responde(pProducer, pBuffer, recvSize);

						// Delete message.
						pProducer->SetState(Connection::Idle, NULL);
						pConnection->SetState(Connection::Idle, NULL);
						delete pMessage;

						continue;
					}
					else if (command == "ABORT")
					{
						// Expect no paramters
						if (paramCount != 0)
						{
							RespondeBadFormat(pConnection);
							continue;
						}

						// Ignore idle states
						const Connection::eState eConState = pConnection->GetState();
						if (eConState == Connection::Idle)
						{
							RespondeIdle(pConnection);
							continue;
						}

						Message * pMessage = pConnection->GetMessage();

						// Check if connection is producting
						if (eConState == Connection::Producing)
						{
							// Get message and remove it from consumer.
							const uint64 consumer = pMessage->GetConsumer();
							if (consumer != 0)
							{
								///< Lock connections mutex
								{
									std::lock_guard<std::mutex> lock3(m_Connections.Mutex);
									ConnectionMap::iterator cIt = m_Connections.Value.find(consumer);
									if (cIt != m_Connections.Value.end())
									{
										cIt->second->SetState(Connection::Idle);
									}
								}
								///< Unlock connections mutex
							}

							// Delete message
							pConnection->SetState(Connection::Idle);
							delete pMessage;
						}
						else if (eConState == Connection::Consuming)
						{
							pConnection->SetState(Connection::Idle);

							// Add message back in queue.
							///< Lock message mutex
							{
								std::lock_guard<std::mutex> lock(m_Messages.Mutex);
								m_Messages.Value.push_front(pMessage);
							}
							///< Unlock connections mutex
						}

						continue;
					}
					else
					{
						RespondeUnkownCommand(pConnection);
						continue;
					}
				}				
			}

			// Clean up the buffer
			delete pBuffer;
		});


		// Start message thread.
		m_MessageThread = std::thread([this]()
		{
			while (m_Running.Get())
			{
				m_MessageSemaphore.Wait();
				m_PullSemaphore.Wait();

				// Exit if we are not running anymore.
				if (m_Running.Get() == false)
				{
					return;
				}

				///< Lock message mutex
				{
					std::lock_guard<std::mutex> lock1(m_Messages.Mutex);

					// No messages, skip.
					if (m_Messages.Value.size() == 0)
					{
						m_PullSemaphore.Notify();
						continue;
					}

					///< Lock pull mutex
					{
						std::lock_guard<std::mutex> lock2(m_PullQueue.Mutex);

						// No pullers, skip.
						if (m_PullQueue.Value.size() == 0)
						{
							m_MessageSemaphore.Notify();
							continue;
						}

						// Get message
						Message * pMessage = m_Messages.Value.front();

						// Get consumer
						const uint64 consumerId = m_PullQueue.Value.front();

						// Get connection
						///< Lock connections mutex
						{
							std::lock_guard<std::mutex> lock3(m_Connections.Mutex);

							// Get producer, ignore message if disconnected.
							ConnectionMap::iterator pIt = m_Connections.Value.find(pMessage->GetProdcer());
							if (pIt == m_Connections.Value.end())
							{
								delete pMessage;

								// Pop message from queue
								m_Messages.Value.pop_front();

								m_PullSemaphore.Notify();
								continue;
							}

							// Now pop pulling queue, the message is valid, but is the consumer?
							m_PullQueue.Value.pop();

							// Get consumer, ignore disconnected consumers.
							ConnectionMap::iterator cIt = m_Connections.Value.find(consumerId);
							if (cIt == m_Connections.Value.end())
							{
								m_MessageSemaphore.Notify();
								continue;
							}

							// Try to send the message to the pulling client.
							Connection * pConnection = cIt->second;
							if (pConnection->GetSocket().Send(pMessage->Get<char*>(), pMessage->GetSize()) != pMessage->GetSize())
							{
								Disconnect(pConnection);
								m_MessageSemaphore.Notify();
								continue;
							}
							
							// Set message in order to know which message consumer is supposed to ack.
							pConnection->SetState(Connection::Consuming, pMessage);
							pMessage->SetConsumer(pConnection->GetId());
						}
						///< Unlock connections mutex

						// Pop message from queue
						m_Messages.Value.pop_front();
					}
					///< Unlock pull mutex
				}
				///< Unlock message mutex


				/// TEMPORARY SLEEP.
				Sleep(Time::Milliseconds(10));
			}
		});


		return true;
	}

	void Server::Stop()
	{
		// Do never stop twice.
		if (m_Running.Get() == false)
		{
			return;
		}

		// Set running to false, in order to close all threads.
		m_Running.Set(false);

		m_MessageSemaphore.Notify();
		m_PullSemaphore.Notify();

		// Stop threads.
		m_ListeningThread.join();
		m_ConnectionThread.join();
		m_MessageThread.join();

		// Delete connections.
		for (auto cIt = m_Connections.Value.begin(); cIt != m_Connections.Value.end(); cIt++)
		{
			delete cIt->second;
		}
		m_Connections.Value.clear();

		// Delete messages.
		for (auto mIt = m_Messages.Value.begin(); mIt != m_Messages.Value.end(); mIt++)
		{
			delete *mIt;
		}
		m_Messages.Value.clear();
	}

	bool Server::IsRunning()
	{
		return m_Running.Get();
	}


	unsigned int Server::GetMaxMessageSize() const
	{
		return m_Settings.MaxMessageSize;
	}

	void Server::Disconnect(Connection * p_pConnection)
	{
		///< Lock connections mutex
		std::lock_guard<std::mutex> lock(m_Connections.Mutex);
		{

			auto cIt = m_Connections.Value.find(p_pConnection->GetId());
			if (cIt == m_Connections.Value.end())
			{
				throw std::exception("Server - Disconnect: Cannot find connection for some reason.");
			}

			// Handle messages.
			Message * pMessage = p_pConnection->GetMessage();
			if (pMessage)
			{
				if (p_pConnection->GetState() == Connection::Producing)
				{
					const uint64 consumerId = pMessage->GetConsumer();
					if (consumerId != 0)
					{
						auto cIt2 = m_Connections.Value.find(consumerId);
						if (cIt2 != m_Connections.Value.end())
						{
							cIt2->second->SetState(Connection::Idle);
						}

						///< Lock message mutex
						{
							std::lock_guard<std::mutex> lock1(m_Messages.Mutex);
							delete pMessage;
						}
						///< Unlock message mutex
					}

				}
				else if (p_pConnection->GetState() == Connection::Consuming)
				{
					///< Lock message mutex
					{
						std::lock_guard<std::mutex> lock1(m_Messages.Mutex);
						pMessage->SetConsumer(0);
						m_Messages.Value.push_front(pMessage);
					}
					///< Unlock message mutex
				}
			}


			m_Selector.Remove(p_pConnection);
			m_Connections.Value.erase(cIt);

			QueLogInfo("Client disconnected: " << p_pConnection->GetSocket().GetPeerAddress().GetPretty()
				<< ":" << p_pConnection->GetSocket().GetPeerPort());

			delete p_pConnection;

		}
		///< Unlock connections mutex		
	}

	bool Server::Responde(Connection * p_pConnection, const char * p_pVoid , const unsigned int p_DataSize)
	{
		if (p_pConnection->GetSocket().Send(p_pVoid, p_DataSize) != p_DataSize)
		{
			Disconnect(p_pConnection);
			return false;
		}

		return true;
	}

	bool Server::Responde(Connection * p_pConnection, const std::string & p_Message)
	{
		if (p_pConnection->GetSocket().Send(p_Message.c_str(), p_Message.size()) != p_Message.size())
		{
			Disconnect(p_pConnection);
			return false;
		}

		return true;
	}

	bool Server::RespondeUnkownCommand(Connection * p_pConnection)
	{
		return Responde(p_pConnection, "UNKOWN_COMMAND\n");
	}

	bool Server::RespondeBadFormat(Connection * p_pConnection)
	{
		return Responde(p_pConnection, "BAD_FORMAT\n");
	}

	bool Server::RespondeMessageTooBig(Connection * p_pConnection)
	{
		return Responde(p_pConnection, "MESSAGE_TOO_BIG\n");
	}	

	bool Server::RespondeBusy(Connection * p_pConnection)
	{
		return Responde(p_pConnection, "BUSY\n");
	}

	bool Server::RespondeIdle(Connection * p_pConnection)
	{
		return Responde(p_pConnection, "IDLE\n");
	}

}