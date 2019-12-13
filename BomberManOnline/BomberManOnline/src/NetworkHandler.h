#pragma once

#include <SFML/Network.hpp>
#include "ServerMessageType.h"
#include "NonCopyable.h"
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <memory>

class NetworkHandler : private NonCopyable
{
public:
	static NetworkHandler& getInstance()
	{
		static NetworkHandler instance;
		return instance;
	}

	bool isReceivedPackets() const;
	sf::Packet getLatestPacket();

	bool connectToServer();
	void disconnectFromServer();
	void sendMessageToServer(sf::Packet& packetToSend);

private:
	NetworkHandler();
	std::queue<sf::Packet> m_receivedPackets;
	std::unique_ptr<sf::TcpSocket> m_tcpSocket;
	std::atomic<bool> m_connectedToServer;
	std::thread m_listenThread;
	std::mutex m_mutex;

	void listen();
};