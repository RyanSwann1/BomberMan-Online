#include "NetworkHandler.h"
#include <assert.h>
#include <iostream>

std::vector<sf::Packet>& NetworkHandler::getNetworkMessages()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_receivedPackets;
}

bool NetworkHandler::connectToServer()
{
	assert(!m_connectedToServer);
	if (m_tcpSocket.connect(sf::IpAddress::LocalHost, 55001, sf::seconds(2.5f)) != sf::Socket::Done)
	{
		return false;
	}

	m_connectedToServer = true;
	m_listenThread = std::thread(&NetworkHandler::listen, this);
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

void NetworkHandler::listen()
{
	while (m_connectedToServer)
	{
		sf::Packet receivedPacket;
		if (m_tcpSocket.receive(receivedPacket) == sf::Socket::Done)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_receivedPackets.push_back(receivedPacket);
		}
	}
}