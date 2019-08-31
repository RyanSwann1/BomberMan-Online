#include "NetworkHandler.h"
#include <iostream>
#include "Level.h"
#include "Resources.h"
#include "ServerMessages.h"
#include <algorithm>
#include <assert.h>
#include "Utilities.h"

//http://www.codersblock.org/blog/multiplayer-fps-part-1

//https://gamedev.stackexchange.com/questions/151068/networking-how-does-server-fixed-timestep-work

//https://gamedev.stackexchange.com/questions/6645/lag-compensation-with-networked-2d-games

//https://www.youtube.com/watch?v=aVmqv3z4gnA

//https://www.bogotobogo.com/DesignPatterns/introduction.php


int main()
{
	sf::RenderWindow window(sf::VideoMode(640, 480), "SFML_WINDOW", sf::Style::Default);
	
	if (!NetworkHandler::getInstance().connectToServer())
	{
		std::cerr << "Couldn't connect to server\n";
		return -1;
	}

	if (!Textures::getInstance().loadAllTextures())
	{
		return -1;
	}

	std::unique_ptr<Level> level;
	std::vector<sf::Vector2f> recentPositions;
	recentPositions.reserve(MAX_RECENT_POSITIONS);
	sf::Clock gameClock;
	float deltaTime = 0;
	int clientID = 0;

	while (window.isOpen())
	{
		//Handle Server Messages
		if (!NetworkHandler::getInstance().getNetworkMessages().empty())
		{
			for (auto& receivedMessage : NetworkHandler::getInstance().getNetworkMessages())
			{
				eServerMessageType messageType;
				receivedMessage >> messageType;
				switch (messageType)
				{
				case eServerMessageType::eInitializeClientID:
					receivedMessage >> clientID;
					break;
				case eServerMessageType::eInitialGameData:
				{
					ServerMessageInitialGameData initialGameData;
					receivedMessage >> initialGameData;
					level = Level::create(clientID, initialGameData);
				}
				break;
				case eServerMessageType::eInvalidMoveRequest:
				case eServerMessageType::eNewPlayerPosition:
				case eServerMessageType::ePlaceBomb:
				case eServerMessageType::eDestroyBox:
				case eServerMessageType::ePlayerDisconnected :
				{
					level->onReceivedServerMessage(messageType, receivedMessage, recentPositions, window);
				}
				break;
				}
			}

			NetworkHandler::getInstance().getNetworkMessages().clear();
		}

		//Input Handling
		sf::Event sfmlEvent;
		while (window.pollEvent(sfmlEvent))
		{
			if (sfmlEvent.type == sf::Event::Closed)
			{
				window.close();
			}
			else if (sfmlEvent.type == sf::Event::KeyPressed)
			{
				level->handleInput(sfmlEvent, recentPositions);
			}
		}

		//Update
		if (level)
		{
			level->update(deltaTime);
		}
		
		//Render
		window.clear(sf::Color::Black);
		if (level)
		{
			level->render(window);
		}
		window.display();

		deltaTime = gameClock.restart().asSeconds();
	}

	NetworkHandler::getInstance().disconnectFromServer();
}