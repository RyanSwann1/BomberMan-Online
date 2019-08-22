#pragma once

#include <SFML/Network.hpp>

class NetworkHandler
{
public:
	static NetworkHandler& getInstance()
	{
		static NetworkHandler instance;
		return instance;
	}

	bool connectToServer();

private:
	NetworkHandler() {}
	sf::TcpSocket m_tcpSocket;
	bool m_connectedToServer = false;
};