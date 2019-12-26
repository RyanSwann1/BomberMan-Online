#include "Server.h"
#include "XMLParser/XMLParser.h"
#include <iostream>
#include "ServerMessageType.h"
#include "ServerMessages.h"
#include <assert.h>
#include "Utilities.h"
#include "PlayerServer.h"
#include "PlayerServerAI.h"
#include "PathFinding.h"

constexpr size_t MAX_PLAYERS = 2;
constexpr int MAX_AI_PLAYERS = 1;
const sf::Time TIME_OUT_DURATION = sf::seconds(0.032f);

Server::Server()
	: m_tcpListener(),
	m_socketSelector(),
	m_players(),
	m_clientsToRemove(),
	m_spawnPositions(),
	m_collisionLayer(),
	m_gameObjects(),
	m_levelName(),
	m_levelSize(),
	m_clock(),
	m_currentState(eServerState::eLobby),
	m_running(false)
{
	m_players.reserve(MAX_PLAYERS);
	m_clientsToRemove.reserve(MAX_PLAYERS);
	m_spawnPositions.reserve(MAX_PLAYERS);
}

std::unique_ptr<Server> Server::create(const sf::IpAddress & ipAddress, unsigned short portNumber)
{
	Server* server = new Server;
	if (server->m_tcpListener.listen(portNumber, ipAddress) == sf::Socket::Done)
	{
		server->m_socketSelector.add(server->m_tcpListener);
		server->m_running = true;
		server->m_levelName = "Level1.tmx";
		if (!XMLParser::loadLevelAsServer(server->m_levelName, server->m_levelSize,
			server->m_collisionLayer, server->m_spawnPositions, server->m_tileSize))
		{
			delete server;
			return std::unique_ptr<Server>();
		}

		//Initialize AI Players
		for (int i = 0; i < MAX_AI_PLAYERS; i++)
		{
			assert(!server->m_spawnPositions.empty());
			sf::Vector2f startingPosition = server->m_spawnPositions.back();
			server->m_spawnPositions.pop_back();

			int clientID = static_cast<int>(server->m_players.size());
			server->m_players.emplace_back(std::make_unique<PlayerServerAI>(clientID, startingPosition, *server));
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

bool Server::isPickUpAtPosition(sf::Vector2f position) const
{
	for (const GameObject& gameObject : m_gameObjects)
	{
		if (gameObject.getPosition() == position
			&& (gameObject.getType() == eGameObjectType::eMovementPickUp ||
				gameObject.getType() == eGameObjectType::eExtraBombPickUp ||
				gameObject.getType() == eGameObjectType::eBiggerExplosionPickUp))
		{
			return true;
		}
	}

	return false;
}

bool Server::isBombAtPosition(sf::Vector2f position) const
{
	auto bomb = std::find_if(m_bombs.cbegin(), m_bombs.cend(), [position](const auto& bomb) { return bomb.getPosition() == position; });
	return bomb != m_bombs.cend();
}

const PlayerServer* Server::getPlayer(int ID) const
{
	auto cIter = std::find_if(m_players.cbegin(), m_players.cend(), [ID](const auto& player) { return player->getID() == ID; });
	return (cIter != m_players.cend() ? cIter->get() : nullptr);
}

const BombServer* Server::getBomb(sf::Vector2f position) const
{
	auto bomb = std::find_if(m_bombs.cbegin(), m_bombs.cend(), [position](const auto& bomb) { return bomb.getPosition() == position; });
	return (bomb != m_bombs.cend() ? &(*bomb) : nullptr);
}

const std::vector<std::unique_ptr<PlayerServer>>& Server::getPlayers() const
{
	return m_players;
}

const std::vector<std::vector<eCollidableTile>>& Server::getCollisionLayer() const
{
	return m_collisionLayer;
}

eCollidableTile Server::getCollidableTile(sf::Vector2i position) const
{
	assert(position.x >= 0 && position.x < m_levelSize.x && position.y >= 0 && position.y < m_levelSize.y);
	if (position.x >= 0 && position.x < m_levelSize.x && position.y >= 0 && position.y < m_levelSize.y)
	{
		return m_collisionLayer[position.y][position.x];
	}
}

const std::vector<GameObject>& Server::getGameObjects() const
{
	return m_gameObjects;
}

const std::vector<BombServer>& Server::getBombs() const
{
	return m_bombs;
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
		if (m_currentState == eServerState::eGame)
		{
			float frameTime = m_clock.restart().asSeconds();
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

		m_players.push_back(std::make_unique<PlayerServerHuman>(std::move(tcpSocket), clientID, startingPosition, m_socketSelector));
		std::cout << "New client added to server\n";

		//Player Limit Reached
		if (m_players.size() == MAX_PLAYERS)
		{
			startGame();
		}
	}
}

void Server::listen()
{
	for (const auto& player : m_players)
	{
		if (player->getControllerType() != ePlayerControllerType::eHuman)
		{
			continue;
		}

		auto& client = *static_cast<PlayerServerHuman*>(player.get());
		if (m_socketSelector.isReady(*client.getTCPSocket()))
		{
			sf::Packet receivedPacket;
			if (client.getTCPSocket()->receive(receivedPacket) == sf::Socket::Done)
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
					m_clientsToRemove.push_back(client.getID());
				}
				break;
				
				case eServerMessageType::eRequestKickBomb :
				{
					sf::Vector2f position;
					receivedPacket >> position.x >> position.y;
					eDirection kickDirection;
					receivedPacket >> kickDirection;
					
					onBombKick(position, kickDirection);
				}
				break;
				}
			}
		}
	}
}

void Server::kickBombInDirection(sf::Vector2f bombPosition, sf::Vector2f newPosition)
{
	auto bomb = std::find_if(m_bombs.begin(), m_bombs.end(), [bombPosition](const auto& bomb) { return bomb.getPosition() == bombPosition; });
	assert(bomb != m_bombs.end());
	if (bomb != m_bombs.end())
	{
		bomb->setNewPosition(newPosition);

		sf::Packet packetToSend;
		packetToSend << eServerMessageType::eBombKicked << bomb->getPosition() << newPosition;
		broadcastMessage(packetToSend);
	}
}

void Server::placeBomb(sf::Vector2f position, int explosionRange)
{
	assert(position.x >= 0 && position.y >= 0 && position.x <= m_levelSize.x * m_tileSize.x && position.y <= m_levelSize.y * m_tileSize.y);
	if (position.x >= 0 && position.y >= 0 && position.x <= m_levelSize.x * m_tileSize.x && position.y <= m_levelSize.y * m_tileSize.y)
	{
		m_bombs.emplace_back(position, explosionRange);
	}
}

void Server::broadcastMessage(sf::Packet & packetToSend)
{
	for (const auto& player : m_players)
	{
		if(player->getControllerType() == ePlayerControllerType::eHuman)
		{
			auto& client = *static_cast<PlayerServerHuman*>(player.get());
			if (client.getTCPSocket()->send(packetToSend) != sf::Socket::Done)
			{
				std::cout << "Cannot send message to client\n";
			}
		}
	}
}

void Server::setNewPlayerPosition(PlayerServerHuman& client, ServerMessagePlayerMove playerMoveMessage)
{
	if (Utilities::isPositionCollidable(m_collisionLayer, playerMoveMessage.newPosition, m_tileSize))
	{
		sf::Packet packetToSend;
		ServerMessageInvalidMove invalidMoveMessage(playerMoveMessage.newPosition, client.getPreviousPosition());
		packetToSend << eServerMessageType::eInvalidMoveRequest << invalidMoveMessage;
		if (client.getTCPSocket()->send(packetToSend) != sf::Socket::Done)
		{
			std::cout << "Failed to send message to client\n";
		}
	}
	else
	{
		client.setNewPosition(playerMoveMessage.newPosition, *this);
	}
}

void Server::placeBomb(PlayerServerHuman & client, sf::Vector2f placementPosition)
{
	if (!Utilities::isPositionCollidable(m_collisionLayer, placementPosition, m_tileSize) && client.placeBomb())
	{
		sf::Packet packetToSend;
		packetToSend << eServerMessageType::ePlaceBomb << placementPosition.x << placementPosition.y;
		broadcastMessage(packetToSend);
		
		placeBomb(placementPosition, client.getCurrentBombExplosionSize());
	}
}

void Server::update(float frameTime)
{
	//Clients To Remove
	for (auto clientToRemove = m_clientsToRemove.begin(); clientToRemove != m_clientsToRemove.end();)
	{
		int clientIDToRemove = (*clientToRemove);
		auto client = std::find_if(m_players.begin(), m_players.end(), [clientIDToRemove](const auto& player) { return player->getID() == clientIDToRemove; });
		if (client != m_players.end())
		{
			sf::Packet packetToSend;
			packetToSend << eServerMessageType::ePlayerDisconnected << clientIDToRemove;
			broadcastMessage(packetToSend);

			if ((*client)->getControllerType() == ePlayerControllerType::eHuman)
			{
				m_socketSelector.remove(*static_cast<PlayerServerHuman*>((*client).get())->getTCPSocket());
			}

			std::cout << "Client Removed\n";
			m_players.erase(client);
		}

		clientToRemove = m_clientsToRemove.erase(clientToRemove);
	}

	//Players
	for (auto& player : m_players)
	{
		player->update(frameTime);
	}

	//Game Objects
	for (auto gameObject = m_gameObjects.begin(); gameObject != m_gameObjects.end();)
	{
		gameObject->update(frameTime);

		bool gameObjectDestroyed = false;
		if (gameObject->getType() == eGameObjectType::eMovementPickUp ||
			gameObject->getType() == eGameObjectType::eExtraBombPickUp ||
			gameObject->getType() == eGameObjectType::eBiggerExplosionPickUp)
		{
			sf::Vector2f pickUpPosition = gameObject->getPosition();
			auto player = std::find_if(m_players.begin(), m_players.end(), [pickUpPosition](const auto& player) { return player->getPosition() == pickUpPosition; });
			if (player != m_players.end())
			{
				handlePickUpCollision(*player->get(), gameObject->getType());
				gameObject = m_gameObjects.erase(gameObject);
				gameObjectDestroyed = true;
			}
		}
		
		if(!gameObjectDestroyed)
		{
			++gameObject;
		}
	}

	//Bombs
	for (auto bomb = m_bombs.begin(); bomb != m_bombs.end();)
	{
		bomb->update(frameTime);

		//Bomb Explosion
		if (bomb->getTimer().isExpired())
		{
			int explosionSize = bomb->getExplosionSize();
			sf::Vector2f explosionPosition = Utilities::getClosestGridPosition(bomb->getPosition(), m_tileSize);

			sf::Packet packetToSend;
			packetToSend << eServerMessageType::eBombExplosion << explosionPosition.x << explosionPosition.y << explosionSize;
			broadcastMessage(packetToSend);

			onBombExplosion(explosionPosition);

			//sf::Vector2f endPosition(explosionPosition.x + (explosionSize * m_tileSize.x), explosionPosition.y);
			//while (Utilities::traverseDirection(explosionPosition, endPosition, m_tileSize, eDirection::eRight))
			//{
			//	onBombExplosion(explosionPosition);
			//	if (getCollidableTile(explosionPosition) == eCollidableTile::eBox || getCollidableTile(explosionPosition) == eCollidableTile::eWall)
			//	{
			//		break;
			//	}
			//}

			explosionPosition = Utilities::getClosestGridPosition(bomb->getPosition(), m_tileSize);
			for (int x = explosionPosition.x + m_tileSize.x; x <= explosionPosition.x + (m_tileSize.x * explosionSize); x += m_tileSize.x)
			{
				eCollidableTile collidableTile = getCollidableTile(sf::Vector2f(x, explosionPosition.y));
				onBombExplosion(sf::Vector2f(x, explosionPosition.y));
				if (collidableTile == eCollidableTile::eBox || collidableTile == eCollidableTile::eWall)
				{
					break;
				}
			}

			for (int x = explosionPosition.x - m_tileSize.x; x >= explosionPosition.x - (m_tileSize.x * explosionSize); x -= m_tileSize.x)
			{
				eCollidableTile collidableTile = getCollidableTile(sf::Vector2f(x, explosionPosition.y));
				onBombExplosion(sf::Vector2f(x, explosionPosition.y));
				if (collidableTile == eCollidableTile::eBox || collidableTile == eCollidableTile::eWall)
				{
					break;
				}
			}

			for (int y = explosionPosition.y - m_tileSize.y; y >= explosionPosition.y - (m_tileSize.y * explosionSize); y -= m_tileSize.y)
			{
				eCollidableTile collidableTile = getCollidableTile(sf::Vector2f(explosionPosition.x, y));
				onBombExplosion(sf::Vector2f(explosionPosition.x, y));
				if (collidableTile == eCollidableTile::eBox || collidableTile == eCollidableTile::eWall)
				{
					break;
				}
			}

			for (int y = explosionPosition.y + m_tileSize.y; y <= explosionPosition.y + (m_tileSize.y * explosionSize); y += m_tileSize.y)
			{
				eCollidableTile collidableTile = getCollidableTile(sf::Vector2f(explosionPosition.x, y));
				onBombExplosion(sf::Vector2f(explosionPosition.x, y));
				if (collidableTile == eCollidableTile::eBox || collidableTile == eCollidableTile::eWall)
				{
					break;
				}
			}

			bomb = m_bombs.erase(bomb);
		}
		else
		{
			++bomb;
		}
	}

	//Game Object Queue
	if (!m_gameObjectQueue.empty())
	{
		for (const auto& i : m_gameObjectQueue)
		{
			m_gameObjects.push_back(i);
		}

		m_gameObjectQueue.clear();
	}
}

void Server::onBombExplosion(sf::Vector2f explosionPosition)
{
	if (getCollidableTile(explosionPosition) == eCollidableTile::eBox)
	{
		changeCollidableTile(explosionPosition, eCollidableTile::eNonCollidable);

		//Spawn PickUp
		if (Utilities::getRandomNumber(0, 10) >= 9)
		{
			sf::Packet packetToSend;
			switch (Utilities::getRandomNumber(0, 2))
			{
			case 0:
				packetToSend << eServerMessageType::eSpawnExtraBombPickUp << explosionPosition.x << explosionPosition.y;
				m_gameObjectQueue.emplace_back(explosionPosition, 0.0f, eGameObjectType::eExtraBombPickUp);

				break;
			case 1:
				packetToSend << eServerMessageType::eSpawnMovementPickUp << explosionPosition.x << explosionPosition.y;
				m_gameObjectQueue.emplace_back(explosionPosition, 0.0f, eGameObjectType::eMovementPickUp);

				break;
			case 2:
				packetToSend << eServerMessageType::eSpawnBiggerExplosionPickUp << explosionPosition.x << explosionPosition.y;
				m_gameObjectQueue.emplace_back(explosionPosition, 0.0f, eGameObjectType::eBiggerExplosionPickUp);

				break;
			}

			broadcastMessage(packetToSend);
		}
	}

	//Damage colliding players
	for (const auto& player : m_players)
	{
		sf::Vector2i playerPosition(static_cast<int>(player->getPosition().x / m_tileSize.x), static_cast<int>(player->getPosition().y / m_tileSize.y));
		if (sf::Vector2i(static_cast<int>(explosionPosition.x / m_tileSize.x), static_cast<int>(explosionPosition.y / m_tileSize.y)) == playerPosition)
		{
			m_clientsToRemove.push_back(player->getID());
		}
	}
}

void Server::onBombKick(sf::Vector2f playerPosition, eDirection kickDirection)
{
	auto bombToKick = std::find_if(m_bombs.begin(), m_bombs.end(), [playerPosition] (const auto& bomb) { return bomb.getPosition() == playerPosition; });
	if (bombToKick != m_bombs.end())
	{
		sf::Vector2f kickToPosition = PathFinding::getInstance().getFurthestNonCollidablePosition(playerPosition, kickDirection, *this);
		if (kickToPosition != playerPosition)
		{
			bombToKick->setNewPosition(kickToPosition);
			sf::Packet packetToSend;
			packetToSend << eServerMessageType::eBombKicked << bombToKick->getPosition() << kickToPosition;
			
			broadcastMessage(packetToSend);
		}
	}
}

void Server::handlePickUpCollision(PlayerServer & player, eGameObjectType gameObjectType)
{
	switch (gameObjectType)
	{
	case eGameObjectType::eMovementPickUp :
	{
		player.increaseMovementSpeed(MOVEMENT_SPEED_INCREMENT);

		sf::Packet packetToSend;
		packetToSend << eServerMessageType::eMovementPickUpCollision << player.getID() << MOVEMENT_SPEED_INCREMENT;
		broadcastMessage(packetToSend);

		break;
	}
	case eGameObjectType::eExtraBombPickUp :
	{
		player.increaseBombCount();

		sf::Packet packetToSend;
		packetToSend << eServerMessageType::eExtraBombPickUpCollision << player.getID();
		broadcastMessage(packetToSend);

		break;
	}
	case eGameObjectType::eBiggerExplosionPickUp :
	{
		player.increaseBombExplosionSize();

		sf::Packet packetToSend;
		packetToSend << eServerMessageType::eExtraBombPickUpCollision << player.getID();
		broadcastMessage(packetToSend);
		
		break;
	}
	}
}

void Server::startGame()
{
	assert(m_players.size() == MAX_PLAYERS);

	m_currentState = eServerState::eGame;

	sf::Packet packetToSend;
	packetToSend.clear();
	packetToSend << eServerMessageType::eInitialGameData;
	ServerMessageInitialGameData initialGameDataMessage;
	initialGameDataMessage.levelName = m_levelName;

	for (const auto& player : m_players)
	{
		initialGameDataMessage.playerDetails.emplace_back(player->getID(), player->getPosition());
	}

	packetToSend << initialGameDataMessage;
	broadcastMessage(packetToSend);
}

void Server::changeCollidableTile(sf::Vector2f position, eCollidableTile collidableTile)
{
	assert(position.x >= 0 && position.x < m_levelSize.x * m_tileSize.x && position.y >= 0 && position.y < m_levelSize.y * m_tileSize.y);
	if (position.x >= 0 && position.x < m_levelSize.x * m_tileSize.x && position.y >= 0 && position.y < m_levelSize.y * m_tileSize.y)
	{
		m_collisionLayer[static_cast<int>(position.y / m_tileSize.y)][static_cast<int>(position.x / m_tileSize.x)] = collidableTile;
	}
}

eCollidableTile Server::getCollidableTile(sf::Vector2f position) 
{
	assert(position.x >= 0 && position.x < (m_levelSize.x * m_tileSize.x) && position.y >= 0 && position.y < (m_levelSize.y * m_tileSize.x));
	if (position.x >= 0 && position.x < m_levelSize.x * m_tileSize.y && position.y >= 0 && position.y < m_levelSize.y * m_tileSize.y)
	{
		return m_collisionLayer[static_cast<int>(position.y / m_tileSize.y)][static_cast<int>(position.x / m_tileSize.x)];
	}
}