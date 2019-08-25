#pragma once

#include "NonCopyable.h"
#include "PlayerControllerType.h"
#include "Direction.h"
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
		m_movementSpeed(2.5f)
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
	sf::Clock m_clock;

	void addNewClient();
	void listen();
	void broadcastMessage(sf::Packet& packetToSend);

	void movePlayer(Client& client, ServerMessagePlayerMove playerMoveMessage);

	void update(float frameTime);
};