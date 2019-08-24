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
	Client(std::unique_ptr<sf::TcpSocket> tcpSocket, int ID)
		: m_tcpSocket(std::move(tcpSocket)),
		m_ID(ID)
	{}

	int m_ID;
	std::unique_ptr<sf::TcpSocket> m_tcpSocket;
	sf::Vector2f m_position;
	eDirection m_moveDirection;
	ePlayerControllerType m_controllerType;
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
	std::vector<Client> m_clients;
	std::string m_levelName;
	sf::Vector2i m_mapDimensions;
	std::vector<sf::Vector2i> m_collisionLayer;
	std::vector<sf::Vector2i> m_spawnPositions;

	void addNewClient();
	void listen();
	void broadcastMessage(sf::Packet& packetToSend);

	void movePlayer(Client& client, sf::Vector2f newPosition);
};