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
constexpr int MAX_AI_PLAYERS =  2;

Server::Server()
	: m_tcpListener(),
	m_socketSelector(),
	m_running(false),
	m_players(),
	m_levelName(),
	m_mapDimensions(),
	m_collisionLayer(),
	m_spawnPositions(),
	m_clock(),
	m_gameRunning(false)
{
	m_players.reserve(MAX_CLIENTS);
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
		if (!XMLParser::loadLevelAsServer(server->m_levelName, server->m_mapDimensions,
			server->m_collisionLayer, server->m_spawnPositions))
		{
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

		PathFinding::getInstance().initGraph(server->m_mapDimensions);

		return std::unique_ptr<Server>(server);
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

		if (m_players.size() >= 3)
		{
			//TODO: Send once max players have joined
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

			m_gameRunning = true;
		}
	}
}

void Server::listen()
{
	for (auto& player : m_players)
	{
		if (player->m_controllerType == ePlayerControllerType::eHuman)
		{
			auto& client = *static_cast<PlayerServerHuman*>(player.get());

			if (m_socketSelector.isReady(*client.m_tcpSocket))
			{
				std::cout << "Player Ready\n";
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
						movePlayer(client, playerMoveMessage);
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

void Server::broadcastMessage(sf::Packet & packetToSend)
{
	for (auto& player : m_players)
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

void Server::movePlayer(PlayerServerHuman& client, ServerMessagePlayerMove playerMoveMessage)
{
	sf::Packet packetToSend;
	//Invalid Move
	if (client.m_movementSpeed != playerMoveMessage.speed || client.m_moving || Utilities::isPositionCollidable(m_collisionLayer, playerMoveMessage.newPosition))
	{
		ServerMessageInvalidMove invalidMoveMessage(playerMoveMessage.newPosition, client.m_position);
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
		client.m_bombPlacementTimer.resetElaspedTime();
	}
}

void Server::update(float frameTime)
{
	if (!m_gameRunning)
	{
		return;
	}

	for (auto iter = m_clientsToRemove.begin(); iter != m_clientsToRemove.end();)
	{
		int clientIDToRemove = (*iter);
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

		iter = m_clientsToRemove.erase(iter);
	}

	for (auto& player : m_players)
	{
		if (player->m_controllerType == ePlayerControllerType::eAI)
		{
			updateAI(*static_cast<PlayerServerAI*>(player.get()), frameTime);
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

		player->m_bombPlacementTimer.update(frameTime);
	}

	for (auto bomb = m_bombs.begin(); bomb != m_bombs.end();)
	{
		bomb->m_lifeTime.update(frameTime);

		if (bomb->m_lifeTime.isExpired())
		{
			for (int x = bomb->m_position.x - 16; x <= bomb->m_position.x + 16; x += 32)
			{
				sf::Vector2f explosionPosition(x, bomb->m_position.y);
				if (m_collisionLayer[explosionPosition.y / 16][explosionPosition.x / 16] == eCollidableTile::eBox)
				{
					m_collisionLayer[explosionPosition.y / 16][explosionPosition.x / 16] = eCollidableTile::eNonCollidable;

					sf::Packet packetToSend;
					packetToSend << eServerMessageType::eDestroyBox << explosionPosition.x << explosionPosition.y;
					broadcastMessage(packetToSend);
				}
			
				auto player = std::find_if(m_players.begin(), m_players.end(), [explosionPosition](const auto& player) { return explosionPosition == player->m_position; });
				if (player != m_players.end())
				{
					m_clientsToRemove.push_back(player->get()->m_ID);
				}
			}
			
			for (int y = bomb->m_position.y - 16; y <= bomb->m_position.y + 16; y += 32)
			{
				sf::Vector2f explosionPosition(bomb->m_position.x, y);
				if (m_collisionLayer[explosionPosition.y / 16][explosionPosition.x / 16] == eCollidableTile::eBox)
				{
					m_collisionLayer[explosionPosition.y / 16][explosionPosition.x / 16] = eCollidableTile::eNonCollidable;

					sf::Packet packetToSend;
					packetToSend << eServerMessageType::eDestroyBox << explosionPosition.x << explosionPosition.y;
					broadcastMessage(packetToSend);
				}

				auto player = std::find_if(m_players.begin(), m_players.end(), [explosionPosition](const auto& player) { return explosionPosition == player->m_position; });
				if (player != m_players.end())
				{
					m_clientsToRemove.push_back(player->get()->m_ID);
				}
			}

			bomb = m_bombs.erase(bomb);
		}
		else
		{
			++bomb;
		}
	}
}

void Server::updateAI(PlayerServerAI& player, float frameTime)
{
	switch (player.m_currentState)
	{
	case eAIState::eIdle :
	{
		bool targetFound = false;
		if (player.m_behavour == eAIBehaviour::eAggressive)
		{
			sf::Vector2i playerPosition(player.m_position.x / 16, player.m_position.y / 16);
			for (const auto& targetPlayer : m_players)
			{
				if (targetPlayer->m_ID != player.m_ID)
				{
					sf::Vector2i targetPosition(targetPlayer->m_position.x / 16, targetPlayer->m_position.y / 16);
					if (PathFinding::getInstance().isPositionReachable(playerPosition, targetPosition, m_collisionLayer, m_mapDimensions))
					{
						targetFound = true;
						player.m_currentState = eAIState::eMoveToPlayer;
						player.m_pathToTile.clear();
						break;
					}
				}
			}
		}
		if (!targetFound || player.m_behavour == eAIBehaviour::ePassive)
		{
			PathFinding::getInstance().pathToClosestBox(sf::Vector2i(player.m_position.x / 16, player.m_position.y / 16),
				m_collisionLayer, m_mapDimensions, player.m_pathToTile);
			if (!player.m_pathToTile.empty())
			{
				player.m_currentState = eAIState::eMoveToBox;
				player.m_moving = true;

				player.m_newPosition = player.m_pathToTile.back();
				player.m_pathToTile.pop_back();
				player.m_previousPosition = player.m_position;

				sf::Packet globalPacket;
				globalPacket << static_cast<int>(eServerMessageType::eNewPlayerPosition) << player.m_newPosition.x << player.m_newPosition.y << player.m_ID;
				broadcastMessage(globalPacket);
			}
		}
	}
		
		break;
	case eAIState::eMoveToBox :
	{
		player.m_movementFactor += frameTime * player.m_movementSpeed;
		player.m_position = Utilities::Interpolate(player.m_previousPosition, player.m_newPosition, player.m_movementFactor);

		if (player.m_position == player.m_newPosition)
		{
			player.m_movementFactor = 0;
			sf::Packet globalPacket;
			globalPacket << static_cast<int>(eServerMessageType::eNewPlayerPosition) << player.m_position.x << player.m_position.y << player.m_ID;
			broadcastMessage(globalPacket);

			bool targetFound = false;
			if (player.m_behavour == eAIBehaviour::eAggressive)
			{
				sf::Vector2i playerPosition(player.m_position.x / 16, player.m_position.y / 16);
				for (const auto& targetPlayer : m_players)
				{
					if (targetPlayer->m_ID != player.m_ID)
					{
						sf::Vector2i targetPosition(targetPlayer->m_position.x / 16, targetPlayer->m_position.y / 16);
						if (PathFinding::getInstance().isPositionReachable(playerPosition, targetPosition, m_collisionLayer, m_mapDimensions))
						{
							targetFound = true;
							player.m_currentState = eAIState::eMoveToPlayer;
							player.m_pathToTile.clear();
							break;
						}
					}
				}
			}

			if (!targetFound)
			{
				if (player.m_pathToTile.empty())
				{
					player.m_moving = false;
					player.m_currentState = eAIState::ePlantBomb;
				}
				else
				{
					player.m_moving = true;
					player.m_newPosition = player.m_pathToTile.back();
					player.m_pathToTile.pop_back();
					player.m_previousPosition = player.m_position;
				}
			}
		}
	}
		
		break;
	case eAIState::eSetTargetPosition :
	{
		for (const auto& target : m_players)
		{
			if (target->m_ID != player.m_ID)
			{
				PathFinding::getInstance().getPathToTile(sf::Vector2i(player.m_position.x, player.m_position.y), sf::Vector2i(target->m_position.x, target->m_position.y),
					m_collisionLayer, m_mapDimensions, player.m_pathToTile);
				if (!player.m_pathToTile.empty())
				{
					player.m_currentState = eAIState::eMoveToPlayer;
					player.m_moving = true;

					player.m_newPosition = player.m_pathToTile.back();
					player.m_pathToTile.pop_back();
					player.m_previousPosition = player.m_position;

					sf::Packet globalPacket;
					globalPacket << static_cast<int>(eServerMessageType::eNewPlayerPosition) << player.m_newPosition.x << player.m_newPosition.y << player.m_ID;
					broadcastMessage(globalPacket);
				}
			}
		}
	}
		
		break;
	case eAIState::eMoveToPlayer :
	{
		player.m_movementFactor += frameTime * player.m_movementSpeed;
		player.m_position = Utilities::Interpolate(player.m_previousPosition, player.m_newPosition, player.m_movementFactor);

		if (player.m_position == player.m_newPosition)
		{
			player.m_movementFactor = 0;
			sf::Packet globalPacket;
			globalPacket << static_cast<int>(eServerMessageType::eNewPlayerPosition) << player.m_position.x << player.m_position.y << player.m_ID;
			broadcastMessage(globalPacket);

			if (player.m_pathToTile.empty())
			{
				player.m_moving = false;
				player.m_currentState = eAIState::ePlantBomb;
			}
			else
			{
				player.m_moving = true;
				player.m_newPosition = player.m_pathToTile.back();
				player.m_pathToTile.pop_back();
				player.m_previousPosition = player.m_position;
			}
		}
	}
	
		break;
	case eAIState::eSetSafePosition :
	{
		PathFinding::getInstance().pathToClosestSafePosition(sf::Vector2i(player.m_position.x / 16, player.m_position.y / 16),
			m_collisionLayer, m_mapDimensions, player.m_pathToTile);
		player.m_currentState = eAIState::eMoveToSafePosition;
		player.m_moving = true;

		player.m_newPosition = player.m_pathToTile.back();
		player.m_pathToTile.pop_back();
		player.m_previousPosition = player.m_position;
	}
		
		break;
	case eAIState::eMoveToSafePosition :
	{
		player.m_movementFactor += frameTime * player.m_movementSpeed;
		player.m_position = Utilities::Interpolate(player.m_previousPosition, player.m_newPosition, player.m_movementFactor);

		if (player.m_position == player.m_newPosition)
		{
			player.m_movementFactor = 0;
			sf::Packet globalPacket;
			globalPacket << static_cast<int>(eServerMessageType::eNewPlayerPosition) << player.m_position.x << player.m_position.y << player.m_ID;
			broadcastMessage(globalPacket);

			if (player.m_pathToTile.empty())
			{
				player.m_moving = false;
				player.m_currentState = eAIState::eWait;
			}
			else
			{
				player.m_moving = true;
				player.m_newPosition = player.m_pathToTile.back();
				player.m_pathToTile.pop_back();
				player.m_previousPosition = player.m_position;
			}
		}
	}
		
		break;
	case eAIState::ePlantBomb :
	{
		if (player.m_bombPlacementTimer.isExpired())
		{
			ServerMessageBombPlacement bombPlacementMessage;
			bombPlacementMessage.position = player.m_position;
			bombPlacementMessage.lifeTimeDuration = player.m_bombPlacementTimer.getExpirationTime();

			sf::Packet packetToSend;
			packetToSend << eServerMessageType::ePlaceBomb << bombPlacementMessage;
			broadcastMessage(packetToSend);
			std::cout << "Place Bomb\n";
			m_bombs.emplace_back(bombPlacementMessage.position, bombPlacementMessage.lifeTimeDuration);

			player.m_currentState = eAIState::eSetSafePosition;
		}
	}
		
		break;
	case eAIState::eWait :
	{
		player.m_waitTimer.setActive(true);
		player.m_waitTimer.update(frameTime);
		if (player.m_waitTimer.isExpired())
		{
			player.m_currentState = eAIState::eIdle;
			player.m_waitTimer.resetElaspedTime();
		}
	}
		
		break;
	}
}