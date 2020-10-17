#include "NetworkHandler.h"
#include <assert.h>
#include <iostream>

NetworkHandler::NetworkHandler()
	: m_receivedPackets(),
	m_tcpSocket(),
	m_connectedToServer(false),
	m_listenThread(),
	m_mutex()
{}

std::mutex& NetworkHandler::getListenMutex()
{
	return m_mutex;
}

bool NetworkHandler::isReceivedPackets() const
{
	return !m_receivedPackets.empty();
}

sf::Packet NetworkHandler::getLatestPacket()
{
	assert(!m_receivedPackets.empty());
	sf::Packet packetReceived = m_receivedPackets.front();
	m_receivedPackets.pop();

	return packetReceived;
}

bool NetworkHandler::connectToServer()
{
	assert(!m_connectedToServer);
	if (m_tcpSocket.connect(sf::IpAddress::LocalHost, 55001, sf::seconds(2.5f)) != sf::Socket::Done)
	{
		return false;
	}

	m_connectedToServer = true;
	m_listenThread = std::thread(&NetworkHandler::listen, this, std::ref(m_tcpSocket));

	return true;
}

void NetworkHandler::disconnectFromServer()
{
	if (m_connectedToServer)
	{
		m_connectedToServer = false;

		sf::Packet disconnectPacket;
		disconnectPacket << eServerMessageType::eRequestDisconnection;
		m_tcpSocket.send(disconnectPacket);
		m_listenThread.join();
		m_tcpSocket.disconnect();
	}
}

void NetworkHandler::sendMessageToServer(sf::Packet & packetToSend)
{
	assert(m_connectedToServer);
	if (m_tcpSocket.send(packetToSend) != sf::Socket::Done)
	{
		std::cout << "Failed to send message\n";
	}
}

void NetworkHandler::listen(sf::TcpSocket& tcpSocket)
{
	while (m_connectedToServer)
	{
		sf::Packet receivedPacket;
		if (m_tcpSocket.receive(receivedPacket) == sf::Socket::Done)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_receivedPackets.push(receivedPacket);
		}
		else
		{
			std::cout << "Received bad packet\n";
		}
	}
}