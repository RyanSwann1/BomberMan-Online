#include "NetworkHandler.h"
#include <iostream>
#include "Level.h"
#include "Resources.h"
#include "Player.h"

sf::Vector2f lerp(sf::Vector2f startPosition, sf::Vector2f endPosition, float t)
{
	return startPosition * 1;
}

int main()
{
	sf::RenderWindow window(sf::VideoMode(640, 480), "SFML_WINDOW", sf::Style::Default);
	
	//if (!NetworkHandler::getInstance().connectToServer())
	//{
	//	std::cerr << "Couldn't connect to server\n";
	//	return -1;
	//}

	if (!Textures::getInstance().loadAllTextures())
	{
		return -1;
	}

	std::unique_ptr<Level> level = Level::create("Level1.tmx");
	if (!level)
	{
		return -1;
	}

	Player player;

	
	sf::Clock gameClock;
	float deltaTime = 0;
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

		window.clear(sf::Color::Black);
		level->render(window);
		window.display();

		deltaTime = gameClock.restart().asSeconds();
	}
}