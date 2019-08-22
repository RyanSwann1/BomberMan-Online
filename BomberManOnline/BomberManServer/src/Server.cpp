#include "Server.h"
#include "XMLParser.h"
#include <iostream>
#include "ServerMessageType.h"

constexpr size_t MAX_CLIENTS = 4;

Server::Server()
	: m_tcpListener(),
	m_socketSelector(),
	m_running(false),
	m_clients()
{
	m_clients.reserve(MAX_CLIENTS);
}

std::unique_ptr<Server> Server::create(const sf::IpAddress & ipAddress, unsigned short portNumber)
{
	std::unique_ptr<Server> server = std::make_unique<Server>();
	if (server->m_tcpListener.listen(portNumber, ipAddress) == sf::Socket::Done)
	{
		server->m_socketSelector.add(server->m_tcpListener);
		server->m_running = true;
		server->m_levelName = "Level1.tmx";
		if (!XMLParser::loadMapAsServer(server->m_levelName, server->m_mapDimensions, server->m_collisionLayer, server->m_spawnPositions))
		{
			return std::unique_ptr<Server>();
		}

		return server;
	}
	else
	{
		return std::unique_ptr<Server>();
	}
}

void Server::run()
{
	while (m_running)
	{
		if (m_socketSelector.wait())
		{
			if (m_socketSelector.isReady(m_tcpListener))
			{
				addNewClient();
			}
			else
			{
				listen();
			}
		}
	}
}

void Server::addNewClient()
{
	std::unique_ptr<sf::TcpSocket> tcpSocket = std::make_unique<sf::TcpSocket>();
	if (m_tcpListener.accept(*tcpSocket) == sf::Socket::Done)
	{
		m_clients.emplace_back(std::move(tcpSocket));
		std::cout << "New client added to server\n";
	}
}

void Server::listen()
{
	for (auto& client : m_clients)
	{
		if (m_socketSelector.isReady(*client))
		{
			sf::Packet receivedPacket;
			if (client->receive(receivedPacket) == sf::Socket::Done)
			{
				int messageType = 0;
				receivedPacket >> messageType;
				switch (static_cast<eServerMessageType>(messageType))
				{
				case eServerMessageType::ePlayerMove :
				{
					sf::Vector2i newPosition;
					receivedPacket >> newPosition.x >> newPosition.y;
					movePlayer(client, newPosition);
					break;
				}
				}	
			}
		}
	}
}

void Server::broadcastMessage(sf::Packet & packetToSend)
{
	for (auto& client : m_clients)
	{
		if (client->send(packetToSend) != sf::Socket::Done)
		{
			std::cout << "Cannot send message to client\n";
		}
	}
}

void Server::movePlayer(std::unique_ptr<sf::TcpSocket>& client, sf::Vector2i newPosition)
{
	bool collision = false;
	for (const auto& collidableObject : m_collisionLayer)
	{
		if (collidableObject == newPosition)
		{
			collision = true;
		}
	}

	sf::Packet packetToSend;
	if (collision)
	{
		packetToSend << static_cast<int>(eServerMessageType::eInvalidRequest);
		if (client->send(packetToSend) == sf::Socket::Done)
		{
			std::cout << "Failed to send message to client\n";
		}
	}
	else
	{
		packetToSend << static_cast<int>(eServerMessageType::eNewPlayerPosition) << newPosition.x << newPosition.y;
		broadcastMessage(packetToSend);
	}
}