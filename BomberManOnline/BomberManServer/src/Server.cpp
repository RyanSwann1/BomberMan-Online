#include "Server.h"
#include "XMLParser/XMLParser.h"
#include <iostream>
#include "ServerMessageType.h"
#include "ServerMessages.h"
#include <assert.h>
#include "Utilities.h"

constexpr size_t MAX_CLIENTS = 4;
const sf::Time TIME_OUT_DURATION = sf::seconds(0.032f);

Server::Server()
	: m_tcpListener(),
	m_socketSelector(),
	m_running(false),
	m_clients(),
	m_levelName(),
	m_mapDimensions(),
	m_collisionLayer(),
	m_spawnPositions(),
	m_clock()
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
		float frameTime = m_clock.restart().asSeconds();
		update(frameTime);

		if (m_socketSelector.wait(TIME_OUT_DURATION))
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
		m_socketSelector.add(*m_clients.back().m_tcpSocket);
		std::cout << "New client added to server\n";

		if (m_clients.size() >= 2)
		{
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
				{
					ServerMessagePlayerMove playerMoveMessage;
					receivedPacket >> playerMoveMessage;
					movePlayer(client, playerMoveMessage);
				}
					break;

				case eServerMessageType::ePlayerBombPlacementRequest :
				{
					sf::Vector2f position;
					receivedPacket >> position.x >> position.y;
					placeBomb(client, position);
				}
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

void Server::movePlayer(Client& client, ServerMessagePlayerMove playerMoveMessage)
{
	sf::Packet packetToSend;
	if (client.m_movementSpeed != playerMoveMessage.speed || client.m_moving || Utilities::isPositionCollidable(m_collisionLayer, playerMoveMessage.newPosition))
	{
		ServerMessageInvalidMove invalidMoveMessage(playerMoveMessage.newPosition, client.m_position);
		packetToSend << eServerMessageType::eInvalidMoveRequest << invalidMoveMessage;
		if (client.m_tcpSocket->send(packetToSend) != sf::Socket::Done)
		{
			std::cout << "Failed to send message to client\n";
		}
	}
	else
	{
		client.m_newPosition = playerMoveMessage.newPosition;
		client.m_previousPosition = client.m_position;
		client.m_moving = true;

		sf::Packet globalPacket;
		globalPacket << static_cast<int>(eServerMessageType::eNewPlayerPosition) << playerMoveMessage.newPosition.x << playerMoveMessage.newPosition.y << client.m_ID;
		broadcastMessage(globalPacket);
	}
}

void Server::placeBomb(Client & client, sf::Vector2f placementPosition)
{
	if (client.m_bombPlacementTimer.isExpired() && !Utilities::isPositionCollidable(m_collisionLayer, placementPosition))
	{
		ServerMessageBombPlacement bombPlacementMessage;
		bombPlacementMessage.position = placementPosition;
		bombPlacementMessage.lifeTimeDuration = client.m_bombPlacementTimer.getExpirationTime();

		sf::Packet packetToSend;
		packetToSend << eServerMessageType::ePlaceBomb << bombPlacementMessage;
		broadcastMessage(packetToSend);
		std::cout << "Place Bomb\n";
		m_bombs.emplace_back(placementPosition, client.m_bombPlacementTimer.getExpirationTime());
	}
}

void Server::update(float frameTime)
{
	for (auto& client : m_clients)
	{
		if (client.m_moving)
		{
			client.m_movementFactor += frameTime * client.m_movementSpeed;
			client.m_position = Utilities::Interpolate(client.m_previousPosition, client.m_newPosition, client.m_movementFactor);

			if (client.m_position == client.m_newPosition)
			{
				client.m_moving = false;
			}
		}

		client.m_bombPlacementTimer.update(frameTime);
	}

	for (auto iter = m_bombs.begin(); iter != m_bombs.end();)
	{
		iter->m_lifeTime.update(frameTime);

		if (iter->m_lifeTime.isExpired())
		{
			for (int x = iter->m_position.x - 1; x <= iter->m_position.x; x += 2)
			{

			}


			iter = m_bombs.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}