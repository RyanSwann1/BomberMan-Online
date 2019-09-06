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
	m_pickUps.reserve(MAX_GAME_OBJECTS);
}

void Level::spawnExplosions(sf::Vector2f bombExplodePosition)
{
	m_gameObjects.emplace_back(bombExplodePosition, EXPLOSION_LIFETIME_DURATION, eAnimationName::eExplosion, eGameObjectType::eExplosion);
	
	sf::Vector2i tileSize = Textures::getInstance().getTileSheet().getTileSize();
	for (int x = bombExplodePosition.x - tileSize.x; x <= bombExplodePosition.x + tileSize.x; x += tileSize.x * 2)
	{
		if (m_collisionLayer[static_cast<int>(bombExplodePosition.y / tileSize.y)][static_cast<int>(x / tileSize.y)] != eCollidableTile::eWall)
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
		return std::unique_ptr<Level>();
	}

	//Initialize Remote Players
	for (const auto& playerDetails : initialGameData.playerDetails)
	{
		if (playerDetails.ID == localClientID)
		{
			level->m_players.emplace_back(std::make_unique<PlayerClientLocalPlayer>(playerDetails.ID, playerDetails.spawnPosition));
			level->m_localPlayer = static_cast<PlayerClientLocalPlayer*>(level->m_players.back().get());
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
	sf::Vector2i tileSize = Textures::getInstance().getTileSheet().getTileSize();
	switch (sfmlEvent.key.code)
	{
	case sf::Keyboard::A:
	{
		sf::Vector2f newPosition(m_localPlayer->m_position.x - tileSize.x, m_localPlayer->m_position.y);
		if (!m_localPlayer->m_moving && !Utilities::isPositionCollidable(m_collisionLayer, newPosition, tileSize))
		{
			m_localPlayer->setNewPosition(newPosition);
		}

		break;
	}

	case sf::Keyboard::D:
	{
		sf::Vector2f newPosition(m_localPlayer->m_position.x + tileSize.x, m_localPlayer->m_position.y);
		if (!m_localPlayer->m_moving && !Utilities::isPositionCollidable(m_collisionLayer, newPosition, tileSize))
		{
			m_localPlayer->setNewPosition(newPosition);
		}

		break;
	}

	case sf::Keyboard::W:
	{
		sf::Vector2f newPosition(m_localPlayer->m_position.x, m_localPlayer->m_position.y - tileSize.y);
		if (!m_localPlayer->m_moving && !Utilities::isPositionCollidable(m_collisionLayer, newPosition, tileSize))
		{
			m_localPlayer->setNewPosition(newPosition);
		}

		break;
	}

	case sf::Keyboard::S:
	{
		sf::Vector2f newPosition(m_localPlayer->m_position.x, m_localPlayer->m_position.y + tileSize.y);
		if (!m_localPlayer->m_moving && !Utilities::isPositionCollidable(m_collisionLayer, newPosition, tileSize))
		{
			m_localPlayer->setNewPosition(newPosition);
		}
		break;
	}

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
		player->m_sprite.render(window);
	}

	//Game Objects
	for (const auto& gameObject : m_gameObjects)
	{
		gameObject.m_sprite.render(window);
	}

	//Pick Up
	for (const auto& pickUp : m_pickUps)
	{
		window.draw(pickUp.m_shape);
	}
}

void Level::update(float deltaTime)
{
	if (m_localPlayer)
	{
		m_localPlayer->m_bombPlacementTimer.update(deltaTime);
	}

	for (auto& player : m_players)
	{
		if (!player->m_moving)
		{
			continue;
		}

		player->m_movementFactor += deltaTime * player->m_movementSpeed;
		player->m_position = Utilities::Interpolate(player->m_previousPosition, player->m_newPosition, player->m_movementFactor);
		player->m_sprite.setPosition(player->m_position);
		player->m_sprite.update(deltaTime);

		//Reached destination
		if (player->m_position == player->m_newPosition)
		{
			if (m_localPlayer->m_previousPositions.size() > MAX_PREVIOUS_POINTS)
			{
				for (auto iter = m_localPlayer->m_previousPositions.begin(); iter != m_localPlayer->m_previousPositions.end();)
				{
					m_localPlayer->m_previousPositions.erase(iter);
					break;
				}
			}

			player->m_moving = false;
			player->m_movementFactor = 0;
		}
	}

	for (auto gameObject = m_gameObjects.begin(); gameObject != m_gameObjects.end();)
	{
		gameObject->m_lifeTimer.update(deltaTime);
		gameObject->m_sprite.update(deltaTime);

		if (gameObject->m_lifeTimer.isExpired())
		{
			if (gameObject->m_type == eGameObjectType::eBomb)
			{
				spawnExplosions(gameObject->m_position);
			}
			gameObject = m_gameObjects.erase(gameObject);
		}
		else
		{
			++gameObject;
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
		for (auto previousPosition = m_localPlayer->m_previousPositions.begin(); previousPosition != m_localPlayer->m_previousPositions.end();)
		{
			if (clearRemaining)
			{
				previousPosition = m_localPlayer->m_previousPositions.erase(previousPosition);
			}
			else if ((*previousPosition).position == invalidMoveMessage.invalidPosition)
			{
				m_localPlayer->m_position = invalidMoveMessage.lastValidPosition;
				m_localPlayer->m_previousPosition = invalidMoveMessage.lastValidPosition;
				m_localPlayer->m_moving = false;
				m_localPlayer->m_movementFactor = 0;

				previousPosition = m_localPlayer->m_previousPositions.erase(previousPosition);
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
		if (clientID == m_localPlayer->m_ID)
		{
			for (auto iter = m_localPlayer->m_previousPositions.begin(); iter != m_localPlayer->m_previousPositions.end();)
			{
				if ((*iter).position == newPosition)
				{
					iter = m_localPlayer->m_previousPositions.erase(iter);
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
			auto player = std::find_if(m_players.begin(), m_players.end(), [clientID](const auto& player) { return player->m_ID == clientID; });
			assert(player != m_players.end());

			(*player)->setNewPosition(newPosition);
		}
	}
		break;

	case eServerMessageType::ePlaceBomb :
	{
		sf::Vector2f placementPosition;
		receivedMessage >> placementPosition.x >> placementPosition.y;

		m_gameObjects.emplace_back(placementPosition, BOMB_LIFETIME_DURATION, eAnimationName::eBomb, eGameObjectType::eBomb);
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
		if (m_localPlayer->m_ID == clientID)
		{
			window.close();
		}
		else
		{
			auto iter = std::find_if(m_players.begin(), m_players.end(), [clientID](const auto& player) { return player->m_ID == clientID; });
			assert(iter != m_players.end());
			
			m_players.erase(iter);
		}
	}
		break;
	case eServerMessageType::eSpawnMovementPickUp :
	{
		sf::Vector2f position;
		receivedMessage >> position.x >> position.y;
		m_pickUps.emplace_back(position, sf::Color::Red, Textures::getInstance().getTileSheet().getTileSize(), eGameObjectType::eMovementPickUp);
	}
		break;
	}
}