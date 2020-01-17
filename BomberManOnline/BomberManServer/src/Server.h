#pragma once

#include "NonCopyable.h"
#include "CollidableTile.h"
#include "PlayerServer.h"
#include "BombServer.h"
#include "TileManager.h"
#include <SFML/Network.hpp>
#include <memory>
#include <vector>

//https://stackoverflow.com/questions/385506/when-is-optimisation-premature

//https://www.youtube.com/watch?v=k7RM-ot2NWY&list=PLZHQObOWTQDPD3MizzM2xVFitgF8hE_ab&index=2

struct ServerMessagePlayerMove;
class PlayerServerHuman;
class PlayerServerAI;
class Server : private NonCopyable
{
	enum class eServerState
	{
		eLobby = 0,
		eGame
	};

public:
	static std::unique_ptr<Server> create(const sf::IpAddress& ipAddress, unsigned short portNumber);

	bool isPickUpAtPosition(sf::Vector2f position) const;
	bool isBombAtPosition(sf::Vector2f position) const;
	const PlayerServer* getPlayer(int ID) const;
	const BombServer* getBomb(sf::Vector2f position) const;
	const std::vector<std::unique_ptr<PlayerServer>>& getPlayers() const;
	const TileManager& getTileManager() const;
	const std::vector<GameObject>& getGameObjects() const;
	const std::vector<BombServer>& getBombs() const;
	sf::Vector2i getTileSize() const;
	sf::Vector2i getLevelSize() const;

	void kickBombInDirection(sf::Vector2f bombPosition, sf::Vector2f newPosition);
	void placeBomb(sf::Vector2f position, int explosionRange);
	void broadcastMessage(sf::Packet& packetToSend);
	void run();

private:
	Server();
	sf::TcpListener m_tcpListener;
	sf::SocketSelector m_socketSelector;
	std::vector<std::unique_ptr<PlayerServer>> m_players;
	std::vector<int> m_clientsToRemove;
	std::vector<sf::Vector2f> m_spawnPositions;
	TileManager m_tileManager;
	std::vector<GameObject> m_gameObjectQueue;
	std::vector<GameObject> m_gameObjects;
	std::vector<BombServer> m_bombs;
	std::string m_levelName;
	sf::Vector2i m_levelSize;
	sf::Vector2i m_tileSize;
	sf::Clock m_clock;
	eServerState m_currentState;
	bool m_running;

	void addNewClient();
	void listen();
	void setNewPlayerPosition(PlayerServerHuman& client, ServerMessagePlayerMove playerMoveMessage);
	void placeBomb(PlayerServerHuman& client, sf::Vector2f placementPosition);
	void update(float frameTime);
	void onBombExplosion(sf::Vector2f explosionPosition);
	void onBombKick(sf::Vector2f playerPosition, eDirection kickDirection);
	void handlePickUpCollision(PlayerServer& player, eGameObjectType gameObjectType);
	void startGame();
};