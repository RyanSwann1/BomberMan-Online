#pragma once

#include "NonCopyable.h"
#include "PlayerControllerType.h"
#include "Direction.h"
#include "Timer.h"
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <memory>
#include <vector>

struct Client
{
	Client(std::unique_ptr<sf::TcpSocket> tcpSocket, int ID, sf::Vector2f startingPosition, ePlayerControllerType controllerType)
		: m_ID(ID),
		m_tcpSocket(std::move(tcpSocket)),
		m_previousPosition(),
		m_position(startingPosition),
		m_newPosition(),
		m_moveDirection(),
		m_controllerType(controllerType),
		m_moving(false),
		m_movementFactor(0),
		m_movementSpeed(2.5f),
		m_bombPlacementTimer(2.0f, true)
	{}

	int m_ID;
	std::unique_ptr<sf::TcpSocket> m_tcpSocket;
	sf::Vector2f m_previousPosition;
	sf::Vector2f m_position;
	sf::Vector2f m_newPosition;
	eDirection m_moveDirection;
	ePlayerControllerType m_controllerType;
	bool m_moving;
	float m_movementFactor;
	float m_movementSpeed;
	Timer m_bombPlacementTimer;
};

struct BombServer
{
	BombServer(sf::Vector2f startingPosition, int owningPlayerID)
		: m_position(startingPosition),
		m_explosionSize(2, 2),
		m_lifeTime(2.5f, true),
		m_owningPlayerID(owningPlayerID),
		m_damage(1)
	{}

	sf::Vector2f m_position;
	sf::Vector2i m_explosionSize;
	Timer m_lifeTime;
	int m_owningPlayerID;
	int m_damage;
};

struct ServerMessagePlayerMove;
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
	std::vector<Client> m_clients;
	std::string m_levelName;
	sf::Vector2i m_mapDimensions;
	std::vector<sf::Vector2f> m_collisionLayer;
	std::vector<sf::Vector2f> m_spawnPositions;
	std::vector<BombServer> m_bombs;
	sf::Clock m_clock;

	void addNewClient();
	void listen();
	void broadcastMessage(sf::Packet& packetToSend);

	void movePlayer(Client& client, ServerMessagePlayerMove playerMoveMessage);
	void placeBomb(Client& client, sf::Vector2f placementPosition);

	void update(float frameTime);
};