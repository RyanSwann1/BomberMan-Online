#include "GameObjectsServer.h"
#include <iostream>

PlayerServerHuman::~PlayerServerHuman()
{
	std::cout << "Destroyed Human Player\n";
	m_tcpSocket->disconnect();
}
