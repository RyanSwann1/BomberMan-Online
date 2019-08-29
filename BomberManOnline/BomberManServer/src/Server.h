#pragma once

#include "NonCopyable.h"
#include "CollidableTile.h"
#include "Player.h"
#include <SFML/Network.hpp>
#include <memory>
#include <vector>

struct BombServer
{
	BombServer(sf::Vector2f startingPosition, float expirationTime)
		: m_position(startingPosition),
		m_lifeTime(expirationTime, true)
	{}

	sf::Vector2f m_position;
	Timer m_lifeTime;
};

struct ServerMessagePlayerMove;
struct PlayerServerHuman;
class Server : private NonCopyable
{
public:
	static std::unique_ptr<Server> create(const sf::IpAddress& ipAddress, unsigned short portNumber);

	void run();

private:
	Server();
	sf::TcpListener m_tcpListener;
	sf::SocketSelector m_socketSelector;
	bool m_running;
	std::vector<std::unique_ptr<Player>> m_players;
	std::vector<int> m_clientsToRemove;
	std::string m_levelName;
	sf::Vector2i m_mapDimensions;
	std::vector<sf::Vector2f> m_spawnPositions;
	std::vector<BombServer> m_bombs;
	std::vector<std::vector<eCollidableTile>> m_collisionLayer;
	sf::Clock m_clock;

	void addNewClient();
	void listen();
	void broadcastMessage(sf::Packet& packetToSend);

	void movePlayer(PlayerServerHuman& client, ServerMessagePlayerMove playerMoveMessage);
	void placeBomb(PlayerServerHuman& client, sf::Vector2f placementPosition);

	void update(float frameTime);

	void updateAI();
};