#include "Level.h"
#include "XMLParser/XMLParser.h"
#include "Resources.h"
#include "NetworkHandler.h"
#include "ServerMessages.h"
#include <assert.h>
#include <utility>
#include "Utilities.h"
#include <iostream>

constexpr size_t MAX_GAME_OBJECTS = 50;
constexpr size_t MAX_PLAYERS = 4;
constexpr size_t MAX_PREVIOUS_POINTS = 10;

Level::Level(std::string&& levelName)
	: m_levelName(std::move(levelName)),
	m_levelSize(),
	m_tileLayers(),
	m_spawnPositions(),
	m_collisionLayer(),
	m_localPlayer(nullptr),
	m_players(),
	m_gameObjects()
{
	m_players.reserve(MAX_PLAYERS);
	m_gameObjects.reserve(MAX_GAME_OBJECTS);
}

void Level::spawnPickUp(sf::Vector2f position, eGameObjectType gameObjectType)
{
	switch (gameObjectType)
	{
	case eGameObjectType::eMovementPickUp :
		m_gameObjects.emplace_back(position, 0.0f, eAnimationName::eMovementSpeedPickUp, eGameObjectType::eMovementPickUp);
		break;
	case eGameObjectType::eExtraBombPickUp :
		m_gameObjects.emplace_back(position, 0.0f, eAnimationName::eExtraBombPickUp, eGameObjectType::eExtraBombPickUp);
		break;
	case eGameObjectType::eBiggerExplosionPickUp :
		m_gameObjects.emplace_back(position, 0.0f, eAnimationName::eBiggerExplosionPickUp, eGameObjectType::eBiggerExplosionPickUp);
		break;
	}
}

void Level::spawnBomb(sf::Vector2f position, int explosionSize)
{
	m_bombs.emplace_back(position, explosionSize);
}

void Level::spawnExplosions(sf::Vector2f position, int explosionSize)
{
	m_gameObjects.emplace_back(position, EXPLOSION_LIFETIME_DURATION, eAnimationName::eExplosion, eGameObjectType::eExplosion);
	sf::Vector2i tileSize = Textures::getInstance().getTileSheet().getTileSize();
	
	for (int x = position.x - (tileSize.x * explosionSize); x <= position.x + (tileSize.x * explosionSize); x += tileSize.x)
	{
		assert(x >= 0 && position.y >= 0 && x < m_levelSize.x * tileSize.x && position.y < m_levelSize.y * tileSize.y);
		if (x >= 0 && position.y >= 0 && x < m_levelSize.x * tileSize.x && position.y < m_levelSize.y * tileSize.y)
		{
			if (m_collisionLayer[static_cast<int>(position.y / tileSize.y)][static_cast<int>(x / tileSize.x)] != eCollidableTile::eWall)
			{
				m_gameObjects.emplace_back(sf::Vector2f(x, position.y), EXPLOSION_LIFETIME_DURATION, eAnimationName::eExplosion, eGameObjectType::eExplosion);
			}
		}
	}
	
	for (int y = position.y - (tileSize.y * explosionSize); y <= position.y + (tileSize.y * explosionSize); y += tileSize.y)
	{
		assert(position.x >= 0 && y >= 0 && position.x < m_levelSize.x * tileSize.x && y < m_levelSize.y * tileSize.y);
		if (position.x >= 0 && y >= 0 && position.x < m_levelSize.x * tileSize.x && y < m_levelSize.y * tileSize.y)
		{
			if (m_collisionLayer[static_cast<int>(y / tileSize.y)][static_cast<int>(position.x / tileSize.x)] != eCollidableTile::eWall)
			{
				m_gameObjects.emplace_back(sf::Vector2f(position.x, y), EXPLOSION_LIFETIME_DURATION, eAnimationName::eExplosion, eGameObjectType::eExplosion);
			}
		}
	}
}

std::unique_ptr<Level> Level::create(int localClientID, ServerMessageInitialGameData & initialGameData)
{
	//Load Level
	Level* level = new Level(std::move(initialGameData.levelName));
	if (!XMLParser::loadLevelAsClient(level->m_levelName, level->m_levelSize, level->m_tileLayers,
		level->m_collisionLayer, level->m_spawnPositions))
	{
		delete level;
		return std::unique_ptr<Level>();
	}

	//Initialize Players
	for (const auto& playerDetails : initialGameData.playerDetails)
	{
		//Create Local Player
		if (playerDetails.ID == localClientID)
		{
			level->m_players.emplace_back(std::make_unique<PlayerClient>(playerDetails.ID, playerDetails.spawnPosition, ePlayerType::eLocal));
			level->m_localPlayer = level->m_players.back().get();
		}
		//Create Remote Player
		else
		{
			level->m_players.emplace_back(std::make_unique<PlayerClient>(playerDetails.ID, playerDetails.spawnPosition, ePlayerType::eRemote));
		}
	}

	return std::unique_ptr<Level>(level);
}

void Level::handleInput(const sf::Event & sfmlEvent)
{
	assert(m_localPlayer);
	sf::Vector2f localPlayerPosition(m_localPlayer->getPosition());
	sf::Vector2i tileSize(Textures::getInstance().getTileSheet().getTileSize());
	switch (sfmlEvent.key.code)
	{
	case sf::Keyboard::A:
		m_localPlayer->setNewPosition(sf::Vector2f(localPlayerPosition.x - tileSize.x, localPlayerPosition.y),
			m_collisionLayer, tileSize, m_localPlayerPreviousPositions);

		break;

	case sf::Keyboard::D:
		m_localPlayer->setNewPosition(sf::Vector2f(localPlayerPosition.x + tileSize.x, localPlayerPosition.y),
			m_collisionLayer, tileSize, m_localPlayerPreviousPositions);

		break;

	case sf::Keyboard::W:
		m_localPlayer->setNewPosition(sf::Vector2f(localPlayerPosition.x, localPlayerPosition.y - tileSize.y),
			m_collisionLayer, tileSize, m_localPlayerPreviousPositions);

		break;

	case sf::Keyboard::S:
		m_localPlayer->setNewPosition(sf::Vector2f(localPlayerPosition.x, localPlayerPosition.y + tileSize.y),
			m_collisionLayer, tileSize, m_localPlayerPreviousPositions);

		break;

	case sf::Keyboard::Space:
		if (m_localPlayer->placeBomb())
		{
			sf::Packet packetToSend;
			packetToSend << eServerMessageType::ePlayerBombPlacementRequest << localPlayerPosition.x << localPlayerPosition.y;
			NetworkHandler::getInstance().sendMessageToServer(packetToSend);
		}

		break;
	}
}

void Level::render(sf::RenderWindow & window) const
{
	//Tile Layer
	for (const auto& tileLayer : m_tileLayers)
	{
		for (int y = 0; y < m_levelSize.y; ++y)
		{
			for (int x = 0; x < m_levelSize.x; ++x)
			{
				int tileID = tileLayer.m_tileLayer[y][x];
				if (tileID > 0)
				{
					const auto& tileSheet = Textures::getInstance().getTileSheet();
					sf::Sprite tileSprite(tileSheet.getTexture(), tileSheet.getFrameRect(tileID));
					tileSprite.setPosition(x * tileSheet.getTileSize().x, y * tileSheet.getTileSize().y);

					window.draw(tileSprite);
				}
			}
		}
	}

	//Collision Layer
	for (int y = 0; y < m_levelSize.y; y++)
	{
		for (int x = 0; x < m_levelSize.x; x++)
		{
			if (m_collisionLayer[y][x] == eCollidableTile::eBox)
			{
				const auto& tileSheet = Textures::getInstance().getTileSheet();
				sf::Sprite boxSprite(tileSheet.getTexture(), tileSheet.getFrameRect(204));
				boxSprite.setPosition(sf::Vector2f(x * tileSheet.getTileSize().x, y * tileSheet.getTileSize().y));

				window.draw(boxSprite);
			}
		}
	}

	//Players
	for (const auto& player : m_players)
	{
		player->render(window);
	}

	//Game Objects
	for (const auto& gameObject : m_gameObjects)
	{
		gameObject.render(window);
	}

	//Bombs
	for (const auto& bomb : m_bombs)
	{
		bomb.render(window);
	}
}

void Level::update(float deltaTime)
{
	//Players
	for (auto& player : m_players)
	{
		player->update(deltaTime);

		//Reached destination
		if (player->getPosition() == player->getNewPosition())
		{
			if (m_localPlayerPreviousPositions.size() > MAX_PREVIOUS_POINTS)
			{
				for (auto iter = m_localPlayerPreviousPositions.begin(); iter != m_localPlayerPreviousPositions.end();)
				{
					m_localPlayerPreviousPositions.erase(iter);
					break;
				}
			}
		}
	}

	//Game Objects
	for (auto gameObject = m_gameObjects.begin(); gameObject != m_gameObjects.end();)
	{
		gameObject->update(deltaTime);

		if (gameObject->getType() == eGameObjectType::eMovementPickUp || 
			gameObject->getType() == eGameObjectType::eExtraBombPickUp || 
			gameObject->getType() == eGameObjectType::eBiggerExplosionPickUp) 
		{
			sf::Vector2f pickUpPosition(gameObject->getPosition());
			auto player = std::find_if(m_players.cbegin(), m_players.cend(), [pickUpPosition](const auto& player) { return player->getPosition() == pickUpPosition; });
			if (player != m_players.cend())
			{
				gameObject = m_gameObjects.erase(gameObject);
			}
			else
			{
				++gameObject;
			}
		}
		else
		{
			if (gameObject->getTimer().isExpired())
			{
				gameObject = m_gameObjects.erase(gameObject);
			}
			else
			{
				++gameObject;
			}
		}
	}

	//Bombs
	for (auto bomb = m_bombs.begin(); bomb != m_bombs.end();)
	{
		bomb->update(deltaTime);

		if (bomb->getTimer().isExpired())
		{
			spawnExplosions(bomb->getPosition(), bomb->getExplosionSize());
			bomb = m_bombs.erase(bomb);
		}
		else
		{
			++bomb;
		}
	}
}

void Level::onReceivedServerMessage(eServerMessageType receivedMessageType, sf::Packet & receivedMessage, sf::RenderWindow& window)
{
	switch (receivedMessageType)
	{
	case eServerMessageType::eInvalidMoveRequest :
	{
		ServerMessageInvalidMove invalidMoveMessage;
		receivedMessage >> invalidMoveMessage;

		bool previousPositionFound = false;
		bool clearRemaining = false;
		for (auto previousPosition = m_localPlayerPreviousPositions.begin(); previousPosition != m_localPlayerPreviousPositions.end();)
		{
			if (clearRemaining)
			{
				previousPosition = m_localPlayerPreviousPositions.erase(previousPosition);
			}
			else if ((*previousPosition).position == invalidMoveMessage.invalidPosition)
			{
				m_localPlayer->stopAtPosition(invalidMoveMessage.lastValidPosition);

				previousPosition = m_localPlayerPreviousPositions.erase(previousPosition);
				clearRemaining = true;
				previousPositionFound = true;
			}
			else
			{
				++previousPosition;
			}
		}

		if (!previousPositionFound)
		{
			std::cout << "Close Window\n";
			//NetworkHandler::getInstance().disconnectFromServer();
			//window.close();
		}
	}
	break;
		
	case eServerMessageType::eNewPlayerPosition :
	{
		sf::Vector2f newPosition;
		int clientID = INVALID_CLIENT_ID;
		receivedMessage >> newPosition.x >> newPosition.y >> clientID;
		if (clientID == m_localPlayer->getID())
		{
			for (auto iter = m_localPlayerPreviousPositions.begin(); iter != m_localPlayerPreviousPositions.end();)
			{
				if ((*iter).position == newPosition)
				{
					iter = m_localPlayerPreviousPositions.erase(iter);
					break;
				}
				else
				{
					++iter;
				}
			}
		}
		else
		{
			auto remotePlayer = std::find_if(m_players.begin(), m_players.end(), [clientID](const auto& player) { return player->getID() == clientID; });
			assert(remotePlayer != m_players.end());

			(*remotePlayer)->setNewPosition(newPosition, m_collisionLayer, Textures::getInstance().getTileSheet().getTileSize(), 
				m_localPlayerPreviousPositions);
		}
	}
		break;

	case eServerMessageType::ePlaceBomb :
	{
		sf::Vector2f placementPosition;
		receivedMessage >> placementPosition.x >> placementPosition.y;
		int explosionSize = 0;
		receivedMessage >> explosionSize;

		spawnBomb(placementPosition, explosionSize);
	}
		break;
	case eServerMessageType::eDestroyBox :
	{
		sf::Vector2f boxPosition;
		receivedMessage >> boxPosition.x >> boxPosition.y;
		sf::Vector2i tileSize = Textures::getInstance().getTileSheet().getTileSize();
		m_collisionLayer[static_cast<int>(boxPosition.y / tileSize.y)][static_cast<int>(boxPosition.x / tileSize.x)] = eCollidableTile::eNonCollidable;
	}
		break;
	case eServerMessageType::ePlayerDisconnected :
	{
		int clientID = INVALID_CLIENT_ID;
		receivedMessage >> clientID;
		assert(clientID != INVALID_CLIENT_ID);
		if (m_localPlayer->getID() == clientID)
		{
			window.close();
		}
		else
		{
			auto iter = std::find_if(m_players.begin(), m_players.end(), [clientID](const auto& player) { return player->getID() == clientID; });
			assert(iter != m_players.end());
			
			m_players.erase(iter);
		}
	}
		break;
	case eServerMessageType::eSpawnMovementPickUp :
	{
		sf::Vector2f startingPosition;
		receivedMessage >> startingPosition.x >> startingPosition.y;
		spawnPickUp(startingPosition, eGameObjectType::eMovementPickUp);
	}
		break;
		
	case eServerMessageType::eMovementPickUpCollision :
	{
		int clientID = INVALID_CLIENT_ID;
		float movementSpeedIncrement = 0;
		receivedMessage >> clientID >> movementSpeedIncrement;
		for (auto& player : m_players)
		{
			if (clientID == player->getID())
			{
				player->increaseMovementSpeed(movementSpeedIncrement);
				break;
			}
		}
	}
		break;
		
	case eServerMessageType::eSpawnExtraBombPickUp :
	{
		sf::Vector2f startingPosition;
		receivedMessage >> startingPosition.x >> startingPosition.y;

		spawnPickUp(startingPosition, eGameObjectType::eExtraBombPickUp);
	}
	break;

	case eServerMessageType::eExtraBombPickUpCollision :
	{
		int clientID = INVALID_CLIENT_ID;
		receivedMessage >> clientID;
		for (auto& player : m_players)
		{
			if (clientID == player->getID())
			{
				player->increaseBombCount();
				break;
			}
		}
	}
		break;

	case eServerMessageType::eSpawnBiggerExplosionPickUp :
	{
		sf::Vector2f startingPosition;
		receivedMessage >> startingPosition.x >> startingPosition.y;

		spawnPickUp(startingPosition, eGameObjectType::eBiggerExplosionPickUp);
	}
		break;

	case eServerMessageType::eBiggerExplosionPickUpCollision :
	{
		int clientID = INVALID_CLIENT_ID;
		receivedMessage >> clientID;
		for (auto& player : m_players)
		{
			if (player->getID() == clientID)
			{
				player->increaseBombExplosionSize();
				break;
			}
		}
	}
		break;
	}
}

MovementPoint::MovementPoint(sf::Vector2f position, eDirection moveDirection)
	: position(position),
	moveDirection(moveDirection)
{}