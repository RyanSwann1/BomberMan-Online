#include "NetworkHandler.h"
#include <assert.h>
#include <iostream>

bool NetworkHandler::connectToServer()
{
	assert(m_connectedToServer);
	if (m_tcpSocket.connect(sf::IpAddress::LocalHost, 55001, sf::seconds(2.5f)) != sf::Socket::Done)
	{
		return false;
	}

	m_connectedToServer = true;
	return true;
}

void NetworkHandler::sendMessageToServer(sf::Packet & packetToSend)
{
	assert(m_connectedToServer);
	if (m_tcpSocket.send(packetToSend) != sf::Socket::Done)
	{
		std::cout << "Failed to send message\n";
	}
}