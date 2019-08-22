#include "Server.h"
#include "XMLParser.h"
#include <iostream>

constexpr size_t MAX_CLIENTS = 4;

Server::Server()
	: m_tcpListener(),
	m_socketSelector(),
	m_running(false),
	m_clients()
{
	m_clients.reserve(MAX_CLIENTS);
}

std::unique_ptr<Server> Server::create(const sf::IpAddress & ipAddress, unsigned short portNumber)
{
	std::unique_ptr<Server> server = std::make_unique<Server>();
	if (server->m_tcpListener.listen(portNumber, ipAddress) == sf::Socket::Done)
	{
		server->m_socketSelector.add(server->m_tcpListener);
		server->m_running = true;
		server->m_levelName = "Level1.tmx";
		if (!XMLParser::loadMapAsServer(server->m_levelName, server->m_mapDimensions, server->m_collisionLayer, server->m_spawnPositions))
		{
			return std::unique_ptr<Server>();
		}

		return server;
	}
	else
	{
		return std::unique_ptr<Server>();
	}
}

void Server::run()
{
	while (m_running)
	{
		if (m_socketSelector.wait())
		{
			//Accept new client
			if (m_socketSelector.isReady(m_tcpListener))
			{
				addNewClient();
			}
		}
	}
}

void Server::addNewClient()
{
	std::unique_ptr<sf::TcpSocket> tcpSocket = std::make_unique<sf::TcpSocket>();
	if (m_tcpListener.accept(*tcpSocket) == sf::Socket::Done)
	{
		m_clients.emplace_back(std::move(tcpSocket));
		std::cout << "New client added to server\n";
	}
}