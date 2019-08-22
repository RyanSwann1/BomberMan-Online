#include "NetworkHandler.h"
#include <iostream>
#include <SFML/Graphics.hpp>
#include <vector>


int main()
{
	sf::RenderWindow window(sf::VideoMode(640, 480), "SFML_WINDOW", sf::Style::Default);
	
	if (!NetworkHandler::getInstance().connectToServer())
	{
		std::cerr << "Couldn't connect to server\n";
		return -1;
	}

	//bool loadMapAsClient(const std::string& mapName, sf::Vector2i& mapDimensions,
	//	std::vector<std::vector<int>>& tileData, std::vector<sf::Vector2i>& collisionLayer, std::vector<sf::Vector2i>& spawnPositions);


	while (window.isOpen())
	{
		sf::Event sfmlEvent;
		while (window.pollEvent(sfmlEvent))
		{
			if (sfmlEvent.type == sf::Event::Closed)
			{
				window.close();
			}
		}
	}
}