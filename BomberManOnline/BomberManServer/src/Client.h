#pragma once

#include <SFML/Network.hpp>
#include <memory>

struct Client
{
	Client(std::unique_ptr<sf::TcpSocket>& tcpSocket)
		: m_tcpSocket(std::move(tcpSocket))
	{

	}

	std::unique_ptr<sf::TcpSocket> m_tcpSocket;
};