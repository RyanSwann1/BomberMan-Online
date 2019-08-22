#include "NetworkHandler.h"
#include <iostream>
#include "Level.h"

int main()
{
	sf::RenderWindow window(sf::VideoMode(640, 480), "SFML_WINDOW", sf::Style::Default);
	
	//if (!NetworkHandler::getInstance().connectToServer())
	//{
	//	std::cerr << "Couldn't connect to server\n";
	//	return -1;
	//}

	std::unique_ptr<Level> level = Level::create("Level1.tmx");
	if (!level)
	{
		return -1;
	}




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