#include "Level.h"
#include "XMLParser/XMLParser.h"
#include "Resources.h"
#include "NetworkHandler.h"
#include "ServerMessages.h"
#include <assert.h>
#include "Utilities.h"

constexpr size_t MAX_GAME_OBJECTS = 50;
constexpr size_t MAX_PLAYERS = 4;

Level::Level()
	: m_levelName(),
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

void Level::spawnExplosions(sf::Vector2f bombExplodePosition)
{
	m_gameObjects.emplace_back(bombExplodePosition, EXPLOSION_LIFETIME_DURATION, eAnimationName::eExplosion, eGameObjectType::eExplosion);
	
	sf::Vector2i tileSize = Textures::getInstance().getTileSheet().getTileSize();
	for (int x = bombExplodePosition.x - tileSize.x; x <= bombExplodePosition.x + tileSize.x; x += tileSize.x * 2)
	{
		if (m_collisionLayer[static_cast<int>(bombExplodePosition.y / tileSize.y)][static_cast<int>(x / tileSize.x)] != eCollidableTile::eWall)
		{
			m_gameObjects.emplace_back(sf::Vector2f(x, bombExplodePosition.y), EXPLOSION_LIFETIME_DURATION, eAnimationName::eExplosion, eGameObjectType::eExplosion);
		}
	}

	for (int y = bombExplodePosition.y - tileSize.y; y <= bombExplodePosition.y + tileSize.y; y += tileSize.y * 2)
	{
		if (m_collisionLayer[static_cast<int>(y / tileSize.y)][static_cast<int>(bombExplodePosition.x / tileSize.x)] != eCollidableTile::eWall)
		{
			m_gameObjects.emplace_back(sf::Vector2f(bombExplodePosition.x, y), EXPLOSION_LIFETIME_DURATION, eAnimationName::eExplosion, eGameObjectType::eExplosion);
		}
	}
}

std::unique_ptr<Level> Level::create(int localClientID, const ServerMessageInitialGameData & initialGameData)
{
	Level* level = new Level;
	level->m_levelName = initialGameData.levelName;
	if (!XMLParser::loadLevelAsClient(level->m_levelName, level->m_levelSize, level->m_tileLayers,
		level->m_collisionLayer, level->m_spawnPositions))
	{
		delete level;
		return std::unique_ptr<Level>();
	}

	//Initialize Remote Players
	for (const auto& playerDetails : initialGameData.playerDetails)
	{
		if (playerDetails.ID == localClientID)
		{
			level->m_players.emplace_back(std::make_unique<PlayerClient>(playerDetails.ID, playerDetails.spawnPosition));
			level->m_localPlayer = level->m_players.back().get();
		}
		else
		{
			level->m_players.emplace_back(std::make_unique<PlayerClient>(playerDetails.ID, playerDetails.spawnPosition));
		}
	}

	return std::unique_ptr<Level>(level);
}

void Level::handleInput(const sf::Event & sfmlEvent)
{
	sf::Vector2i tileSize(Textures::getInstance().getTileSheet().getTileSize());
	assert(m_localPlayer);
	sf::Vector2f playerPosition(m_localPlayer->getPosition());
	switch (sfmlEvent.key.code)
	{
	case sf::Keyboard::A:
		m_localPlayer->setLocalPlayerPosition(sf::Vector2f(playerPosition.x - tileSize.x, playerPosition.y), 
			m_collisionLayer, tileSize, m_localPlayerPreviousPositions);

		break;
	
	case sf::Keyboard::D:
		m_localPlayer->setLocalPlayerPosition(sf::Vector2f(playerPosition.x + tileSize.x, playerPosition.y),
			m_collisionLayer, tileSize, m_localPlayerPreviousPositions);

		break;

	case sf::Keyboard::W:
		m_localPlayer->setLocalPlayerPosition(sf::Vector2f(playerPosition.x, playerPosition.y - tileSize.y),
			m_collisionLayer, tileSize, m_localPlayerPreviousPositions);

		break;

	case sf::Keyboard::S:
		m_localPlayer->setLocalPlayerPosition(sf::Vector2f(playerPosition.x, playerPosition.y + tileSize.y),
			m_collisionLayer, tileSize, m_localPlayerPreviousPositions);

		break;

	case sf::Keyboard::Space:
		m_localPlayer->plantBomb();

		break;
	}
}

void Level::render(sf::RenderWindow & window) const
{
	//Tile Layer
	const auto& tileSheet = Textures::getInstance().getTileSheet();
	for (const auto& tileLayer : m_tileLayers)
	{
		for (int y = 0; y < m_levelSize.y; ++y)
		{
			for (int x = 0; x < m_levelSize.x; ++x)
			{
				int tileID = tileLayer.m_tileLayer[y][x];
				if (tileID > 0)
				{
					sf::Sprite tileSprite(Textures::getInstance().getTileSheet().getTexture(), tileSheet.getFrameRect(tileID));
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
				sf::Sprite boxSprite(Textures::getInstance().getTileSheet().getTexture(), Textures::getInstance().getTileSheet().getFrameRect(204));
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

			player->stop();
		}
	}

	//Game Objects
	for (auto gameObject = m_gameObjects.begin(); gameObject != m_gameObjects.end();)
	{
		gameObject->update(deltaTime);

		if (gameObject->getTag() == eGameObjectTag::ePickUp)
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
		else if(gameObject->getTag() == eGameObjectTag::eNone)
		{
			if (gameObject->getTimer().isExpired())
			{
				if (gameObject->getType() == eGameObjectType::eBomb)
				{
					spawnExplosions(gameObject->getPosition());
				}

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
			NetworkHandler::getInstance().disconnectFromServer();
			window.close();
			return;
		}
	}
	break;
		
	case eServerMessageType::eNewPlayerPosition :
	{
		sf::Vector2f newPosition;
		int clientID = 0;
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

			(*remotePlayer)->setRemotePlayerPosition(newPosition);
		}
	}
		break;

	case eServerMessageType::ePlaceBomb :
	{
		sf::Vector2f placementPosition;
		receivedMessage >> placementPosition.x >> placementPosition.y;

		m_gameObjects.emplace_back(placementPosition, BOMB_LIFETIME_DURATION, eAnimationName::eBomb, eGameObjectType::eBomb, eGameObjectTag::eNone, eTimerActive::eTrue);
	}
		break;
	case eServerMessageType::eDestroyBox :
	{
		sf::Vector2i tileSize = Textures::getInstance().getTileSheet().getTileSize();
		sf::Vector2f boxPosition;
		receivedMessage >> boxPosition.x >> boxPosition.y;
		m_collisionLayer[static_cast<int>(boxPosition.y / tileSize.y)][static_cast<int>(boxPosition.x / tileSize.x)] = eCollidableTile::eNonCollidable;
	}
		break;
	case eServerMessageType::ePlayerDisconnected :
	{
		int clientID = 0;
		receivedMessage >> clientID;
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

		m_gameObjects.emplace_back(startingPosition, 0.0f, eAnimationName::eMovementSpeedPickUp, eGameObjectType::eMovementPickUp);
	}
		break;
		
	case eServerMessageType::eMovementPickUpCollision :
	{
		int clientID = 0;
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
	}
}

MovementPoint::MovementPoint(sf::Vector2f position, eDirection moveDirection)
	: position(position),
	moveDirection(moveDirection)
{}