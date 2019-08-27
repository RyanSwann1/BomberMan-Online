#pragma once

#include "NonCopyable.h"
#include "TileLayer.h"
#include "Player.h"
#include "Box.h"
#include "Bomb.h"
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

	const std::vector<sf::Vector2f>& getCollisionLayer() const;
	std::vector<Box>& getBoxes();

	void handleInput(const sf::Event& sfmlEvent, std::vector<sf::Vector2f>& recentPositions);
	void onReceivedServerMessage(eServerMessageType receivedMessageType, sf::Packet& receivedMessage, std::vector<sf::Vector2f>& recentPositions);

	void render(sf::RenderWindow& window) const;
	void update(float deltaTime);

private:
	Level();
	std::string m_levelName;
	sf::Vector2i m_levelDimensions;
	std::vector<sf::Vector2f> m_collisionLayer;
	std::vector<TileLayer> m_tileLayers;
	std::vector<sf::Vector2f> m_spawnPositions;

	Player* m_localPlayer;
	std::vector<Box> m_boxes;
	std::vector<Player> m_players;
	std::vector<Bomb> m_bombs;
};