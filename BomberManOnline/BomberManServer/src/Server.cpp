#include "Server.h"
#include "XMLParser/XMLParser.h"
#include <iostream>
#include "ServerMessageType.h"
#include "ServerMessages.h"
#include <assert.h>
#include "Utilities.h"
#include "GameObjectsServer.h"
#include "PathFinding.h"

constexpr size_t MAX_CLIENTS = 4;
const sf::Time TIME_OUT_DURATION = sf::seconds(0.032f);
constexpr int MAX_AI_PLAYERS = 3;

Server::Server()
	: m_tcpListener(),
	m_socketSelector(),
	m_players(),
	m_clientsToRemove(),
	m_spawnPositions(),
	m_collisionLayer(),
	m_bombs(),
	m_pickUps(),
	m_levelName(),
	m_levelSize(),
	m_clock(),
	m_currentState(eServerState::eLobby),
	m_running(false)
{
	m_players.reserve(MAX_CLIENTS);
	m_clientsToRemove.reserve(MAX_CLIENTS);
	m_spawnPositions.reserve(MAX_CLIENTS);
}

std::unique_ptr<Server> Server::create(const sf::IpAddress & ipAddress, unsigned short portNumber)
{
	Server* server = new Server;
	if (server->m_tcpListener.listen(portNumber, ipAddress) == sf::Socket::Done)
	{
		server->m_socketSelector.add(server->m_tcpListener);
		server->m_running = true;
		server->m_levelName = "Level1.tmx";
		std::vector<sf::Vector2f> collisionLayer;
		if (!XMLParser::loadLevelAsServer(server->m_levelName, server->m_levelSize,
			server->m_collisionLayer, server->m_spawnPositions, server->m_tileSize))
		{
			delete server;
			return std::unique_ptr<Server>();
		}

		//Initialize AI Players
		for (int i = 0; i < MAX_AI_PLAYERS; i++)
		{
			int clientID = static_cast<int>(server->m_players.size());
			std::unique_ptr<sf::TcpSocket> tcpSocket = std::make_unique<sf::TcpSocket>();
			sf::Vector2f startingPosition = server->m_spawnPositions.back();
			server->m_spawnPositions.pop_back();
			
			server->m_players.emplace_back(std::make_unique<PlayerServerAI>(clientID, startingPosition, ePlayerControllerType::eAI));
		}

		PathFinding::getInstance().createGraph(server->m_levelSize);

		return std::unique_ptr<Server>(server);
	}
	else
	{
		delete server;
		return std::unique_ptr<Server>();
	}
}

const std::vector<std::unique_ptr<Player>>& Server::getPlayers() const
{
	return m_players;
}

const std::vector<std::vector<eCollidableTile>>& Server::getCollisionLayer() const
{
	return m_collisionLayer;
}

const std::vector<PickUpServer>& Server::getPickUps() const
{
	return m_pickUps;
}

sf::Vector2i Server::getTileSize() const
{
	return m_tileSize;
}

sf::Vector2i Server::getLevelSize() const
{
	return m_levelSize;
}

void Server::run()
{
	std::cout << "Started listening\n";

	while (m_running)
	{
		float frameTime = m_clock.restart().asSeconds();
		if (m_currentState == eServerState::eGame)
		{
			update(frameTime);
		}
		
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
		int clientID = static_cast<int>(m_players.size());
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

		std::unique_ptr<Player> newPlayer = std::make_unique<PlayerServerHuman>(std::move(tcpSocket), clientID, startingPosition, ePlayerControllerType::eHuman);
		m_socketSelector.add(*static_cast<PlayerServerHuman*>(newPlayer.get())->m_tcpSocket.get());
		m_players.emplace_back(std::move(newPlayer));
		std::cout << "New client added to server\n";

		if (m_players.size() == 4)
		{
			m_currentState = eServerState::eGame;
			packetToSend.clear();
			packetToSend << eServerMessageType::eInitialGameData;
			ServerMessageInitialGameData initialGameDataMessage;
			initialGameDataMessage.levelName = m_levelName;
			for (const auto& player : m_players)
			{
				initialGameDataMessage.playerDetails.emplace_back(player->m_ID, player->m_position);
			}

			packetToSend << initialGameDataMessage;
			broadcastMessage(packetToSend);
		}
	}
}

void Server::listen()
{
	for (const auto& player : m_players)
	{
		if (player->m_controllerType == ePlayerControllerType::eHuman)
		{
			auto& client = *static_cast<PlayerServerHuman*>(player.get());

			if (m_socketSelector.isReady(*client.m_tcpSocket))
			{
				sf::Packet receivedPacket;
				if (client.m_tcpSocket->receive(receivedPacket) == sf::Socket::Done)
				{
					eServerMessageType serverMessageType;
					receivedPacket >> serverMessageType;
					switch (serverMessageType)
					{
					case eServerMessageType::ePlayerMoveToPosition:
					{
						ServerMessagePlayerMove playerMoveMessage;
						receivedPacket >> playerMoveMessage;
						setNewPlayerPosition(client, playerMoveMessage);
					}
					break;

					case eServerMessageType::ePlayerBombPlacementRequest:
					{
						sf::Vector2f position;
						receivedPacket >> position.x >> position.y;
						placeBomb(client, position);
					}
					break;

					case eServerMessageType::eRequestDisconnection:
					{
						m_clientsToRemove.push_back(client.m_ID);
					}
					break;
					}
				}
			}
		}
	}
}

void Server::placeBomb(sf::Vector2f position, float lifeTimeDuration)
{
	assert(position.x >= 0 && position.y >= 0 && position.x <= m_levelSize.x * m_tileSize.x && position.y <= m_levelSize.y * m_tileSize.y);

	m_bombs.emplace_back(position, lifeTimeDuration);
}

void Server::broadcastMessage(sf::Packet & packetToSend)
{
	for (const auto& player : m_players)
	{
		if(player->m_controllerType == ePlayerControllerType::eHuman)
		{
			auto& client = *static_cast<PlayerServerHuman*>(player.get());
			if (client.m_tcpSocket->send(packetToSend) != sf::Socket::Done)
			{
				std::cout << "Cannot send message to client\n";
			}
		}
	}
}

void Server::setNewPlayerPosition(PlayerServerHuman& client, ServerMessagePlayerMove playerMoveMessage)
{
	//Invalid Move
	if (client.m_moving || Utilities::isPositionCollidable(m_collisionLayer, playerMoveMessage.newPosition, m_tileSize))
	{
		sf::Packet packetToSend;
		ServerMessageInvalidMove invalidMoveMessage(playerMoveMessage.newPosition, client.m_previousPosition);
		packetToSend << eServerMessageType::eInvalidMoveRequest << invalidMoveMessage;
		if (client.m_tcpSocket->send(packetToSend) != sf::Socket::Done)
		{
			std::cout << "Failed to send message to client\n";
		}
	}
	//Valid Move
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

void Server::placeBomb(PlayerServerHuman & client, sf::Vector2f placementPosition)
{
	if (client.m_bombPlacementTimer.isExpired() && !Utilities::isPositionCollidable(m_collisionLayer, placementPosition, m_tileSize))
	{
		ServerMessageBombPlacement bombPlacementMessage;
		bombPlacementMessage.position = placementPosition;
		bombPlacementMessage.lifeTimeDuration = client.m_bombPlacementTimer.getExpirationTime();

		sf::Packet packetToSend;
		packetToSend << eServerMessageType::ePlaceBomb << bombPlacementMessage;
		broadcastMessage(packetToSend);
		std::cout << "Place Bomb\n";
		m_bombs.emplace_back(placementPosition, client.m_bombPlacementTimer.getExpirationTime());
		client.m_bombPlacementTimer.resetElaspedTime();
	}
}

void Server::update(float frameTime)
{
	//Clients To Remove
	for (auto clientToRemove = m_clientsToRemove.begin(); clientToRemove != m_clientsToRemove.end();)
	{
		int clientIDToRemove = (*clientToRemove);
		auto client = std::find_if(m_players.begin(), m_players.end(), [clientIDToRemove](const auto& player) { return player->m_ID == clientIDToRemove; });
		if (client != m_players.end())
		{
			sf::Packet packetToSend;
			packetToSend << eServerMessageType::ePlayerDisconnected << clientIDToRemove;
			broadcastMessage(packetToSend);

			if ((*client)->m_controllerType == ePlayerControllerType::eHuman)
			{
				m_socketSelector.remove(*static_cast<PlayerServerHuman*>((*client).get())->m_tcpSocket);
			}
			std::cout << "Client Removed\n";
			m_players.erase(client);
		}

		clientToRemove = m_clientsToRemove.erase(clientToRemove);
	}

	//Players
	for (auto& player : m_players)
	{
		player->m_bombPlacementTimer.update(frameTime);

		if (player->m_controllerType == ePlayerControllerType::eAI)
		{
		
			static_cast<PlayerServerAI*>(player.get())->update(*this, frameTime);
		}
		else
		{
			if (player->m_moving)
			{
				player->m_movementFactor += frameTime * player->m_movementSpeed;
				player->m_position = Utilities::Interpolate(player->m_previousPosition, player->m_newPosition, player->m_movementFactor);

				if (player->m_position == player->m_newPosition)
				{
					player->m_moving = false;
					player->m_movementFactor = 0;
				}
			}
		}
	}

	//Bombs
	for (auto bomb = m_bombs.begin(); bomb != m_bombs.end();)
	{
		bomb->m_lifeTime.update(frameTime);

		if (bomb->m_lifeTime.isExpired())
		{
			onBombExplosion(bomb->m_position);
			onBombExplosion(sf::Vector2f(bomb->m_position.x - m_tileSize.x, bomb->m_position.y));
			onBombExplosion(sf::Vector2f(bomb->m_position.x + m_tileSize.x, bomb->m_position.y));
			onBombExplosion(sf::Vector2f(bomb->m_position.x, bomb->m_position.y - m_tileSize.y));
			onBombExplosion(sf::Vector2f(bomb->m_position.x, bomb->m_position.y + m_tileSize.y));
		
			bomb = m_bombs.erase(bomb);
		}
		else
		{
			++bomb;
		}
	}

	//Pick Ups
	for (auto pickUp = m_pickUps.begin(); pickUp != m_pickUps.end();)
	{
		sf::Vector2f pickUpPosition = pickUp->m_position;
		auto player = std::find_if(m_players.begin(), m_players.end(), [pickUpPosition] (const auto& player) { return player->m_position == pickUpPosition; });
		if (player != m_players.end())
		{
			handlePickUpCollision(*player->get(), pickUp->m_type);
			pickUp = m_pickUps.erase(pickUp);
		}
		else
		{
			++pickUp;
		}
	}
}

void Server::onBombExplosion(sf::Vector2f explosionPosition)
{
	assert(explosionPosition.x >= 0 && explosionPosition.x < m_levelSize.x * m_tileSize.x && explosionPosition.y >= 0 && explosionPosition.y < m_levelSize.y * m_tileSize.y);
	
	if (explosionPosition.x >= 0 && explosionPosition.x < m_levelSize.x * m_tileSize.x && explosionPosition.y >= 0 && explosionPosition.y < m_levelSize.y * m_tileSize.y)
	{

		if (m_collisionLayer[static_cast<int>(explosionPosition.y / m_tileSize.y)][static_cast<int>(explosionPosition.x / m_tileSize.x)] == eCollidableTile::eBox)
		{
			m_collisionLayer[static_cast<int>(explosionPosition.y / m_tileSize.y)][static_cast<int>(explosionPosition.x / m_tileSize.x)] = eCollidableTile::eNonCollidable;
			sf::Packet packetToSend;
			packetToSend << eServerMessageType::eDestroyBox << explosionPosition.x << explosionPosition.y;
			broadcastMessage(packetToSend);

			if (Utilities::getRandomNumber(0, 10) > 7)
			{
				packetToSend.clear();
				packetToSend << eServerMessageType::eSpawnMovementPickUp << explosionPosition.x << explosionPosition.y;
				broadcastMessage(packetToSend);

				std::cout << "Spawn Pickup\n";
				m_pickUps.emplace_back(explosionPosition, eGameObjectType::eMovementPickUp);
			}
		}

		for (const auto& player : m_players)
		{
			sf::Vector2i playerPosition(static_cast<int>(player->m_position.x / m_tileSize.x), static_cast<int>(player->m_position.y / m_tileSize.y));
			if (sf::Vector2i(static_cast<int>(explosionPosition.x / m_tileSize.x), static_cast<int>(explosionPosition.y / m_tileSize.y)) == playerPosition)
			{
				m_clientsToRemove.push_back(player->m_ID);
			}
		}
	}
}

void Server::handlePickUpCollision(Player & player, eGameObjectType gameObjectType)
{
	switch (gameObjectType)
	{
	case eGameObjectType::eMovementPickUp :
	{
		player.m_movementSpeed += MOVEMENT_SPEED_INCREMENT;

		sf::Packet packetToSend;
		packetToSend << eServerMessageType::eMovementPickUpCollision << player.m_ID << MOVEMENT_SPEED_INCREMENT;
		broadcastMessage(packetToSend);
	}
		
		break;
	}
}