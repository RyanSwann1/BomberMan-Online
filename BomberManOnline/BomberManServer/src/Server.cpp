#include "Server.h"
#include "XMLParser/XMLParser.h"
#include <iostream>
#include "ServerMessageType.h"
#include "ServerMessages.h"
#include <assert.h>

constexpr size_t MAX_CLIENTS = 4;

Server::Server()
	: m_tcpListener(),
	m_socketSelector(),
	m_running(false),
	m_clients(),
	m_levelName(),
	m_mapDimensions(),
	m_collisionLayer(),
	m_spawnPositions()
{
	m_clients.reserve(MAX_CLIENTS);
}

std::unique_ptr<Server> Server::create(const sf::IpAddress & ipAddress, unsigned short portNumber)
{
	Server* server = new Server;
	std::unique_ptr<Server> uniqueServer = std::unique_ptr<Server>(server);
	if (server->m_tcpListener.listen(portNumber, ipAddress) == sf::Socket::Done)
	{
		uniqueServer->m_socketSelector.add(uniqueServer->m_tcpListener);
		uniqueServer->m_running = true;
		uniqueServer->m_levelName = "Level1.tmx";
		if (!XMLParser::loadMapAsServer(uniqueServer->m_levelName, uniqueServer->m_mapDimensions, uniqueServer->m_collisionLayer, uniqueServer->m_spawnPositions))
		{
			return std::unique_ptr<Server>();
		}

		return uniqueServer;
	}
	else
	{
		return std::unique_ptr<Server>();
	}
}

void Server::run()
{
	std::cout << "Started listening\n";
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
		int clientID = static_cast<int>(m_clients.size());
		sf::Packet packetToSend;
		packetToSend << eServerMessageType::eInitializeClientID << clientID;
		if (tcpSocket->send(packetToSend) != sf::Socket::Done)
		{
			std::cout << "Failed to send packet to newly connected client\n";
			std::cout << "Client couldn't join server\n";
			return;
		}

		//Initialize client starting location
		assert(!m_spawnPositions.empty());
		sf::Vector2f startingPosition = m_spawnPositions.back();
		m_spawnPositions.pop_back();
		m_clients.emplace_back(std::move(tcpSocket), clientID, startingPosition, ePlayerControllerType::eRemotePlayer);
		std::cout << "New client added to server\n";

		//TODO: Send once max players have joined
		packetToSend.clear();
		packetToSend << eServerMessageType::eInitialGameData;
		ServerMessageInitialGameData initialGameDataMessage;
		initialGameDataMessage.levelName = m_levelName;
		for (const auto& client : m_clients)
		{
			initialGameDataMessage.playerDetails.emplace_back(client.m_ID, client.m_position, client.m_controllerType);
		}

		packetToSend << initialGameDataMessage;
		broadcastMessage(packetToSend);
	}
}

void Server::listen()
{
	for (auto& client : m_clients)
	{
		if (m_socketSelector.isReady(*client.m_tcpSocket))
		{
			sf::Packet receivedPacket;
			if (client.m_tcpSocket->receive(receivedPacket) == sf::Socket::Done)
			{
				eServerMessageType serverMessageType;
				receivedPacket >> serverMessageType;
				switch (serverMessageType)
				{
				case eServerMessageType::ePlayerMoveToPosition :
					sf::Vector2f newPosition;
					receivedPacket >> newPosition.x >> newPosition.y;
					movePlayer(client, newPosition);
					break;
				}
			}
		}
	}
}

void Server::broadcastMessage(sf::Packet & packetToSend)
{
	for (auto& client : m_clients)
	{
		if (client.m_tcpSocket->send(packetToSend) != sf::Socket::Done)
		{
			std::cout << "Cannot send message to client\n";
		}
	}
}

void Server::movePlayer(Client& client, sf::Vector2f newPosition)
{
	bool collision = false;
	for (const auto& collidableObject : m_collisionLayer)
	{
		if (collidableObject == sf::Vector2i(newPosition.x, newPosition.y))
		{
			collision = true;
			break;
		}
	}

	sf::Packet packetToSend;
	if (collision)
	{
		std::cout << "Collision\n";
		packetToSend << static_cast<int>(eServerMessageType::eInvalidMoveRequest) << newPosition.x << newPosition.y;
		if (client.m_tcpSocket->send(packetToSend) != sf::Socket::Done)
		{
			std::cout << "Failed to send message to client\n";
		}
	}
	else
	{
		std::cout << "No Collision\n";
		client.m_position = newPosition;

		packetToSend << eServerMessageType::eValidMoveRequest << newPosition.x << newPosition.y;
		if (client.m_tcpSocket->send(packetToSend) != sf::Socket::Done)
		{
			std::cout << "Failed to send message to client\n";
		}

		sf::Packet globalPacket;
		globalPacket << static_cast<int>(eServerMessageType::eNewPlayerPosition) << newPosition.x << newPosition.y;
		broadcastMessage(packetToSend);
	}
}