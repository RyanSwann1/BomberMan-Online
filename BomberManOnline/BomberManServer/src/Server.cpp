#include "Server.h"

Server::Server()
	: m_tcpListener(),
	m_socketSelector(),
	m_running(false)
{}

std::unique_ptr<Server> Server::create(const sf::IpAddress & ipAddress, unsigned short portNumber)
{
	std::unique_ptr<Server> server = std::make_unique<Server>();
	if (server->m_tcpListener.listen(portNumber, ipAddress))
	{
		server->m_socketSelector.add(server->m_tcpListener);
		server->m_running = true;

		return server;
	}
	else
	{
		return std::unique_ptr<Server>();
	}
}