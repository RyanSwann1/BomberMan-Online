#include "NetworkHandler.h"
#include <assert.h>

bool NetworkHandler::connectToServer()
{
	assert(m_connectedToServer);
	if (m_tcpSocket.connect(sf::IpAddress::LocalHost, 55001, sf::seconds(2.5f)) != sf::Socket::Done)
	{
		return false;
	}

	m_connectedToServer = true;
	return true;
}