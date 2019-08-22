#pragma once

#include "Utilities/NonCopyable.h"
#include <SFML/Network.hpp>
#include <memory>

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

private:
	Server();

	sf::TcpListener m_tcpListener;
	sf::SocketSelector m_socketSelector;
	bool m_running;
};