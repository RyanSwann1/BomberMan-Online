#pragma once

#include <SFML/Network.hpp>
#include "ServerMessageType.h"
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <memory>

class NetworkHandler
{
public:
	static NetworkHandler& getInstance()
	{
		static NetworkHandler instance;
		return instance;
	}

	std::vector<sf::Packet>& getNetworkMessages();

	bool connectToServer();
	void disconnectFromServer();
	void sendMessageToServer(sf::Packet& packetToSend);

private:
	NetworkHandler() {}
	std::vector<sf::Packet> m_receivedPackets;
	std::unique_ptr<sf::TcpSocket> m_tcpSocket;
	std::atomic<bool> m_connectedToServer;
	std::thread m_listenThread;
	std::mutex m_mutex;

	void listen();
};