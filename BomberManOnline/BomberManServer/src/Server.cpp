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
#include "TileID.h"

constexpr size_t MAX_PLAYERS = 1;
constexpr int MAX_AI_PLAYERS = 0;
const sf::Time TIME_OUT_DURATION = sf::seconds(0.032f);

Server::Server()
	: m_tcpListener(),
	m_socketSelector(),
	m_players(),
	m_playersToRemove(),
	m_spawnPositions(),
	m_pickUps(),
	m_levelName(),
	m_levelSize(),
	m_clock(),
	m_currentState(eServerState::eLobby),
	m_running(true)
{
	m_players.reserve(MAX_PLAYERS);
	m_playersToRemove.reserve(MAX_PLAYERS);
	m_spawnPositions.reserve(MAX_PLAYERS);
}

std::unique_ptr<Server> Server::create(const sf::IpAddress & ipAddress, unsigned short portNumber)
{
	Server* server = new Server;
	if (server->m_tcpListener.listen(portNumber, ipAddress) == sf::Socket::Done)
	{
		//server->m_levelCollapser.activate({ 16 * 3, 16 * 3 });
		server->m_socketSelector.add(server->m_tcpListener);
		server->m_levelName = "Level1.tmx";
		if (!XMLParser::loadLevelAsServer(server->m_levelName, server->m_levelSize,
			server->m_tileManager.m_tileLayers, server->m_tileManager.m_collisionLayer, 
			server->m_spawnPositions, server->m_tileSize))
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

float Server::getElaspedTime() const
{
	return m_elaspedTime;
}

bool Server::isPickUpAtPosition(sf::Vector2f position) const
{
	for (const GameObject& gameObject : m_pickUps)
	{
		if (gameObject.getPosition() == position && gameObject.isPickUp())
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
	auto player = std::find_if(m_players.cbegin(), m_players.cend(), [ID](const auto& player) { return player->getID() == ID; });
	return (player != m_players.cend() ? player->get() : nullptr);
}

const BombServer* Server::getBomb(sf::Vector2f position) const
{
	auto bomb = std::find_if(m_bombs.cbegin(), m_bombs.cend(), [position](const auto& bomb) { return bomb.getPosition() == position; });
	return (bomb != m_bombs.cend() ? &(*bomb) : nullptr);
}

const TileManager& Server::getTileManager() const
{
	return m_tileManager;
}

const std::vector<std::unique_ptr<PlayerServer>>& Server::getPlayers() const
{
	return m_players;
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
					m_playersToRemove.push_back(client.getID());
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
	assert(Utilities::isPositionInLevelBounds(position, m_tileSize, m_levelSize));
	if (Utilities::isPositionInLevelBounds(position, m_tileSize, m_levelSize));
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
	if (m_tileManager.isPositionCollidable(Utilities::convertToGridPosition(playerMoveMessage.newPosition, m_tileSize)))
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
	if (!m_tileManager.isPositionCollidable(Utilities::convertToGridPosition(placementPosition, m_tileSize)) && client.placeBomb())
	{
		sf::Packet packetToSend;
		packetToSend << eServerMessageType::ePlaceBomb << placementPosition.x << placementPosition.y;
		broadcastMessage(packetToSend);
		
		placeBomb(placementPosition, client.getCurrentBombExplosionSize());
	}
}

void Server::update(float frameTime)
{
	m_elaspedTime += frameTime;
	m_levelCollapser.update(*this, m_tileManager, frameTime);

	//Clients To Remove
	for (auto playerToRemove = m_playersToRemove.begin(); playerToRemove != m_playersToRemove.end();)
	{
		int playerIDToRemove = (*playerToRemove);
		auto client = std::find_if(m_players.begin(), m_players.end(), [playerIDToRemove](const auto& player) { return player->getID() == playerIDToRemove; });
		if (client != m_players.end())
		{
			sf::Packet packetToSend;
			packetToSend << eServerMessageType::ePlayerDisconnected << playerIDToRemove;
			broadcastMessage(packetToSend);

			if ((*client)->getControllerType() == ePlayerControllerType::eHuman)
			{
				m_socketSelector.remove(*static_cast<PlayerServerHuman*>((*client).get())->getTCPSocket());
			}

			std::cout << "Player Removed\n";
			m_players.erase(client);
		}

		playerToRemove = m_playersToRemove.erase(playerToRemove);
	}

	//Game Objects
	for (auto pickUp = m_pickUps.begin(); pickUp != m_pickUps.end();)
	{
		pickUp->update(frameTime);

		bool gameObjectDestroyed = false;

		if (m_levelCollapser.isActive() &&
			m_tileManager.isTileOnPosition(eTileID::eWall, Utilities::convertToGridPosition(pickUp->getPosition(), m_tileSize)))
		{
			pickUp = m_pickUps.erase(pickUp);
			gameObjectDestroyed = true;
		}
		else 
		{
			sf::Vector2f pickUpPosition = pickUp->getPosition();
			auto player = std::find_if(m_players.begin(), m_players.end(), [pickUpPosition](const auto& player) { return player->getPosition() == pickUpPosition; });
			if (player != m_players.end())
			{
				handlePickUpCollision(*player->get(), pickUp->getType());
				pickUp = m_pickUps.erase(pickUp);
				gameObjectDestroyed = true;
			}
		}
		
		if(!gameObjectDestroyed)
		{
			++pickUp;
		}
	}

	//Bombs
	for (auto bomb = m_bombs.begin(); bomb != m_bombs.end();)
	{
		bomb->update(frameTime);

		sf::Vector2f bombPosition = Utilities::getClosestGridPosition(bomb->getPosition(), m_tileSize);
		if (m_levelCollapser.isActive() && 
			m_tileManager.isTileOnPosition(eTileID::eWall, Utilities::convertToGridPosition(bombPosition, m_tileSize)))
		{
			bomb = m_bombs.erase(bomb);
		}
		//Bomb Explosion
		else if (bomb->getTimer().isExpired())
		{
			int explosionSize = bomb->getExplosionSize();
			sf::Vector2f bombPosition = Utilities::getClosestGridPosition(bomb->getPosition(), m_tileSize);

			sf::Packet packetToSend;
			packetToSend << eServerMessageType::eBombExplosion << bombPosition.x << bombPosition.y << explosionSize;
			broadcastMessage(packetToSend);

			onBombExplosion(bombPosition);

			for (sf::Vector2i direction : Utilities::getAllDirections())
			{
				for (int i = 1; i <= explosionSize; ++i)
				{
					sf::Vector2f explosionPosition = bombPosition + Utilities::scale(m_tileSize, direction, i);
					if (m_tileManager.isTileOnPosition(eTileID::eBox, Utilities::convertToGridPosition(explosionPosition, m_tileSize)))
					{
						onBombExplosion(explosionPosition);
						break;
					}
					else if (m_tileManager.isPositionCollidable(Utilities::convertToGridPosition(explosionPosition, m_tileSize)))
					{
						break;
					}
					else
					{
						onBombExplosion(explosionPosition);
					}
				}
			}

			bomb = m_bombs.erase(bomb);
		}
		else
		{
			++bomb;
		}
	}

	//Players
	for (auto& player : m_players)
	{
		player->update(frameTime);

		sf::Vector2f playerPosition = Utilities::getClosestGridPosition(player->getPosition(), m_tileSize);
		if (m_levelCollapser.isActive() &&
			m_tileManager.isTileOnPosition(eTileID::eWall, Utilities::convertToGridPosition(playerPosition, m_tileSize)))
		{
			m_playersToRemove.push_back(player->getID());
		}
	}

	//Game Object Queue
	if (!m_pickUpQueue.empty())
	{
		for (const auto& i : m_pickUpQueue)
		{
			m_pickUps.push_back(i);
		}

		m_pickUpQueue.clear();
	}
}

void Server::onBombExplosion(sf::Vector2f explosionPosition)
{
	auto gameObject = std::find_if(m_pickUps.begin(), m_pickUps.end(), [explosionPosition](const auto& gameObject)
		{ return gameObject.getPosition() == explosionPosition; });
	if (gameObject != m_pickUps.cend())
	{
		sf::Packet packetToSend;
		packetToSend << eServerMessageType::eRemovePickUpAtLocation << explosionPosition;
		broadcastMessage(packetToSend);

		m_pickUps.erase(gameObject);
	}
	else if (m_tileManager.isTileOnPosition(eTileID::eBox, Utilities::convertToGridPosition(explosionPosition, m_tileSize)))
	{
		m_tileManager.removeTile(eTileID::eBox, Utilities::convertToGridPosition(explosionPosition, m_tileSize));

		//Spawn PickUp
		if (Utilities::getRandomNumber(0, 10) >= 9)
		{
			sf::Packet packetToSend;
			switch (Utilities::getRandomNumber(0, 2))
			{
			case 0:
				packetToSend << eServerMessageType::eSpawnExtraBombPickUp << explosionPosition;
				m_pickUpQueue.emplace_back(explosionPosition, 0.0f, eGameObjectType::eExtraBombPickUp);

				break;
			case 1:
				packetToSend << eServerMessageType::eSpawnMovementPickUp << explosionPosition;
				m_pickUpQueue.emplace_back(explosionPosition, 0.0f, eGameObjectType::eMovementPickUp);

				break;
			case 2:
				packetToSend << eServerMessageType::eSpawnBiggerExplosionPickUp << explosionPosition;
				m_pickUpQueue.emplace_back(explosionPosition, 0.0f, eGameObjectType::eBiggerExplosionPickUp);

				break;
			}

			broadcastMessage(packetToSend);
		}
	}

	//Damage colliding players
	for (const auto& player : m_players)
	{
		if (Utilities::convertToGridPosition(player->getPosition(), m_tileSize) == 
			Utilities::convertToGridPosition(explosionPosition, m_tileSize))
		{
			m_playersToRemove.push_back(player->getID());
		}
	}
}

void Server::onBombKick(sf::Vector2f playerPosition, eDirection kickDirection)
{
	auto bombToKick = std::find_if(m_bombs.begin(), m_bombs.end(), [playerPosition] (const auto& bomb) { return bomb.getPosition() == playerPosition; });
	if (bombToKick != m_bombs.end())
	{
		sf::Vector2f kickToPosition = PathFinding::getInstance().getFurthestNonCollidablePosition(playerPosition, kickDirection, *this, MAX_KICK_RANGE);
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