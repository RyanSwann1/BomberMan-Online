#include "Level.h"
#include "XMLParser/XMLParser.h"
#include "Resources.h"
#include "NetworkHandler.h"
#include "ServerMessages.h"
#include <assert.h>
#include "Utilities.h"

constexpr size_t MAX_BOMBS = 50;
constexpr size_t MAX_PLAYERS = 4;
constexpr size_t MAX_EXPLOSIONS = 50;
constexpr float EXPLOSION_DURATION = 0.5f;

Level::Level()
	: m_levelName(),
	m_levelSize(),
	m_tileLayers(),
	m_spawnPositions(),
	m_collisionLayer(),
	m_localPlayer(nullptr),
	m_players(),
	m_bombs(),
	m_explosions()
{
	m_players.reserve(MAX_PLAYERS);
	m_explosions.reserve(MAX_EXPLOSIONS);
}

void Level::spawnExplosions(sf::Vector2f bombExplodePosition)
{
	int tileSize = Textures::getInstance().getTileSheet().getTileSize();
	for (int x = bombExplodePosition.x - tileSize; x <= bombExplodePosition.x + tileSize; x += tileSize * 2)
	{
		if (m_collisionLayer[bombExplodePosition.y / tileSize][x / tileSize] == eCollidableTile::eNonCollidable)
		{
			m_explosions.emplace_back(sf::Vector2f(x, bombExplodePosition.y), EXPLOSION_DURATION);
		}
	}

	for (int y = bombExplodePosition.y - tileSize; y <= bombExplodePosition.y + tileSize; y += tileSize * 2)
	{
		if (m_collisionLayer[y / tileSize][bombExplodePosition.x / tileSize] == eCollidableTile::eNonCollidable)
		{
			m_explosions.emplace_back(sf::Vector2f(bombExplodePosition.x, y), EXPLOSION_DURATION);
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
	for (const auto& player : initialGameData.playerDetails)
	{
		if (player.ID == localClientID)
		{
			level->m_players.emplace_back(std::make_unique<PlayerClientLocalPlayer>(Textures::getInstance().getTileSheet().getTileSize(), player.ID, player.spawnPosition));
			level->m_localPlayer = static_cast<PlayerClientLocalPlayer*>(level->m_players.back().get()); //&*level->m_players.back();
		}
		else
		{
			level->m_players.emplace_back(std::make_unique<PlayerClient>(Textures::getInstance().getTileSheet().getTileSize(), player.ID, player.spawnPosition));
		}
	}

	return std::unique_ptr<Level>(level);
}

void Level::handleInput(const sf::Event & sfmlEvent)
{
	int tileSize = Textures::getInstance().getTileSheet().getTileSize();
	switch (sfmlEvent.key.code)
	{
	case sf::Keyboard::A:
	{
		sf::Vector2f newPosition(m_localPlayer->m_position.x - tileSize, m_localPlayer->m_position.y);
		if (!m_localPlayer->m_moving && !Utilities::isPositionCollidable(m_collisionLayer, newPosition))
		{
			m_localPlayer->setNewPosition(newPosition);
		}

		break;
	}

	case sf::Keyboard::D:
	{
		sf::Vector2f newPosition(sf::Vector2f(m_localPlayer->m_position.x + tileSize, m_localPlayer->m_position.y));
		if (!m_localPlayer->m_moving && !Utilities::isPositionCollidable(m_collisionLayer, newPosition))
		{
			m_localPlayer->setNewPosition(newPosition);
		}

		break;
	}

	case sf::Keyboard::W:
	{
		sf::Vector2f newPosition(m_localPlayer->m_position.x, m_localPlayer->m_position.y - tileSize);
		if (!m_localPlayer->m_moving && !Utilities::isPositionCollidable(m_collisionLayer, newPosition))
		{
			m_localPlayer->setNewPosition(newPosition);
		}

		break;
	}

	case sf::Keyboard::S:
	{
		sf::Vector2f newPosition(m_localPlayer->m_position.x, m_localPlayer->m_position.y + tileSize);
		if (!m_localPlayer->m_moving && !Utilities::isPositionCollidable(m_collisionLayer, newPosition))
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
					tileSprite.setPosition(x * tileSheet.getTileSize(), y * tileSheet.getTileSize());

					window.draw(tileSprite);
				}
			}
		}
	}

	for (const auto& player : m_players)
	{
		window.draw(player->m_shape);
	}

	sf::Sprite boxSprite(Textures::getInstance().getTileSheet().getTexture(), Textures::getInstance().getTileSheet().getFrameRect(204));
	for (int y = 0; y < m_levelSize.y; y++)
	{
		for (int x = 0; x < m_levelSize.x; x++)
		{
			if (m_collisionLayer[y][x] == eCollidableTile::eBox)
			{
				boxSprite.setPosition(sf::Vector2f(x * 16, y * 16));
				window.draw(boxSprite);
			}
		}
	}

	for (const auto& bomb : m_bombs)
	{
		window.draw(bomb.m_sprite);
	}

	for (const auto& explosion : m_explosions)
	{
		window.draw(explosion.m_sprite);
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
		player->m_shape.setPosition(player->m_position);

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

	for (auto iter = m_bombs.begin(); iter != m_bombs.end();)
	{
		iter->m_lifeTimer.update(deltaTime);

		if (iter->m_lifeTimer.isExpired())
		{
			spawnExplosions(iter->m_position);
			iter = m_bombs.erase(iter);
		}
		else
		{
			++iter;
		}
	}

	for (auto iter = m_explosions.begin(); iter != m_explosions.end();)
	{
		iter->m_lifeTimer.update(deltaTime);

		if (iter->m_lifeTimer.isExpired())
		{
			iter = m_explosions.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

void Level::onReceivedServerMessage(eServerMessageType receivedMessageType, sf::Packet & receivedMessage, sf::RenderWindow& window)
{
	int tileSize = Textures::getInstance().getTileSheet().getTileSize();
	switch (receivedMessageType)
	{
	case eServerMessageType::eInvalidMoveRequest :
	{
		ServerMessageInvalidMove invalidMoveMessage;
		receivedMessage >> invalidMoveMessage;

		bool previousPositionFound = false;
		bool clearRemaining = false;
		for (auto iter = m_localPlayer->m_previousPositions.begin(); iter != m_localPlayer->m_previousPositions.end();)
		{
			if (clearRemaining)
			{
				iter = m_localPlayer->m_previousPositions.erase(iter);
			}
			else if ((*iter).position == invalidMoveMessage.invalidPosition)
			{
				m_localPlayer->m_position = invalidMoveMessage.lastValidPosition;
				m_localPlayer->m_previousPosition = invalidMoveMessage.lastValidPosition;
				m_localPlayer->m_moving = false;
				m_localPlayer->m_movementFactor = 0;

				iter = m_localPlayer->m_previousPositions.erase(iter);
				clearRemaining = true;
				previousPositionFound = true;
			}
			else
			{
				++iter;
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
			auto iter = std::find_if(m_players.begin(), m_players.end(), [clientID](const auto& player) { return player->m_ID == clientID; });
			assert(iter != m_players.end());

			//iter->m_position = newPosition;
			(*iter)->m_newPosition = newPosition;
			(*iter)->m_previousPosition = (*iter)->m_position;
			(*iter)->m_moving = true;
		}
	}
		break;

	case eServerMessageType::ePlaceBomb :
	{
		sf::Vector2f placementPosition;
		float lifeTime = 0;
		receivedMessage >> placementPosition.x >> placementPosition.y >> lifeTime;

		m_bombs.emplace_back(placementPosition, lifeTime);
	}
		break;
	case eServerMessageType::eDestroyBox :
	{
		sf::Vector2f boxPosition;
		receivedMessage >> boxPosition.x >> boxPosition.y;
		m_collisionLayer[boxPosition.y / tileSize][boxPosition.x / tileSize] = eCollidableTile::eNonCollidable;
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
	}
}