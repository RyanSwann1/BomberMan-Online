#pragma once

#include "NonCopyable.h"
#include "TileLayer.h"
#include "Box.h"
#include "GameObjectClient.h"
#include "PlayerClient.h"
#include "CollidableTile.h"
#include "Direction.h"
#include "TileManager.h"
#include <vector>
#include <string>
#include <memory>
#include <SFML/Network.hpp>

struct ServerMessageInitialGameData;
class Level : private NonCopyable
{
public:
	static std::unique_ptr<Level> create(int localClientID, ServerMessageInitialGameData& initialGameData);

	void handleInput(const sf::Event& sfmlEvent);
	void onReceivedServerMessage(eServerMessageType receivedMessageType, sf::Packet& receivedMessage, sf::RenderWindow& window);

	void render(sf::RenderWindow& window) const;
	void update(float deltaTime);

private:
	Level(std::string&& levelName);

	std::string m_levelName;
	sf::Vector2i m_levelSize;
	TileManager m_tileManager;
	std::vector<sf::Vector2f> m_spawnPositions;
	PlayerClient* m_localPlayer;
	std::vector<MovementPoint> m_localPlayerPreviousPositions;
	std::vector<std::unique_ptr<PlayerClient>> m_players;
	std::vector<GameObjectClient> m_gameObjects; 

	void spawnPickUp(sf::Vector2f position, eGameObjectType gameObjectType);
	void onBombExplosion(sf::Vector2f position, int explosionSize);

	void addExplosionObject(sf::Vector2f position);
	void kickBombToPosition(sf::Vector2f bombPosition, sf::Vector2f kickToPosition);
	PlayerClient* getPlayer(int ID);
};