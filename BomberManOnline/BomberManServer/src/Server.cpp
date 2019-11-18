#include "Server.h"
#include "XMLParser/XMLParser.h"
#include <iostream>
#include "ServerMessageType.h"
#include "ServerMessages.h"
#include <assert.h>
#include "Utilities.h"
#include "PlayerServer.h"
#include "PathFinding.h"

constexpr size_t MAX_PLAYERS = 4;
constexpr int MAX_AI_PLAYERS = 3;
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
		if (player->getControllerType() == ePlayerControllerType::eHuman)
		{
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
					}
				}
			}
		}
	}
}

void Server::placeBomb(sf::Vector2f position)
{
	assert(position.x >= 0 && position.y >= 0 && position.x <= m_levelSize.x * m_tileSize.x && position.y <= m_levelSize.y * m_tileSize.y);
	if (position.x >= 0 && position.y >= 0 && position.x <= m_levelSize.x * m_tileSize.x && position.y <= m_levelSize.y * m_tileSize.y)
	{
		m_gameObjects.emplace_back(position, BOMB_LIFETIME_DURATION, eGameObjectType::eBomb, eTimerActive::eTrue);
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
	//Invalid Move
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
	//Valid Move
	else
	{
		client.setNewPosition(playerMoveMessage.newPosition, *this);
	}
}

void Server::placeBomb(PlayerServerHuman & client, sf::Vector2f placementPosition)
{
	Timer& clientBombPlacementTimer = client.getBombPlacementTimer();
	if (clientBombPlacementTimer.isExpired() && !Utilities::isPositionCollidable(m_collisionLayer, placementPosition, m_tileSize))
	{
		ServerMessageBombPlacement bombPlacementMessage;
		bombPlacementMessage.position = placementPosition;
		bombPlacementMessage.lifeTimeDuration = clientBombPlacementTimer.getExpirationTime();

		sf::Packet packetToSend;
		packetToSend << eServerMessageType::ePlaceBomb << bombPlacementMessage;
		broadcastMessage(packetToSend);
		m_gameObjects.emplace_back(placementPosition, clientBombPlacementTimer.getExpirationTime(), eGameObjectType::eBomb);
		clientBombPlacementTimer.resetElaspedTime();
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
		if (gameObject->getType() == eGameObjectType::eBomb && gameObject->getTimer().isExpired())
		{
			onBombExplosion(gameObject->getPosition());
			onBombExplosion(sf::Vector2f(gameObject->getPosition().x - m_tileSize.x, gameObject->getPosition().y));
			onBombExplosion(sf::Vector2f(gameObject->getPosition().x + m_tileSize.x, gameObject->getPosition().y));
			onBombExplosion(sf::Vector2f(gameObject->getPosition().x, gameObject->getPosition().y - m_tileSize.y));
			onBombExplosion(sf::Vector2f(gameObject->getPosition().x, gameObject->getPosition().y + m_tileSize.y));

			gameObject = m_gameObjects.erase(gameObject);
			gameObjectDestroyed = true;
		}
		else if (gameObject->getType() == eGameObjectType::eMovementPickUp)
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

		if (!gameObjectDestroyed)
		{
			++gameObject;
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
	assert(explosionPosition.x >= 0 && explosionPosition.x < m_levelSize.x * m_tileSize.x && explosionPosition.y >= 0 && explosionPosition.y < m_levelSize.y * m_tileSize.y);
	if (explosionPosition.x >= 0 && explosionPosition.x < m_levelSize.x * m_tileSize.x && explosionPosition.y >= 0 && explosionPosition.y < m_levelSize.y * m_tileSize.y)
	{
		if (m_collisionLayer[static_cast<int>(explosionPosition.y / m_tileSize.y)][static_cast<int>(explosionPosition.x / m_tileSize.x)] == eCollidableTile::eBox)
		{
			m_collisionLayer[static_cast<int>(explosionPosition.y / m_tileSize.y)][static_cast<int>(explosionPosition.x / m_tileSize.x)] = eCollidableTile::eNonCollidable;
			sf::Packet packetToSend;
			packetToSend << eServerMessageType::eDestroyBox << explosionPosition.x << explosionPosition.y;
			broadcastMessage(packetToSend);

			//Spawn PickUp
			if (Utilities::getRandomNumber(0, 10) >= 7)
			{
				packetToSend.clear();
			 	const int randNumb = Utilities::getRandomNumber(0, 1);	
				switch (randNumb)
				{
				case 0 :
				{
					packetToSend << eServerMessageType::eSpawnExtraBombPickUp << explosionPosition.x << explosionPosition.y;
					m_gameObjectQueue.emplace_back(explosionPosition, 0.0f, eGameObjectType::eExtraBombPickUp);
					
					break;
				}
				case 1 :
				{
					packetToSend << eServerMessageType::eSpawnMovementPickUp << explosionPosition.x << explosionPosition.y;
					m_gameObjectQueue.emplace_back(explosionPosition, 0.0f, eGameObjectType::eMovementPickUp);

					break;
				}
				}

				broadcastMessage(packetToSend);
			}
		}

		//Damage colliding players
		for (const std::unique_ptr<PlayerServer>& player : m_players)
		{
			sf::Vector2i playerPosition(static_cast<int>(player->getPosition().x / m_tileSize.x), static_cast<int>(player->getPosition().y / m_tileSize.y));
			if (sf::Vector2i(static_cast<int>(explosionPosition.x / m_tileSize.x), static_cast<int>(explosionPosition.y / m_tileSize.y)) == playerPosition)
			{
				m_clientsToRemove.push_back(player->getID());
			}
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
	}
		break;
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

	for (const std::unique_ptr<PlayerServer>& player : m_players)
	{
		initialGameDataMessage.playerDetails.emplace_back(player->getID(), player->getPosition());
	}

	packetToSend << initialGameDataMessage;
	broadcastMessage(packetToSend);
}