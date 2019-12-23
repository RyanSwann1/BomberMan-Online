#include <iostream>
#include "Server.h"
#include <unordered_map>
#include <vector>
#include <array>
#include <assert.h>

//https://gamedev.stackexchange.com/questions/119342/authoritative-server-movement-and-collision

//https://gamedev.stackexchange.com/questions/162872/why-do-games-use-tick-rates-in-their-networking-and-servers-instead-of-an-event
//https://gamedev.stackexchange.com/questions/132831/what-is-the-point-of-update-independent-rendering-in-a-game-loop/132835#132835

//https://gamedev.stackexchange.com/questions/29434/fps-networking-with-server-sending-input-instead-of-gamestate?rq=1wh


//https://embeddedartistry.com/blog/2017/09/11/choosing-the-right-container-sequential-containers/


enum class eBodyPart
{
	eHead = 0,
	eBody,
	eFeet,
	eWeapon,
	eGunBarrel,
	eMAX = eGunBarrel
};

std::array<sf::Sprite, static_cast<int>(eBodyPart::eMAX) + 1> getArray(sf::Texture& texture)
{
	std::array<sf::Sprite, static_cast<int>(eBodyPart::eMAX) + 1> array
	{
		sf::Sprite(texture),
		sf::Sprite(texture),
		sf::Sprite(texture),
		sf::Sprite(texture),
		sf::Sprite(texture)
	};

	return array;
}

struct Tower
{
	Tower(sf::Texture& texture)
		: m_texture(texture),
		number(0),
		m_sprites(getArray(texture))
	{
		number = 5;
	}

	sf::Texture& m_texture;
	int number;
	std::array<sf::Sprite, static_cast<int>(eBodyPart::eMAX) + 1> m_sprites;
};

struct Texture
{
	sf::Texture m_texture;

	const int m_standardTextureWidth = 128;
	const int m_standardTextureHeight = 128;
	const int m_weaponTextureWidth = 31;
	const int m_weaponTextureHeight = 139;
	const int m_deathTextureHeight = 256;
	const int m_padding = 5;
};

int main()
{
	std::array<sf::Sprite, static_cast<int>(eBodyPart::eTotal) - 1>
	{
		sf::Sprite(),
			sf::Sprite()
	}

	std::unique_ptr<Server> server = Server::create(sf::IpAddress::LocalHost, 55001);
	assert(server);
	if (!server)
	{
		std::cout << "Failed to start server\n";
		return -1;
	}

	server->run();
	
	return 0;
}


