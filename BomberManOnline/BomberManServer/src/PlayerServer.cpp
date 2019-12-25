#include "PlayerServer.h"
#include "Utilities.h"
#include "PathFinding.h"
#include "Server.h"
#include "ServerMessageType.h"
#include "ServerMessages.h"
#include <iostream>
#include <algorithm>
#include <assert.h>

//Player Server
PlayerServer::PlayerServer(int ID, sf::Vector2f startingPosition, ePlayerControllerType controllerType)
	: Player(ID, startingPosition, controllerType)
{}

void PlayerServer::setNewPosition(sf::Vector2f newPosition, Server & server)
{
	//Assign new movement direction
	if (newPosition.x > m_position.x)
	{
		m_facingDirection = eDirection::eRight;
	}
	else if (newPosition.x < m_position.x)
	{
		m_facingDirection = eDirection::eLeft;
	}
	else if (newPosition.y < m_position.y)
	{
		m_facingDirection = eDirection::eUp;
	}
	else if (newPosition.y > m_position.y)
	{
		m_facingDirection = eDirection::eDown;
	}

	m_newPosition = newPosition;
	m_previousPosition = m_position;

	sf::Packet globalPacket;
	globalPacket << static_cast<int>(eServerMessageType::eNewPlayerPosition) << m_newPosition.x << m_newPosition.y << m_ID;
	server.broadcastMessage(globalPacket);
}

//Player Human
PlayerServerHuman::PlayerServerHuman(std::unique_ptr<sf::TcpSocket> tcpSocket, int ID, sf::Vector2f startingPosition, sf::SocketSelector& socketSelector)
	: PlayerServer(ID, startingPosition, ePlayerControllerType::eHuman),
	m_tcpSocket(std::move(tcpSocket))
{ 
	socketSelector.add(*m_tcpSocket.get());
}

PlayerServerHuman::~PlayerServerHuman()
{
	std::cout << "Destroyed Human Player\n";
	m_tcpSocket->disconnect();
}

std::unique_ptr<sf::TcpSocket>& PlayerServerHuman::getTCPSocket()
{
	return m_tcpSocket;
}