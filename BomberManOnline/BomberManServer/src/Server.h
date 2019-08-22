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

	void addNewClient();
};