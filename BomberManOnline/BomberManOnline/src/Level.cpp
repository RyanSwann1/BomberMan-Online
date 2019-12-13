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

void Level::onBombExplosion(sf::Vector2f position, int explosionSize)
{
	addExplosionObject(position);
	sf::Vector2i tileSize = Textures::getInstance().getTileSheet().getTileSize();
	
	for (int x = position.x + tileSize.x; x <= position.x + (tileSize.x * explosionSize); x += tileSize.x)
	{
		if (getCollidableTile(sf::Vector2i(x, position.y)) == eCollidableTile::eNonCollidable)
		{
			addExplosionObject(sf::Vector2f(x, position.y));
		}
		else if(getCollidableTile(sf::Vector2i(x, position.y)) == eCollidableTile::eBox)
		{
			addExplosionObject(sf::Vector2f(x, position.y));
			changeCollidableTile(sf::Vector2i(x, position.y), eCollidableTile::eNonCollidable);
			break;
		}
		else if (getCollidableTile(sf::Vector2i(x, position.y)) == eCollidableTile::eWall)
		{
			break;
		}
	}

	for (int x = position.x - tileSize.x; x >= position.x - (tileSize.x * explosionSize); x -= tileSize.x)
	{
		if (getCollidableTile(sf::Vector2i(x, position.y)) == eCollidableTile::eNonCollidable)
		{
			addExplosionObject(sf::Vector2f(x, position.y));
		}
		else if (getCollidableTile(sf::Vector2i(x, position.y)) == eCollidableTile::eBox)
		{
			addExplosionObject(sf::Vector2f(x, position.y));
			changeCollidableTile(sf::Vector2i(x, position.y), eCollidableTile::eNonCollidable);
			break;
		}
		else if (getCollidableTile(sf::Vector2i(x, position.y)) == eCollidableTile::eWall)
		{
			break;
		}
	}

	for (int y = position.y - tileSize.y; y >= position.y - (tileSize.y * explosionSize); y -= tileSize.y)
	{
		if (getCollidableTile(sf::Vector2i(position.x, y)) == eCollidableTile::eNonCollidable)
		{
			addExplosionObject(sf::Vector2f(position.x, y));
		}
		else if (getCollidableTile(sf::Vector2i(position.x, y)) == eCollidableTile::eBox)
		{
			addExplosionObject(sf::Vector2f(position.x, y));
			changeCollidableTile(sf::Vector2i(position.x, y), eCollidableTile::eNonCollidable);
			break;
		}
		else if (getCollidableTile(sf::Vector2i(position.x, y)) == eCollidableTile::eWall)
		{
			break;
		}
	}

	for (int y = position.y + tileSize.y; y <= position.y + (tileSize.y * explosionSize); y += tileSize.y)
	{
		if (getCollidableTile(sf::Vector2i(position.x, y)) == eCollidableTile::eNonCollidable)
		{
			addExplosionObject(sf::Vector2f(position.x, y));
		}
		else if (getCollidableTile(sf::Vector2i(position.x, y)) == eCollidableTile::eBox)
		{
			addExplosionObject(sf::Vector2f(position.x, y));
			changeCollidableTile(sf::Vector2i(position.x, y), eCollidableTile::eNonCollidable);
			break;
		}
		else if (getCollidableTile(sf::Vector2i(position.x, y)) == eCollidableTile::eWall)
		{
			break;
		}
	}
}

eCollidableTile Level::getCollidableTile(sf::Vector2i position) const
{
	sf::Vector2i tileSize = Textures::getInstance().getTileSheet().getTileSize();
	assert(position.x >= 0 && position.y >= 0 && position.x < m_levelSize.x * tileSize.x && position.y < m_levelSize.y * tileSize.y);
	if (position.x >= 0 && position.y >= 0 && position.x < m_levelSize.x * tileSize.x && position.y < m_levelSize.y * tileSize.y)
	{
		return m_collisionLayer[position.y / tileSize.y][position.x / tileSize.x];
	}
}

void Level::changeCollidableTile(sf::Vector2i position, eCollidableTile collidableTile)
{
	sf::Vector2i tileSize = Textures::getInstance().getTileSheet().getTileSize();
	assert(position.x >= 0 && position.y >= 0 && position.x < m_levelSize.x * tileSize.x && position.y < m_levelSize.y * tileSize.y);
	if (position.x >= 0 && position.y >= 0 && position.x < m_levelSize.x * tileSize.x && position.y < m_levelSize.y * tileSize.y)
	{
		m_collisionLayer[position.y / tileSize.y][position.x / tileSize.x] = collidableTile;
	}
}

void Level::addExplosionObject(sf::Vector2f position)
{
	m_gameObjects.emplace_back(position, EXPLOSION_LIFETIME_DURATION, eAnimationName::eExplosion, eGameObjectType::eExplosion, eTimerActive::eTrue);
}

void Level::kickBombToPosition(sf::Vector2f bombPosition, sf::Vector2f kickToPosition)
{
	auto bombToKick = std::find_if(m_gameObjects.begin(), m_gameObjects.end(), [bombPosition](const auto& gameObject) 
		{ return gameObject.getPosition() == bombPosition; });

	assert(bombToKick != m_gameObjects.cend());
	if (bombToKick != m_gameObjects.end())
	{
		bombToKick->setNewPosition(kickToPosition);
	}
}

PlayerClient* Level::getPlayer(int ID)
{
	auto iter = std::find_if(m_players.begin(), m_players.end(), [ID](const auto& player) { return player->getID() == ID; });
	if (iter != m_players.cend())
	{
		return iter->get();
	}
	
	return nullptr;
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
			sf::Vector2f bombPlacementPosition(Utilities::getClosestGridPosition(localPlayerPosition, tileSize));
			packetToSend << eServerMessageType::ePlayerBombPlacementRequest << bombPlacementPosition.x << bombPlacementPosition.y;
			NetworkHandler::getInstance().sendMessageToServer(packetToSend);
		}

		break;
		
	case sf::Keyboard::F :
	{
		sf::Packet packetToSend;
		packetToSend << eServerMessageType::eRequestKickBomb << m_localPlayer->getPosition() << m_localPlayer->getFacingDirection();
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
			const auto& tileSheet = Textures::getInstance().getTileSheet();
			if (getCollidableTile(sf::Vector2i(x * tileSheet.getTileSize().x, y * tileSheet.getTileSize().y)) == eCollidableTile::eBox)
			{
				sf::Sprite boxSprite(tileSheet.getTexture(), tileSheet.getFrameRect(static_cast<int>(eFrameID::eBox)));
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
			auto player = std::find_if(m_players.cbegin(), m_players.cend(), [pickUpPosition](const auto& player) 
				{ return player->getPosition() == pickUpPosition; });
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
}

void Level::onReceivedServerMessage(eServerMessageType receivedMessageType, sf::Packet & receivedMessage, sf::RenderWindow& window)
{
	switch (receivedMessageType)
	{
	case eServerMessageType::eInvalidMoveRequest:
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
			std::cout << "Disconnected for cheating.\n";
			/*NetworkHandler::getInstance().disconnectFromServer();
			window.close();*/
		}
	}
	break;

	case eServerMessageType::eNewPlayerPosition:
	{
		sf::Vector2f newPosition;
		int playerID = INVALID_PLAYER_ID;
		receivedMessage >> newPosition.x >> newPosition.y >> playerID;
		if (playerID == m_localPlayer->getID())
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
			auto remotePlayer = std::find_if(m_players.begin(), m_players.end(), [playerID](const auto& player) { return player->getID() == playerID; });
			assert(remotePlayer != m_players.end());

			(*remotePlayer)->setNewPosition(newPosition, m_collisionLayer, Textures::getInstance().getTileSheet().getTileSize(),
				m_localPlayerPreviousPositions);
		}
	}
	break;

	case eServerMessageType::ePlaceBomb:
	{
		sf::Vector2f placementPosition;
		receivedMessage >> placementPosition.x >> placementPosition.y;

		m_gameObjects.emplace_back(placementPosition, BOMB_LIFETIME_DURATION, eAnimationName::eBomb, eGameObjectType::eBomb, eTimerActive::eTrue);
	}
	break;
	case eServerMessageType::eBombExplosion:
	{
		sf::Vector2f position;
		receivedMessage >> position.x >> position.y;
		int explosionSize = 0;
		receivedMessage >> explosionSize;

		onBombExplosion(position, explosionSize);
	}
	break;
	case eServerMessageType::ePlayerDisconnected:
	{
		int playerID = INVALID_PLAYER_ID;
		receivedMessage >> playerID;
		assert(playerID != INVALID_PLAYER_ID);
		if (m_localPlayer->getID() == playerID)
		{
			window.close();
		}
		else
		{
			auto iter = std::find_if(m_players.begin(), m_players.end(), [playerID](const auto& player) { return player->getID() == playerID; });
			assert(iter != m_players.end());

			m_players.erase(iter);
		}
	}
	break;
	case eServerMessageType::eSpawnMovementPickUp:
	{
		sf::Vector2f startingPosition;
		receivedMessage >> startingPosition.x >> startingPosition.y;
		spawnPickUp(startingPosition, eGameObjectType::eMovementPickUp);
	}
	break;

	case eServerMessageType::eMovementPickUpCollision:
	{
		int playerID = INVALID_PLAYER_ID;
		float movementSpeedIncrement = 0;
		receivedMessage >> playerID >> movementSpeedIncrement;

		PlayerClient* player = getPlayer(playerID);
		assert(player);
		if (player)
			player->increaseMovementSpeed(movementSpeedIncrement);
	
	}
	break;
	case eServerMessageType::eSpawnExtraBombPickUp:
	{
		sf::Vector2f startingPosition;
		receivedMessage >> startingPosition.x >> startingPosition.y;

		spawnPickUp(startingPosition, eGameObjectType::eExtraBombPickUp);
	}
	break;

	case eServerMessageType::eExtraBombPickUpCollision:
	{
		int playerID = INVALID_PLAYER_ID;
		receivedMessage >> playerID;

		PlayerClient* player = getPlayer(playerID);
		assert(player);
		if (player)
			player->increaseBombCount();
	}
	break;

	case eServerMessageType::eSpawnBiggerExplosionPickUp:
	{
		sf::Vector2f startingPosition;
		receivedMessage >> startingPosition.x >> startingPosition.y;

		spawnPickUp(startingPosition, eGameObjectType::eBiggerExplosionPickUp);
	}
	break;

	case eServerMessageType::eBiggerExplosionPickUpCollision:
	{
		int playerID = INVALID_PLAYER_ID;
		receivedMessage >> playerID;

		PlayerClient* player = getPlayer(playerID);
		assert(player);
		if (player)
			player->increaseBombExplosionSize();
	}
	break;

	case eServerMessageType::eBombKicked:
	{
		sf::Vector2f bombOnPosition;
		receivedMessage >> bombOnPosition;
		sf::Vector2f kickToPosition;
		receivedMessage >> kickToPosition;

		kickBombToPosition(bombOnPosition, kickToPosition);
	}
	break;
#ifdef RENDER_PATHING
	case eServerMessageType::ePathToRender:
	{
		std::vector<sf::Vector2f> path;
		receivedMessage >> path;
		int playerID = INVALID_PLAYER_ID;
		receivedMessage >> playerID;
		std::cout << playerID << "\n";

		PlayerClient* player = getPlayer(playerID);
		assert(player);
		if (player)
			player->setPathToRender(path);
	}
	break;
#endif // RENDER_PATHING
	}
}