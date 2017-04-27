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
					int recvSize = 0;
					Connection * pConnection = NULL;
					uint64 connectionId = 0;

					///< Lock connections mutex
					{
						std::lock_guard<std::mutex> lock(m_Connections.Mutex);

						// Get connection
						pConnection = *sIt;
						connectionId = pConnection->GetId();

						// Read from socket.
						TcpSocket & tcpSocket = pConnection->GetSocket();
						recvSize = tcpSocket.Receive(pBuffer, bufferSize);
					}
					///< Unlock connections mutex

					// Close thread if not running anymore.
					if (m_Running.Get() == false)
					{
						break;
					}

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

					QueLogInfo("Received data: " << recvSize);


					ReceivedData * pRecvData = new ReceivedData(pBuffer, recvSize);

					///< Lock received data mutex
					{
						std::lock_guard<std::mutex> lock(m_ReceivedData.Mutex);

						// See if connection exists in the data map.
						ConnectionDataMap::iterator it = m_ReceivedData.Value.find(connectionId);
						
						// create connection data.
						if (it == m_ReceivedData.Value.end())
						{
							m_ReceivedData.Value.insert(std::pair<uint64, ReceivedDataQueue*>(connectionId, new ReceivedDataQueue));
							it = m_ReceivedData.Value.find(connectionId);
						}

						// Add data to queue.
						it->second->push(pRecvData);

						m_ReceivedDataSemaphore.Notify();
					}
					///< Unlock received data mutex
				}
			}

			// Clean up the buffer
			delete pBuffer;
		});

		m_ReceivedDataThread = std::thread([this]()
		{
			while (m_Running.Get())
			{
				// Wait for data to handle.
				m_ReceivedDataSemaphore.Wait();

				///< Lock received data mutex
				{
					std::lock_guard<std::mutex> lock(m_ReceivedData.Mutex);

					// Go through all connection in data map
					for (auto cIt = m_ReceivedData.Value.begin(); cIt != m_ReceivedData.Value.end();)
					{
						const uint64 connectionId = cIt->first;
						bool disconnected = false;

						// Error check queue size
						if (cIt->second->size() == 0)
						{
							throw std::exception("ReceivedDataThread: Connection data queue is empty.");
						}

						// Get next data in queue
						ReceivedData * pData = cIt->second->front();

						// Parse message
						unsigned int bufferSize = 0;
						char * pBuffer = pData->GetData(bufferSize);


						// Run a do while loop once, in order to be able to break it if an error occured.
						enum eErrorType
						{
							ErrorNone,
							ErrorBadFormat,
							ErrorBusy,
							ErrorIdle
						} error = ErrorNone;

						MessageParser messageParser(pBuffer, bufferSize);

						do
						{
							if (messageParser.Parse() == false)
							{
								error = ErrorBadFormat;
								break;
							}

							const std::string & command = messageParser.GetCommand();

							QueLogInfo("Client sent command: " << command);

							if (command == "PUSH")
							{
								///< Lock connections mutex
								{
									std::lock_guard<std::mutex> lock(m_Connections.Mutex);

									ConnectionMap::iterator conIt = m_Connections.Value.find(connectionId);
									if (conIt != m_Connections.Value.end())
									{
										Connection * pConnection = conIt->second;

										if (pConnection->GetState() == Connection::Idle)
										{
											// Let's create the message and store it for pulling.
											unsigned int messageSize = 0;
											char * pMessageBuffer = messageParser.GetMessage(messageSize);
											Message * pMessage = new Message(pMessageBuffer, messageSize, pConnection->GetId());

											///< Lock message mutex
											{
												std::lock_guard<std::mutex> lock(m_Messages.Mutex);
												m_Messages.Value.push_back(pMessage);
											}
											///< Unlock connections mutex

											m_MessageSemaphore.Notify();

											pConnection->SetState(Connection::Producing, pMessage);

										}
										else
										{
											error = ErrorBusy;
										}
									}
									else
									{
										disconnected = true;
									}
								}
								///< Unlock connections mutex
							}
							else if (command == "PULL")
							{
								///< Lock connections mutex
								{
									std::lock_guard<std::mutex> lock(m_Connections.Mutex);

									ConnectionMap::iterator conIt = m_Connections.Value.find(connectionId);
									if (conIt != m_Connections.Value.end())
									{
										Connection * pConnection = conIt->second;

										if (pConnection->GetState() == Connection::Idle)
										{
											///< Lock pull queue mutex
											{
												std::lock_guard<std::mutex> lock(m_PullQueue.Mutex);
												m_PullQueue.Value.push(pConnection->GetId());
											}
											///< Unlock pull queue mutex

											pConnection->SetState(Connection::Consuming);
											m_PullSemaphore.Notify();
										}
										else
										{
											error = ErrorBusy;
										}
									}
									else
									{
										disconnected = true;
									}
								}
								///< Unlock connections mutex

							}
							else if (command == "ABORT")
							{
								///< Lock connections mutex
								{
									std::lock_guard<std::mutex> lock(m_Connections.Mutex);

									ConnectionMap::iterator conIt = m_Connections.Value.find(connectionId);
									if (conIt != m_Connections.Value.end())
									{
										Connection * pConnection = conIt->second;

										// Ignore idle states
										const Connection::eState eConState = pConnection->GetState();
										if (eConState != Connection::Idle)
										{
											Message * pMessage = pConnection->GetMessage();

											// Check if connection is producting
											if (eConState == Connection::Producing)
											{
												// Get message and remove it from consumer.
												const uint64 consumer = pMessage->GetConsumer();
												if (consumer != 0)
												{
													ConnectionMap::iterator cIt = m_Connections.Value.find(consumer);
													if (cIt != m_Connections.Value.end())
													{
														cIt->second->SetState(Connection::Idle);
													}
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
										}
										else
										{
											error = ErrorIdle;
										}
									}
									else
									{
										disconnected = true;
									}
								}
								///< Unlock connections mutex
							}
							else if (command == "ACK")
							{
								///< Lock connections mutex
								{
									std::lock_guard<std::mutex> lock(m_Connections.Mutex);

									ConnectionMap::iterator conIt = m_Connections.Value.find(connectionId);
									if (conIt != m_Connections.Value.end())
									{
										Connection * pConnection = conIt->second;

										Message * pMessage = pConnection->GetMessage();

										if (pConnection->GetState() == Connection::Consuming &&
											pMessage != NULL)
										{
											uint64 producerId = pMessage->GetProdcer();

											unsigned int messageSize = 0;
											char * pMessageBuffer = messageParser.GetMessage(messageSize);

											// Get producer
											Connection * pProducer = NULL;
											ConnectionMap::iterator cIt = m_Connections.Value.find(producerId);
											if (cIt == m_Connections.Value.end())
											{
												// Producer disconnected.
												// Delete message.
												pConnection->SetState(Connection::Idle, NULL);
												delete pMessage;
											}
											else
											{
												// Try to send the message to the pulling client.
												pProducer = cIt->second;


												// Send ack message to producer.
												Responde(pProducer, pMessageBuffer, messageSize);

												// Delete message.
												pProducer->SetState(Connection::Idle, NULL);
												pConnection->SetState(Connection::Idle, NULL);
												delete pMessage;
											}
										}
										else
										{
											error = ErrorIdle;
										}
									}
									else
									{
										disconnected = true;
									}
								}
								///< Unlock connections mutex
							}



						} while (false);

						// Check if error occured.

						///< Lock connections mutex
						{
							std::lock_guard<std::mutex> lock(m_Connections.Mutex);

							ConnectionMap::iterator conIt = m_Connections.Value.find(connectionId);
							if (conIt != m_Connections.Value.end())
							{
								Connection * pConnection = conIt->second;
								if (error != ErrorNone)
								{
									if (error == ErrorBadFormat)
									{
										RespondeBadFormat(pConnection);
									}
									else if (error == ErrorIdle)
									{
										RespondeIdle(pConnection);
									}
									else if (error == ErrorBusy)
									{
										RespondeBusy(pConnection);
									}
								}
							}
							else
							{
								disconnected = true;
							}

						}
						///< Unlock connections mutex

						// Move to next received data or remove.
						// Remove if disconnected.
						if (disconnected || messageParser.GetNextCommandPosition() >= bufferSize)
						{
							// Delete message.
							delete pData;
							cIt->second->pop();

							if (disconnected || cIt->second->size() == 0)
							{
								// Go through queue
								while (cIt->second->size())
								{
									// Delete received data.
									ReceivedData * pReceivedData = cIt->second->front();
									cIt->second->pop();
									delete pReceivedData;
								}

								// Delete queue.
								delete cIt->second;

								m_ReceivedData.Value.erase(cIt++);
								continue;
							}
						}
						else
						{
							pData->MovePosition(messageParser.GetNextCommandPosition());
							m_ReceivedDataSemaphore.Notify();
						}

						// Go to next connection
						++cIt;
					}

				}
				///< Unlock received data mutex

				QueLogInfo("handle message!");
			}
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
		m_ReceivedDataSemaphore.Notify();

		// Stop threads.
		m_ListeningThread.join();
		m_ConnectionThread.join();
		m_MessageThread.join();
		m_ReceivedDataThread.join();

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

		// Delete data in queue
		for (auto dIt = m_ReceivedData.Value.begin(); dIt != m_ReceivedData.Value.end(); dIt++)
		{
			// Go through queue
			while (dIt->second->size())
			{
				// Delete received data.
				ReceivedData * pReceivedData = dIt->second->front();
				dIt->second->pop();
				delete pReceivedData;
			}

			// Delete queue.
			delete dIt->second;
		}
		m_ReceivedData.Value.clear();
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
			return false;
		}

		return true;
	}

	bool Server::Responde(Connection * p_pConnection, const std::string & p_Message)
	{
		if (p_pConnection->GetSocket().Send(p_Message.c_str(), p_Message.size()) != p_Message.size())
		{
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


	Server::ReceivedData::ReceivedData(const char * p_RecvBuffer, const unsigned int p_RecvSize) :
		m_CurrentPosition(0)
	{
		if (p_RecvBuffer == NULL)
		{
			throw std::exception("Server - ReceivedData::ReceivedData: Buffer is NULL.");
		}

		if (p_RecvSize == 0)
		{
			throw std::exception("Server - ReceivedData::ReceivedData: p_RecvSize is 0.");
		}

		m_BufferSize = p_RecvSize;
		m_pBuffer = new char[m_BufferSize];
		memcpy(m_pBuffer, p_RecvBuffer, m_BufferSize);
	}

	Server::ReceivedData::~ReceivedData()
	{
		delete[] m_pBuffer;
	}

	char * Server::ReceivedData::GetData(unsigned int & p_RemainingSize) const
	{
		p_RemainingSize = m_BufferSize - m_CurrentPosition;
		return m_pBuffer + m_CurrentPosition;
	}

	void Server::ReceivedData::MovePosition(const unsigned int p_Count)
	{
		m_CurrentPosition += p_Count;
	}

	bool Server::ReceivedData::IsFinished(const unsigned int p_Position) const
	{
		return m_CurrentPosition >= m_BufferSize;
	}

} 