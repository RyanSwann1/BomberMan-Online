#include <iostream>
#include "Server.h"

//https://gamedev.stackexchange.com/questions/119342/authoritative-server-movement-and-collision

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