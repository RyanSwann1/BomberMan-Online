#pragma once

#include "Utilities/NonCopyable.h"
#include "Client.h"
#include <SFML/Network.hpp>
#include <memory>
#include <vector>

enum class eServerMessageType
{
	eInvalidRequest = 0
};

struct ServerMessage
{
	eServerMessageType type;
	int messsageID;
};

//bool loadMapAsServer(const std::string& mapName, sf::Vector2i& mapDimensions,
//	std::vector<sf::Vector2i>& collisionLayer, std::vector<sf::Vector2i>& spawnPositions);

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
};