#pragma once

#include "Utilities/NonCopyable.h"
#include <SFML/Network.hpp>
#include <memory>
#include <vector>

enum class eDirection
{

};

class Timer
{

};

//Client
class Player
{
	void generateMovementPath();

	sf::Vector2i position;
	sf::Vector2i speed;
	eDirection direction;
	int health;
	Timer m_bombSpawnTimer; //expiratio time == 0.1f;
};

//Server
class Player
{
	sf::Vector2i position;
	sf::Vector2i speed;
	eDirection direction;
	int health;
	Timer m_bombSpawnTimer;// expiration time == 2.5f;
};

class Server : private NonCopyable
{
public:
	Server();
	static std::unique_ptr<Server> create(const sf::IpAddress& ipAddress, unsigned short portNumber);

	void run();

private:
	sf::TcpListener m_tcpListener;
	sf::SocketSelector m_socketSelector;
	bool m_running;
	std::vector<std::unique_ptr<sf::TcpSocket>> m_clients;
	std::string m_levelName;
	sf::Vector2i m_mapDimensions;
	std::vector<sf::Vector2i> m_collisionLayer;
	std::vector<sf::Vector2i> m_spawnPositions;

	void addNewClient();
	void listen();
	void broadcastMessage(sf::Packet& packetToSend);

	void movePlayer(std::unique_ptr<sf::TcpSocket>& client, sf::Vector2i newPosition);
};