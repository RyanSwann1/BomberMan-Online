#pragma once

#include "NonCopyable.h"
#include "CollidableTile.h"
#include "GameObjectsServer.h"
#include <SFML/Network.hpp>
#include <memory>
#include <vector>

//https://stackoverflow.com/questions/385506/when-is-optimisation-premature

struct ServerMessagePlayerMove;
struct PlayerServerHuman;
struct PlayerServerAI;
class Server : private NonCopyable
{
public:
	static std::unique_ptr<Server> create(const sf::IpAddress& ipAddress, unsigned short portNumber);

	void run();

private:
	Server();
	sf::TcpListener m_tcpListener;
	sf::SocketSelector m_socketSelector;
	std::vector<std::unique_ptr<Player>> m_players;
	std::vector<int> m_clientsToRemove;
	std::vector<sf::Vector2f> m_spawnPositions;
	std::vector<std::vector<eCollidableTile>> m_collisionLayer;
	std::vector<BombServer> m_bombs;
	std::vector<PickUpServer> m_pickUps;
	std::string m_levelName;
	sf::Vector2i m_mapDimensions;
	sf::Vector2i m_tileSize;
	sf::Clock m_clock;
	bool m_gameRunning;
	bool m_running;

	void addNewClient();
	void listen();
	void broadcastMessage(sf::Packet& packetToSend);

	void setNewPlayerPosition(PlayerServerHuman& client, ServerMessagePlayerMove playerMoveMessage);
	void placeBomb(PlayerServerHuman& client, sf::Vector2f placementPosition);

	void update(float frameTime);
	void updateAI(PlayerServerAI& player, float frameTime);
	
	void onBombExplosion(sf::Vector2f explosionPosition);
	void handlePickUpCollision(Player& player, eGameObjectType gameObjectType);
};