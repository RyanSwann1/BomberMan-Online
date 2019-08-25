#include "NetworkHandler.h"
#include <iostream>
#include "Level.h"
#include "Resources.h"
#include "Player.h"
#include "ServerMessages.h"
#include "Bomb.h"
#include <algorithm>
#include <assert.h>
#include "Utilities.h"

//http://www.codersblock.org/blog/multiplayer-fps-part-1

//https://gamedev.stackexchange.com/questions/151068/networking-how-does-server-fixed-timestep-work

//https://gamedev.stackexchange.com/questions/6645/lag-compensation-with-networked-2d-games

//https://www.youtube.com/watch?v=aVmqv3z4gnA

constexpr size_t MAX_RECENT_POSITIONS = 10;
constexpr size_t MAX_BOMBS = 50;

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

	int tileSize = Textures::getInstance().getTileSheet().getTileSize();
	Player player(tileSize);
	std::vector<Bomb> bombs;
	bombs.reserve(MAX_BOMBS);
	
	sf::Clock gameClock;
	float deltaTime = 0;
	float factor = 0;
	int clientID = 0;
	bool gameStarted = false;
	std::vector<sf::Vector2f> recentPositions;
	recentPositions.reserve(MAX_RECENT_POSITIONS);
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

		if (!NetworkHandler::getInstance().getNetworkMessages().empty())
		{
			for (auto& networkMessage : NetworkHandler::getInstance().getNetworkMessages())
			{
				eServerMessageType messageType;
				networkMessage >> messageType;
				switch (messageType)
				{
				case eServerMessageType::eInitializeClientID :
					networkMessage >> clientID;
					break;

				case eServerMessageType::eInitialGameData : 
				{
					ServerMessageInitialGameData initialGameData;
					networkMessage >> initialGameData;

					assert(!level);
					level = Level::create(initialGameData.levelName);

					player.m_position = initialGameData.playerDetails[0].spawnPosition;
					player.m_previousPosition = player.m_position;
					player.m_shape.setPosition(player.m_position);
					gameStarted = true;
				}
					break;
				case eServerMessageType::eInvalidMoveRequest :
				{
					ServerMessageInvalidMove invalidMoveMessage;
					networkMessage >> invalidMoveMessage;

					bool clearRemaining = false;
					for (auto iter = recentPositions.begin(); iter != recentPositions.end();)
					{
						if (clearRemaining)
						{
							iter = recentPositions.erase(iter);
						}
						else if ((*iter) == invalidMoveMessage.invalidPosition)
						{
							iter = recentPositions.erase(iter);
							clearRemaining = true;
						}
						else
						{
							++iter;
						}
					}

					recentPositions.clear();
					player.m_position = invalidMoveMessage.lastValidPosition;
					player.m_previousPosition = invalidMoveMessage.lastValidPosition;
					player.m_moving = false;
					factor = 0;
				}

					break;
				case eServerMessageType::eValidMoveRequest :
				{
					sf::Vector2f position;
					networkMessage >> position.x >> position.y;
					for (auto iter = recentPositions.begin(); iter != recentPositions.end();)
					{
						if ((*iter) == position)
						{
							iter = recentPositions.erase(iter);
							break;
						}
						else
						{
							++iter;
						}
					}
				}

					break;
				}
			}

			NetworkHandler::getInstance().getNetworkMessages().clear();
		}

		if (gameStarted)
		{
			//Move player Left
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) &&
				!player.m_moving && !Utilities::isPositionCollidable(level->getCollisionLayer(), sf::Vector2f(player.m_position.x - tileSize, player.m_position.y)))
			{
				factor = 0;
				player.m_newPosition = sf::Vector2f(player.m_position.x - tileSize, player.m_position.y);
				player.m_previousPosition = player.m_position;
				player.m_moving = true;
				recentPositions.push_back(player.m_previousPosition);

				sf::Packet packetToSend;
				packetToSend << eServerMessageType::ePlayerMoveToPosition << player.m_newPosition.x << player.m_newPosition.y;
				NetworkHandler::getInstance().sendMessageToServer(packetToSend);
			}
			//Move player right
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) &&
				!player.m_moving && !Utilities::isPositionCollidable(level->getCollisionLayer(), sf::Vector2f(player.m_position.x + tileSize, player.m_position.y)))
			{
				factor = 0;
				player.m_newPosition = sf::Vector2f(player.m_position.x + tileSize, player.m_position.y);
				player.m_previousPosition = player.m_position;
				player.m_moving = true;
				recentPositions.push_back(player.m_previousPosition);
			
				sf::Packet packetToSend;
				packetToSend << eServerMessageType::ePlayerMoveToPosition << player.m_newPosition.x << player.m_newPosition.y;
				NetworkHandler::getInstance().sendMessageToServer(packetToSend);
			}
			//Move player up
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) &&
				!player.m_moving && !Utilities::isPositionCollidable(level->getCollisionLayer(), sf::Vector2f(player.m_position.x, player.m_position.y - tileSize)))
			{
				factor = 0;
				player.m_newPosition = sf::Vector2f(player.m_position.x, player.m_position.y - tileSize);
				player.m_previousPosition = player.m_position;
				player.m_moving = true;
				recentPositions.push_back(player.m_previousPosition);
				
				sf::Packet packetToSend;
				packetToSend << eServerMessageType::ePlayerMoveToPosition << player.m_newPosition.x << player.m_newPosition.y;
				NetworkHandler::getInstance().sendMessageToServer(packetToSend);
			}
			//Move player down
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) &&
				!player.m_moving && !Utilities::isPositionCollidable(level->getCollisionLayer(), sf::Vector2f(player.m_position.x, player.m_position.y + tileSize)))
			{
				factor = 0;
				player.m_newPosition = sf::Vector2f(player.m_position.x, player.m_position.y + tileSize);
				player.m_previousPosition = player.m_position;
				player.m_moving = true;
				recentPositions.push_back(player.m_previousPosition);

				sf::Packet packetToSend;
				packetToSend << eServerMessageType::ePlayerMoveToPosition << player.m_newPosition.x << player.m_newPosition.y;
				NetworkHandler::getInstance().sendMessageToServer(packetToSend);
			}

			if (player.m_moving)
			{
				factor += deltaTime * player.m_movementSpeed;
				player.m_position = Utilities::Interpolate(player.m_previousPosition, player.m_newPosition, factor);
				player.m_shape.setPosition(player.m_position);

				//Reached destination
				if (player.m_position == player.m_newPosition)
				{
					if (recentPositions.size() > MAX_RECENT_POSITIONS)
					{
						for (auto iter = recentPositions.begin(); iter != recentPositions.end();)
						{
							recentPositions.erase(iter);
							break;
						}
					}

					player.m_moving = false;
				}
			}
		}
		
		window.clear(sf::Color::Black);
		level->render(window);
		window.draw(player.m_shape);
		window.display();

		deltaTime = gameClock.restart().asSeconds();
	}
}