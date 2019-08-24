#include <iostream>
#include "Server.h"

//https://gamedev.stackexchange.com/questions/119342/authoritative-server-movement-and-collision

//https://gamedev.stackexchange.com/questions/162872/why-do-games-use-tick-rates-in-their-networking-and-servers-instead-of-an-event
//https://gamedev.stackexchange.com/questions/132831/what-is-the-point-of-update-independent-rendering-in-a-game-loop/132835#132835

//https://gamedev.stackexchange.com/questions/29434/fps-networking-with-server-sending-input-instead-of-gamestate?rq=1wh

int main()
{
	std::unique_ptr<Server> server = Server::create(sf::IpAddress::LocalHost, 55001);
	if (!server)
	{
		std::cout << "Failed to start server\n";
		return -1;
	}

	server->run();
	
	return 0;
}