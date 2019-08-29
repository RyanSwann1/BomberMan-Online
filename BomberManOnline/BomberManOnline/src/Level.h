#pragma once

#include "NonCopyable.h"
#include "TileLayer.h"
#include "Box.h"
#include "GameObjects.h"
#include "CollidableTile.h"
#include <vector>
#include <string>
#include <memory>
#include <SFML/Network.hpp>
#include <SFML/Window.hpp>

struct ServerMessageInitialGameData;
class Level : private NonCopyable
{
public:
	static std::unique_ptr<Level> create(int localClientID, const ServerMessageInitialGameData& initialGameData);

	void handleInput(const sf::Event& sfmlEvent, std::vector<sf::Vector2f>& recentPositions);
	void onReceivedServerMessage(eServerMessageType receivedMessageType, sf::Packet& receivedMessage, std::vector<sf::Vector2f>& recentPositions, sf::RenderWindow& window);

	void render(sf::RenderWindow& window) const;
	void update(float deltaTime);

private:
	Level();
	std::string m_levelName;
	sf::Vector2i m_levelDimensions;
	std::vector<TileLayer> m_tileLayers;
	std::vector<sf::Vector2f> m_spawnPositions;
	std::vector<std::vector<eCollidableTile>> m_collisionLayer;

	PlayerClient* m_localPlayer;
	std::vector<PlayerClient> m_players;
	std::vector<Bomb> m_bombs;
	std::vector<Explosion> m_explosions;

	void spawnExplosions(sf::Vector2f bombExplodePosition);
};