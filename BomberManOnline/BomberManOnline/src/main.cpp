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
constexpr size_t MAX_PLAYERS = 4;

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

	int tileSize = Textures::getInstance().getTileSheet().getTileSize();
	std::unique_ptr<Level> level;
	std::vector<Player> players;
	players.reserve(MAX_PLAYERS);
	Player* localPlayer = nullptr;
	std::vector<Bomb> bombs;
	bombs.reserve(MAX_BOMBS);
	std::vector<sf::Vector2f> recentPositions;
	recentPositions.reserve(MAX_RECENT_POSITIONS);
	sf::Clock gameClock;
	float deltaTime = 0;
	bool gameStarted = false;

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
				{
					int clientID = 0;
					networkMessage >> clientID;
					players.emplace_back(tileSize, clientID);
					localPlayer = &players.back();
				}

					break;

				case eServerMessageType::eInitialGameData : 
				{
					ServerMessageInitialGameData initialGameData;
					networkMessage >> initialGameData;

					assert(!level);
					level = Level::create(initialGameData.levelName);

					//Initialize local player
					assert(localPlayer);
					int localPlayerID = localPlayer->m_ID;
					auto cIter = std::find_if(initialGameData.playerDetails.cbegin(), initialGameData.playerDetails.cend(),
						[localPlayerID](const auto& playerDetails) { return playerDetails.ID == localPlayerID; });
					assert(cIter != initialGameData.playerDetails.cend());

					//Initialize Remote Players
					for (auto& player : initialGameData.playerDetails)
					{
						if (player.ID != localPlayerID)
						{
							players.emplace_back(tileSize, player.ID, player.spawnPosition);
						}
					}

					localPlayer->m_position = cIter->spawnPosition;
					localPlayer->m_previousPosition = localPlayer->m_position;
					localPlayer->m_shape.setPosition(localPlayer->m_position);
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
					localPlayer->m_position = invalidMoveMessage.lastValidPosition;
					localPlayer->m_previousPosition = invalidMoveMessage.lastValidPosition;
					localPlayer->m_moving = false;
					localPlayer->m_movementFactor = 0;
				}

					break;
				case eServerMessageType::eNewPlayerPosition :
				{
					sf::Vector2f newPosition;
					int clientID = 0;
					networkMessage >> newPosition.x >> newPosition.y >> clientID;
					if (clientID == localPlayer->m_ID)
					{
						for (auto iter = recentPositions.begin(); iter != recentPositions.end();)
						{
							if ((*iter) == newPosition)
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
					else
					{
						auto iter = std::find_if(players.begin(), players.end(), [clientID](const auto& player) { return player.m_ID == clientID; });
						assert(iter != players.end());

						iter->m_newPosition = newPosition;
						iter->m_previousPosition = iter->m_position;
						iter->m_moving = true;
					}

					break;
				}
				case eServerMessageType::ePlaceBomb :
				{
					sf::Vector2f placementPosition;
					float lifeTime = 0;
					networkMessage >> placementPosition.x >> placementPosition.y >> lifeTime;
					
					bombs.emplace_back(Textures::getInstance().getTileSheet(), placementPosition, lifeTime);
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
				!localPlayer->m_moving && !Utilities::isPositionCollidable(level->getCollisionLayer(), sf::Vector2f(localPlayer->m_position.x - tileSize, localPlayer->m_position.y)))
			{
			
				localPlayer->m_newPosition = sf::Vector2f(localPlayer->m_position.x - tileSize, localPlayer->m_position.y);
				localPlayer->m_previousPosition = localPlayer->m_position;
				localPlayer->m_moving = true;
				recentPositions.push_back(localPlayer->m_previousPosition);

				sf::Packet packetToSend;
				packetToSend << eServerMessageType::ePlayerMoveToPosition << ServerMessagePlayerMove(localPlayer->m_newPosition, localPlayer->m_movementSpeed);
				NetworkHandler::getInstance().sendMessageToServer(packetToSend);
			}
			//Move player right
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) &&
				!localPlayer->m_moving && !Utilities::isPositionCollidable(level->getCollisionLayer(), sf::Vector2f(localPlayer->m_position.x + tileSize, localPlayer->m_position.y)))
			{
				
				localPlayer->m_newPosition = sf::Vector2f(localPlayer->m_position.x + tileSize, localPlayer->m_position.y);
				localPlayer->m_previousPosition = localPlayer->m_position;
				localPlayer->m_moving = true;
				recentPositions.push_back(localPlayer->m_previousPosition);
			
				sf::Packet packetToSend;
				packetToSend << eServerMessageType::ePlayerMoveToPosition << ServerMessagePlayerMove(localPlayer->m_newPosition, localPlayer->m_movementSpeed);
				NetworkHandler::getInstance().sendMessageToServer(packetToSend);
			}
			//Move player up
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) &&
				!localPlayer->m_moving && !Utilities::isPositionCollidable(level->getCollisionLayer(), sf::Vector2f(localPlayer->m_position.x, localPlayer->m_position.y - tileSize)))
			{
				
				localPlayer->m_newPosition = sf::Vector2f(localPlayer->m_position.x, localPlayer->m_position.y - tileSize);
				localPlayer->m_previousPosition = localPlayer->m_position;
				localPlayer->m_moving = true;
				recentPositions.push_back(localPlayer->m_previousPosition);
				
				sf::Packet packetToSend;
				packetToSend << eServerMessageType::ePlayerMoveToPosition << ServerMessagePlayerMove(localPlayer->m_newPosition, localPlayer->m_movementSpeed);
				NetworkHandler::getInstance().sendMessageToServer(packetToSend);
			}
			//Move player down
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) &&
				!localPlayer->m_moving && !Utilities::isPositionCollidable(level->getCollisionLayer(), sf::Vector2f(localPlayer->m_position.x, localPlayer->m_position.y + tileSize)))
			{
				
				localPlayer->m_newPosition = sf::Vector2f(localPlayer->m_position.x, localPlayer->m_position.y + tileSize);
				localPlayer->m_previousPosition = localPlayer->m_position;
				localPlayer->m_moving = true;
				recentPositions.push_back(localPlayer->m_previousPosition);

				sf::Packet packetToSend;
				packetToSend << eServerMessageType::ePlayerMoveToPosition << ServerMessagePlayerMove(localPlayer->m_newPosition, localPlayer->m_movementSpeed);
				NetworkHandler::getInstance().sendMessageToServer(packetToSend);
			}
			else if (!localPlayer->m_moving && localPlayer->m_bombPlacementTimer.isExpired() && sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
			{
				localPlayer->m_bombPlacementTimer.resetElaspedTime();

				sf::Packet packetToSend;
				packetToSend << eServerMessageType::ePlayerBombPlacementRequest << localPlayer->m_position.x << localPlayer->m_position.y;
				NetworkHandler::getInstance().sendMessageToServer(packetToSend);
			}

			for (auto& player : players)
			{
				if (!player.m_moving)
				{
					continue;
				}

				player.m_movementFactor += deltaTime * player.m_movementSpeed;
				player.m_position = Utilities::Interpolate(player.m_previousPosition, player.m_newPosition, player.m_movementFactor);
				player.m_shape.setPosition(player.m_position);

				//Reached destination
				if (player.m_position == player.m_newPosition)
				{
					//if (recentPositions.size() > MAX_RECENT_POSITIONS)
					//{
					//	for (auto iter = recentPositions.begin(); iter != recentPositions.end();)
					//	{
					//		recentPositions.erase(iter);
					//		break;
					//	}
					//}

					player.m_moving = false;
					player.m_movementFactor = 0;
				}
			}

			//if (localPlayer->m_moving)
			//{
			//	localPlayer->m_movementFactor += deltaTime * localPlayer->m_movementSpeed;
			//	localPlayer->m_position = Utilities::Interpolate(localPlayer->m_previousPosition, localPlayer->m_newPosition, localPlayer->m_movementFactor);
			//	localPlayer->m_shape.setPosition(localPlayer->m_position);

			//	//Reached destination
			//	if (localPlayer->m_position == localPlayer->m_newPosition)
			//	{
			//		if (recentPositions.size() > MAX_RECENT_POSITIONS)
			//		{
			//			for (auto iter = recentPositions.begin(); iter != recentPositions.end();)
			//			{
			//				recentPositions.erase(iter);
			//				break;
			//			}
			//		}

			//		localPlayer->m_moving = false;
			//	}
			//}
		}
		
		localPlayer->m_bombPlacementTimer.update(deltaTime);

		for (auto iter = bombs.begin(); iter != bombs.end();)
		{
			iter->m_lifeTimer.update(deltaTime);

			if (iter->m_lifeTimer.isExpired())
			{
				iter = bombs.erase(iter);
			}
			else
			{
				++iter;
			}
		}

		window.clear(sf::Color::Black);
		
		if (gameStarted)
		{
			assert(level);
			level->render(window);
			for(auto& player : players)
			{ 
				window.draw(player.m_shape);
			}
			for (const auto& bomb : bombs)
			{
				window.draw(bomb.m_sprite);
			}
		}

		window.display();

		deltaTime = gameClock.restart().asSeconds();
	}
}