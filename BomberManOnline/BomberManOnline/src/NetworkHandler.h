#pragma once

#include <SFML/Network.hpp>
#include "ServerMessageType.h"
#include <vector>
#include <thread>

class NetworkHandler
{
public:
	static NetworkHandler& getInstance()
	{
		static NetworkHandler instance;
		return instance;
	}

	bool connectToServer();
	void sendMessageToServer(sf::Packet& packetToSend);

private:
	NetworkHandler() {}
	sf::TcpSocket m_tcpSocket;
	bool m_connectedToServer = false;
};